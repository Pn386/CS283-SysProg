#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

static int last_exit_code = 0;

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));

    cmd_buff->_cmd_buffer = strdup(cmd_line);
    char *p = cmd_buff->_cmd_buffer;

    while (isspace(*p)) p++;
    if (*p == '\0') return OK;

    char *token_start = p;
    bool in_quotes = false;

    while (*p) {
        if (*p == '"') {
            in_quotes = !in_quotes;
        } else if (isspace(*p) && !in_quotes) {
            *p = '\0';
            if (cmd_buff->argc < CMD_ARGV_MAX - 1) {
                cmd_buff->argv[cmd_buff->argc++] = strdup(token_start);
            }
            token_start = p + 1;
        }
        p++;
    }

    if (*token_start && cmd_buff->argc < CMD_ARGV_MAX - 1) {
        cmd_buff->argv[cmd_buff->argc++] = strdup(token_start);
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;

    // Remove surrounding quotes in arguments
    for (int i = 0; i < cmd_buff->argc; i++) {
        char *arg = cmd_buff->argv[i];
        size_t len = strlen(arg);
        if (len > 1 && arg[0] == '"' && arg[len - 1] == '"') {
            memmove(arg, arg + 1, len - 1);
            arg[len - 2] = '\0';
        }
    }

    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (!input) return BI_NOT_BI;

    if (strcmp(input, EXIT_CMD) == 0) return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0) return BI_CMD_CD;
    if (strcmp(input, "rc") == 0) return BI_CMD_RC;
    if (strcmp(input, "dragon") == 0) return BI_CMD_DRAGON;

    return BI_NOT_BI;
}

int exec_built_in_cmd(cmd_buff_t *cmd) {
    if (!cmd || !cmd->argv[0]) return BI_NOT_BI;

    if (strcmp(cmd->argv[0], EXIT_CMD) == 0) {
        return OK_EXIT;
    } else if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc == 1) return BI_EXECUTED;
        if (chdir(cmd->argv[1]) != 0) perror("cd");
        return BI_EXECUTED;
    } else if (strcmp(cmd->argv[0], "rc") == 0) {
        printf("%d\n", last_exit_code);
        return BI_EXECUTED;
    } else if (strcmp(cmd->argv[0], "dragon") == 0) {
        print_dragon();
        return BI_EXECUTED;
    }
    return BI_NOT_BI;
}

int exec_local_cmd_loop() {
    char cmd_line[SH_CMD_MAX];
    cmd_buff_t cmd = {0};
    int rc;

    printf("dsh2> ");
    fflush(stdout);

    while (fgets(cmd_line, sizeof(cmd_line), stdin)) {
        size_t len = strlen(cmd_line);
        if (len > 0 && cmd_line[len - 1] == '\n') {
            cmd_line[len - 1] = '\0';
        }

        if (build_cmd_buff(cmd_line, &cmd) != OK || cmd.argc == 0) {
            printf("dsh2> ");
            fflush(stdout);
            continue;
        }

        Built_In_Cmds bi_cmd = match_command(cmd.argv[0]);
        if (bi_cmd != BI_NOT_BI) {
            rc = exec_built_in_cmd(&cmd);
            if (rc == OK_EXIT) break;
        } else {
            pid_t pid = fork();
            if (pid == 0) {
                // Child process: execute command
                execvp(cmd.argv[0], cmd.argv);
                perror("Error executing command");
                exit(127);
            } else if (pid > 0) {
                // Parent process: wait for child
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) {
                    last_exit_code = WEXITSTATUS(status);
                } else {
                    last_exit_code = 1;
                }
            } else {
                perror("fork");
                last_exit_code = 1;
            }
        }

        free_cmd_buff(&cmd);
        printf("dsh2> ");
        fflush(stdout);
    }

    free_cmd_buff(&cmd);
    printf("\ncmd loop returned 0\n");
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff) return OK;

    for (int i = 0; i < cmd_buff->argc; i++) {
        free(cmd_buff->argv[i]);
        cmd_buff->argv[i] = NULL;
    }
    free(cmd_buff->_cmd_buffer);
    cmd_buff->_cmd_buffer = NULL;
    cmd_buff->argc = 0;

    return OK;
}

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff) return ERROR;
    memset(cmd_buff, 0, sizeof(cmd_buff_t));
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff) return ERROR;
    free_cmd_buff(cmd_buff);
    return alloc_cmd_buff(cmd_buff);
}
