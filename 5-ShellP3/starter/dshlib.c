#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

/**
 * Allocates memory for a command buffer
 * 
 * @param cmd_buff The command buffer to allocate
 * @return OK on success, ERR_MEMORY on failure
 */
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff) return ERR_MEMORY;
    
    cmd_buff->_cmd_buffer = (char *)malloc(SH_CMD_MAX * sizeof(char));
    if (!cmd_buff->_cmd_buffer) {
        return ERR_MEMORY;
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

/**
 * Frees memory for a command buffer
 * 
 * @param cmd_buff The command buffer to free
 * @return OK on success
 */
int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff) return OK;

    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

/**
 * Clears a command buffer without freeing memory
 * 
 * @param cmd_buff The command buffer to clear
 * @return OK on success
 */
int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff || !cmd_buff->_cmd_buffer) return ERR_MEMORY;
    
    memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    
    // Clear redirection fields
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append_output = 0;
    
    return OK;
}

/**
 * Closes a command buffer and sets the last argument to NULL
 * 
 * @param cmd_buff The command buffer to close
 * @return OK on success
 */
int close_cmd_buff(cmd_buff_t *cmd_buff) {
    if (!cmd_buff) return ERR_MEMORY;
    
    if (cmd_buff->argc < CMD_ARGV_MAX) {
        cmd_buff->argv[cmd_buff->argc] = NULL;
    }
    return OK;
}

// Add these to the struct definition in dshlib.h
// typedef struct cmd_buff {
//     int  argc;
//     char *argv[CMD_ARGV_MAX];
//     char *_cmd_buffer;
//     char *input_file;    // For < redirection
//     char *output_file;   // For > redirection
//     int append_output;   // For >> redirection (1 = append, 0 = overwrite)
// } cmd_buff_t;

/**
 * Builds a command buffer from a command line
 * 
 * @param cmd_line The command line to parse
 * @param cmd_buff The command buffer to build
 * @return OK on success, ERR_MEMORY on failure
 */
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    if (!cmd_line || !cmd_buff || !cmd_buff->_cmd_buffer) return ERR_MEMORY;
    
    // Clear the command buffer
    clear_cmd_buff(cmd_buff);
    
    // Copy the command line to the buffer
    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';
    
    // Initialize redirection fields
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append_output = 0;
    
    // Tokenize the command line
    char *token = strtok(cmd_buff->_cmd_buffer, " \t\n");
    while (token != NULL && cmd_buff->argc < CMD_ARGV_MAX - 1) {
        // Check for input redirection '<'
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t\n");
            if (token) {
                cmd_buff->input_file = token;
            }
        }
        // Check for output redirection '>'
        else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\n");
            if (token) {
                cmd_buff->output_file = token;
                cmd_buff->append_output = 0;
            }
        }
        // Check for output append redirection '>>'
        else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " \t\n");
            if (token) {
                cmd_buff->output_file = token;
                cmd_buff->append_output = 1;
            }
        }
        else {
            cmd_buff->argv[cmd_buff->argc++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    
    // Ensure the last argument is NULL
    close_cmd_buff(cmd_buff);
    
    return (cmd_buff->argc > 0) ? OK : WARN_NO_CMDS;
}

/**
 * Builds a command list from a command line that may include pipes
 * 
 * @param cmd_line The command line to parse
 * @param clist The command list to build
 * @return OK on success, ERR_TOO_MANY_COMMANDS if too many commands
 */
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (!cmd_line || !clist) return ERR_MEMORY;
    
    // Initialize command list
    clist->num = 0;
    
    // Make a copy of the command line
    char cmd_line_copy[SH_CMD_MAX];
    strncpy(cmd_line_copy, cmd_line, SH_CMD_MAX - 1);
    cmd_line_copy[SH_CMD_MAX - 1] = '\0';
    
    // Split by pipes
    char *cmd_start = cmd_line_copy;
    char *pipe_pos;
    
    while ((pipe_pos = strstr(cmd_start, PIPE_STRING)) != NULL && clist->num < CMD_MAX) {
        // Null-terminate at pipe
        *pipe_pos = '\0';
        
        // Allocate command buffer
        if (alloc_cmd_buff(&clist->commands[clist->num]) != OK) {
            return ERR_MEMORY;
        }
        
        // Parse command
        int rc = build_cmd_buff(cmd_start, &clist->commands[clist->num]);
        if (rc != OK && rc != WARN_NO_CMDS) {
            return rc;
        }
        
        // If valid command, increment counter
        if (rc == OK) {
            clist->num++;
        }
        
        // Move to next command after pipe
        cmd_start = pipe_pos + 1;
    }
    
    // Process the last command (or the only one if no pipes)
    if (clist->num < CMD_MAX) {
        if (alloc_cmd_buff(&clist->commands[clist->num]) != OK) {
            return ERR_MEMORY;
        }
        
        int rc = build_cmd_buff(cmd_start, &clist->commands[clist->num]);
        if (rc == OK) {
            clist->num++;
        }
    } else {
        return ERR_TOO_MANY_COMMANDS;
    }
    
    return (clist->num > 0) ? OK : WARN_NO_CMDS;
}

/**
 * Frees memory for a command list
 * 
 * @param cmd_lst The command list to free
 * @return OK on success
 */
int free_cmd_list(command_list_t *cmd_lst) {
    if (!cmd_lst) return OK;
    
    for (int i = 0; i < cmd_lst->num; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
    cmd_lst->num = 0;
    
    return OK;
}

/**
 * Matches a command against built-in commands
 * 
 * @param input The command string to match
 * @return The built-in command enum value
 */
Built_In_Cmds match_command(const char *input) {
    if (!input) return BI_NOT_BI;
    
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    }
    
    return BI_NOT_BI;
}

/**
 * Executes a built-in command
 * 
 * @param cmd The command buffer to execute
 * @return The result of the command execution
 */
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (!cmd || cmd->argc == 0 || !cmd->argv[0]) return BI_NOT_BI;
    
    Built_In_Cmds cmd_type = match_command(cmd->argv[0]);
    
    switch (cmd_type) {
        case BI_CMD_EXIT:
            printf("exiting...\n");
            return BI_CMD_EXIT;
        case BI_CMD_CD:
            if (cmd->argc > 1) {
                if (chdir(cmd->argv[1]) != 0) {
                    perror("cd");
                }
            } else {
                // Change to home directory if no argument
                const char *home = getenv("HOME");
                if (home && chdir(home) != 0) {
                    perror("cd");
                }
            }
            return BI_EXECUTED;
        case BI_CMD_DRAGON:
            printf("Here be dragons!\n");
            return BI_EXECUTED;
        default:
            return BI_NOT_BI;
    }
}

/**
 * Executes a single command
 * 
 * @param cmd The command buffer to execute
 * @return OK on success, error code on failure
 */
int exec_cmd(cmd_buff_t *cmd) {
    if (!cmd || cmd->argc == 0) return WARN_NO_CMDS;
    
    // Check if it's a built-in command
    Built_In_Cmds cmd_type = exec_built_in_cmd(cmd);
    if (cmd_type == BI_CMD_EXIT) {
        return OK_EXIT;
    } else if (cmd_type == BI_EXECUTED) {
        return OK;
    }
    
    // Fork and execute external command
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        perror("fork");
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        // Child process
        
        // Handle input redirection
        if (cmd->input_file) {
            int fd = open(cmd->input_file, O_RDONLY);
            if (fd < 0) {
                perror("open input file");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDIN_FILENO) < 0) {
                perror("dup2 input file");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        
        // Handle output redirection
        if (cmd->output_file) {
            int flags = O_WRONLY | O_CREAT;
            if (cmd->append_output) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            
            int fd = open(cmd->output_file, flags, 0644);
            if (fd < 0) {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("dup2 output file");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        
        // Execute the command
        if (execvp(cmd->argv[0], cmd->argv) < 0) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            return ERR_EXEC_CMD;
        }
    }
    
    return OK;
}

/**
 * Executes a pipeline of commands
 * 
 * @param clist The command list to execute
 * @return OK on success, error code on failure
 */
int execute_pipeline(command_list_t *clist) {
    if (!clist || clist->num == 0) return WARN_NO_CMDS;
    
    // Check if it's a single command
    if (clist->num == 1) {
        return exec_cmd(&clist->commands[0]);
    }
    
    // Handle pipeline
    int pipes[CMD_MAX-1][2]; // Array of pipe file descriptors
    pid_t pids[CMD_MAX];     // Array of process IDs
    
    // Create pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }
    
    // Execute commands in the pipeline
    for (int i = 0; i < clist->num; i++) {
        // Fork process
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        } else if (pids[i] == 0) {
            // Child process
            
            // Set up input from previous pipe (except for first command)
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) < 0) {
                    perror("dup2 input");
                    exit(EXIT_FAILURE);
                }
            } 
            // Handle input redirection for first command
            else if (clist->commands[i].input_file) {
                int fd = open(clist->commands[i].input_file, O_RDONLY);
                if (fd < 0) {
                    perror("open input file");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd, STDIN_FILENO) < 0) {
                    perror("dup2 input file");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                close(fd);
            }
            
            // Set up output to next pipe (except for last command)
            if (i < clist->num - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) < 0) {
                    perror("dup2 output");
                    exit(EXIT_FAILURE);
                }
            } 
            // Handle output redirection for last command
            else if (clist->commands[i].output_file) {
                int flags = O_WRONLY | O_CREAT;
                if (clist->commands[i].append_output) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }
                
                int fd = open(clist->commands[i].output_file, flags, 0644);
                if (fd < 0) {
                    perror("open output file");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2 output file");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                close(fd);
            }
            
            // Close all pipe file descriptors in the child
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute command
            if (execvp(clist->commands[i].argv[0], clist->commands[i].argv) < 0) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    // Close all pipe file descriptors in the parent
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all child processes to finish
    for (int i = 0; i < clist->num; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            return ERR_EXEC_CMD;
        }
    }
    
    return OK;
}

/**
 * Main execution loop for the shell
 * 
 * @return OK on successful termination, error code on failure
 */
int exec_local_cmd_loop() {
    char cmd_buff[SH_CMD_MAX];
    
    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        
        // Remove the trailing newline
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        
        // Skip empty commands
        if (strlen(cmd_buff) == 0) {
            continue;
        }
        
        // Check for exit command directly
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            printf("exiting...\n");
            return OK;
        }
        
        // Parse and execute command
        command_list_t cmd_list;
        int rc = build_cmd_list(cmd_buff, &cmd_list);
        
        if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
            continue;
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        } else if (rc == ERR_MEMORY) {
            printf("error: memory allocation failed\n");
            return ERR_MEMORY;
        }
        
        // Execute the command(s)
        rc = execute_pipeline(&cmd_list);
        
        if (rc == OK_EXIT) {
            free_cmd_list(&cmd_list);
            return OK;
        } else if (rc == ERR_EXEC_CMD) {
            printf(CMD_ERR_EXECUTE);
        }
        
        // Free command list resources
        free_cmd_list(&cmd_list);
    }
    
    return OK;
}