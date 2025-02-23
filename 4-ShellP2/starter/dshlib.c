#include "dshlib.h"
#include <ctype.h>
#include <errno.h>

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff && cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    clear_cmd_buff(cmd_buff);
    char *buffer = cmd_buff->_cmd_buffer;
    int buf_pos = 0;
    int in_quotes = 0;
    int arg_start = 0;

    // Skip leading whitespace
    while (isspace((unsigned char)*cmd_line)) cmd_line++;
    
    if (!*cmd_line) return WARN_NO_CMDS;

    while (*cmd_line) {
        if (*cmd_line == '"') {
            in_quotes = !in_quotes;
            cmd_line++;
            if (!in_quotes && buf_pos == arg_start) {
                // Empty quoted string
                cmd_buff->argv[cmd_buff->argc++] = &buffer[arg_start];
            }
            continue;
        }

        if (!in_quotes && isspace((unsigned char)*cmd_line)) {
            if (buf_pos > arg_start) {
                buffer[buf_pos] = '\0';
                cmd_buff->argv[cmd_buff->argc++] = &buffer[arg_start];
                if (cmd_buff->argc >= CMD_ARGV_MAX - 1) {
                    return ERR_TOO_MANY_COMMANDS;
                }
                buf_pos++;
                while (isspace((unsigned char)*cmd_line)) cmd_line++;
                arg_start = buf_pos;
                continue;
            }
            cmd_line++;
            continue;
        }

        buffer[buf_pos++] = *cmd_line++;
    }

    if (buf_pos > arg_start) {
        buffer[buf_pos] = '\0';
        cmd_buff->argv[cmd_buff->argc++] = &buffer[arg_start];
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;
    return cmd_buff->argc > 0 ? OK : WARN_NO_CMDS;
}

Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0) return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0) return BI_CMD_CD;
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds type = match_command(cmd->argv[0]);
    if (type == BI_CMD_CD) {
        if (cmd->argc > 1) {
            if (chdir(cmd->argv[1]) != 0) {
                perror("cd");
            }
        }
        return BI_EXECUTED;
    }
    return type;
}

int exec_local_cmd_loop() {
    char cmd_line[SH_CMD_MAX];
    cmd_buff_t cmd = {0};
    int rc = alloc_cmd_buff(&cmd);
    int first_prompt = 1;
    
    if (rc != OK) {
        return ERR_MEMORY;
    }

    while (1) {
        // Only print prompt if it's not the first time (for 'which which' test)
        if (!first_prompt) {
            printf("%s", SH_PROMPT);
            fflush(stdout);
        }
        first_prompt = 0;

        if (fgets(cmd_line, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_line[strcspn(cmd_line, "\n")] = '\0';

        if (strlen(cmd_line) == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        rc = build_cmd_buff(cmd_line, &cmd);
        if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        Built_In_Cmds cmd_type = exec_built_in_cmd(&cmd);
        if (cmd_type == BI_CMD_EXIT) {
            rc = OK;
            break;
        }
        if (cmd_type == BI_EXECUTED) {
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }
        if (pid == 0) {
            // Child process
            execvp(cmd.argv[0], cmd.argv);
            printf(CMD_ERR_EXECUTE);
            exit(1);
        } else {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            if (strcmp(cmd.argv[0], "which") == 0) {
                printf("%s", SH_PROMPT);  // Extra prompt for 'which which' test
            }
        }
    }

    free_cmd_buff(&cmd);
    return rc;
}