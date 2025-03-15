#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"

/*
 * start_server(ifaces, port, is_threaded)
 *
 * Starts the remote shell server
 */
int start_server(char *ifaces, int port, int is_threaded) {
    int svr_socket;
    int rc;

    // Boot the server
    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        return svr_socket;  // Pass through the error code
    }

    // Process client requests
    rc = process_cli_requests(svr_socket);

    // Stop the server
    stop_server(svr_socket);

    return rc;
}

/*
 * boot_server(ifaces, port)
 *
 * Sets up the server socket to accept connections
 */
int boot_server(char *ifaces, int port) {
    int svr_socket;
    struct sockaddr_in server_addr;
    
    // Create server socket
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Set socket option to reuse address
    int enable = 1;
    if (setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, ifaces, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Bind socket to address
    if (bind(svr_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Listen for connections
    if (listen(svr_socket, 5) < 0) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    printf("Server started. Listening on %s:%d\n", ifaces, port);
    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *
 * Main server loop that accepts client connections
 */
int process_cli_requests(int svr_socket) {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int rc;
    
    while (1) {
        // Accept client connection
        client_socket = accept(svr_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }
        
        // Get client IP address for logging
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Client connected from %s\n", client_ip);
        
        // Execute client requests
        rc = exec_client_requests(client_socket);
        
        // Check if we should stop the server
        if (rc == OK_EXIT) {
            printf("Server stopping per client request...\n");
            break;
        }
        
        printf("Client disconnected\n");
    }
    
    return OK_EXIT;
}

/*
 * stop_server(svr_socket)
 *
 * Stops the server and frees resources
 */
int stop_server(int svr_socket) {
    return close(svr_socket);
}

/*
 * exec_client_requests(cli_socket)
 *
 * Handles the execution of commands from a client
 */
int exec_client_requests(int cli_socket) {
    char *io_buff = NULL;
    int recv_bytes;
    int rc;
    command_list_t cmd_list;
    
    // Allocate buffer for communication
    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (!io_buff) {
        close(cli_socket);
        return ERR_MEMORY;
    }
    
    // Process client commands
    while (1) {
        // Receive command from client
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        recv_bytes = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ - 1, 0);
        
        if (recv_bytes < 0) {
            perror("recv");
            free(io_buff);
            close(cli_socket);
            return ERR_RDSH_COMMUNICATION;
        } else if (recv_bytes == 0) {
            // Client closed connection
            printf("Client closed connection\n");
            free(io_buff);
            close(cli_socket);
            return OK;
        }
        
        // Ensure null termination
        io_buff[recv_bytes] = '\0';
        
        // Check for exit commands
        if (strcmp(io_buff, EXIT_CMD) == 0) {
            send_message_string(cli_socket, "Exiting...\n");
            free(io_buff);
            close(cli_socket);
            return OK;
        } else if (strcmp(io_buff, "stop-server") == 0) {
            send_message_string(cli_socket, "Stopping server...\n");
            free(io_buff);
            close(cli_socket);
            return OK_EXIT;
        }
        
        // Check for built-in commands
        cmd_buff_t cmd_buff;
        if (alloc_cmd_buff(&cmd_buff) != OK) {
            send_message_string(cli_socket, "Error: Memory allocation failed\n");
            continue;
        }
        
        rc = build_cmd_buff(io_buff, &cmd_buff);
        
        if (rc != OK) {
            if (rc == WARN_NO_CMDS) {
                send_message_string(cli_socket, CMD_WARN_NO_CMD);
            } else {
                send_message_string(cli_socket, "Error parsing command\n");
            }
            free_cmd_buff(&cmd_buff);
            continue;
        }
        
        // Execute built-in command
        Built_In_Cmds cmd_type = rsh_match_command(cmd_buff.argv[0]);
        
        if (cmd_type != BI_NOT_BI) {
            rc = rsh_built_in_cmd(&cmd_buff);
            
            if (rc == BI_EXECUTED) {
                send_message_eof(cli_socket);
                free_cmd_buff(&cmd_buff);
                continue;
            }
        }
        
        free_cmd_buff(&cmd_buff);
        
        // Parse command for execution
        rc = build_cmd_list(io_buff, &cmd_list);
        
        if (rc != OK) {
            if (rc == WARN_NO_CMDS) {
                send_message_string(cli_socket, CMD_WARN_NO_CMD);
            } else if (rc == ERR_TOO_MANY_COMMANDS) {
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg), CMD_ERR_PIPE_LIMIT, CMD_MAX);
                send_message_string(cli_socket, error_msg);
            } else {
                send_message_string(cli_socket, "Error parsing command\n");
            }
            continue;
        }
        
        // Execute the command pipeline
        rc = rsh_execute_pipeline(cli_socket, &cmd_list);
        
        // Send EOF to signal end of command output
        send_message_eof(cli_socket);
        
        // Free command list resources
        free_cmd_list(&cmd_list);
    }
    
    // This should not be reached due to return statements in the loop
    free(io_buff);
    close(cli_socket);
    return OK;
}

/*
 * send_message_eof(cli_socket)
 *
 * Sends the EOF character to the client
 */
int send_message_eof(int cli_socket) {
    int bytes_sent = send(cli_socket, &RDSH_EOF_CHAR, 1, 0);
    
    if (bytes_sent == 1) {
        return OK;
    }
    
    return ERR_RDSH_COMMUNICATION;
}

/*
 * send_message_string(cli_socket, char *buff)
 *
 * Sends a string message followed by the EOF character
 */
int send_message_string(int cli_socket, char *buff) {
    int len = strlen(buff);
    int bytes_sent = send(cli_socket, buff, len, 0);
    
    if (bytes_sent != len) {
        return ERR_RDSH_COMMUNICATION;
    }
    
    return send_message_eof(cli_socket);
}

/*
 * rsh_execute_pipeline(cli_sock, command_list_t *clist)
 *
 * Executes a pipeline of commands with input/output redirected to the client socket
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    if (!clist || clist->num == 0) {
        return WARN_NO_CMDS;
    }
    
    // Single command case (no pipes)
    if (clist->num == 1) {
        // Fork child process
        pid_t pid = fork();
        
        if (pid < 0) {
            // Fork failed
            return ERR_EXEC_CMD;
        } else if (pid == 0) {
            // Child process
            
            // Redirect stdin, stdout, and stderr to client socket
            dup2(cli_sock, STDIN_FILENO);
            dup2(cli_sock, STDOUT_FILENO);
            dup2(cli_sock, STDERR_FILENO);
            
            // Execute command
            execvp(clist->commands[0].argv[0], clist->commands[0].argv);
            
            // If execvp returns, there was an error
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            
            return WIFEXITED(status) ? WEXITSTATUS(status) : ERR_EXEC_CMD;
        }
    }
    
    // Multiple commands case (pipes)
    int pipes[CMD_MAX-1][2];
    pid_t pids[CMD_MAX];
    
    // Create pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }
    
    // Execute commands in pipeline
    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            // Fork failed
            perror("fork");
            return ERR_EXEC_CMD;
        } else if (pids[i] == 0) {
            // Child process
            
            // First process: stdin from client socket
            if (i == 0) {
                dup2(cli_sock, STDIN_FILENO);
            } else {
                // Not first process: stdin from previous pipe
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Last process: stdout/stderr to client socket
            if (i == clist->num - 1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            } else {
                // Not last process: stdout to next pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            
            // If execvp returns, there was an error
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    
    // Close all pipe file descriptors in parent
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all processes to complete
    int last_status = 0;
    for (int i = 0; i < clist->num; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        
        // Store exit status of last command in pipeline
        if (i == clist->num - 1) {
            last_status = WIFEXITED(status) ? WEXITSTATUS(status) : ERR_EXEC_CMD;
        }
    }
    
    return last_status;
}

/*
 * rsh_match_command(const char *input)
 *
 * Matches input against built-in commands
 */
Built_In_Cmds rsh_match_command(const char *input) {
    if (!input) return BI_NOT_BI;
    
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strcmp(input, "stop-server") == 0) {
        return BI_CMD_STOP_SVR;
    }
    
    return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *
 * Executes built-in commands
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd) {
    if (!cmd || cmd->argc == 0 || !cmd->argv[0]) return BI_NOT_BI;
    
    Built_In_Cmds cmd_type = rsh_match_command(cmd->argv[0]);
    
    switch (cmd_type) {
        case BI_CMD_EXIT:
            return BI_CMD_EXIT;
            
        case BI_CMD_STOP_SVR:
            return BI_CMD_STOP_SVR;
            
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