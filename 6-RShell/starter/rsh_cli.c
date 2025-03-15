#include <sys/socket.h>
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
 * exec_remote_cmd_loop(server_ip, port)
 *
 * Main function to execute remote commands via the network
 */
int exec_remote_cmd_loop(char *address, int port)
{
    char *request_buff = NULL;
    char *response_buff = NULL;
    int cli_socket = -1;
    char cmd_buff[SH_CMD_MAX];

    // Allocate buffers for sending commands and receiving responses
    request_buff = malloc(RDSH_COMM_BUFF_SZ);
    response_buff = malloc(RDSH_COMM_BUFF_SZ);
    
    if (!request_buff || !response_buff) {
        return client_cleanup(cli_socket, request_buff, response_buff, ERR_MEMORY);
    }

    // Connect to the server
    cli_socket = start_client(address, port);
    if (cli_socket < 0) {
        return client_cleanup(cli_socket, request_buff, response_buff, ERR_RDSH_CLIENT);
    }

    // Main command loop
    while (1) {
        // Display prompt and get user input
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remove trailing newline
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // Skip empty commands
        if (strlen(cmd_buff) == 0) {
            continue;
        }

        // Copy command to request buffer
        strncpy(request_buff, cmd_buff, RDSH_COMM_BUFF_SZ - 1);
        request_buff[RDSH_COMM_BUFF_SZ - 1] = '\0';

        // Send command to server (include null terminator)
        int send_len = strlen(request_buff) + 1;
        int bytes_sent = send(cli_socket, request_buff, send_len, 0);
        
        if (bytes_sent <= 0) {
            printf("Error: Failed to send command to server\n");
            return client_cleanup(cli_socket, request_buff, response_buff, ERR_RDSH_COMMUNICATION);
        }

        // Receive response from server
        int is_last_chunk = 0;
        while (!is_last_chunk) {
            int recv_size = recv(cli_socket, response_buff, RDSH_COMM_BUFF_SZ - 1, 0);
            
            if (recv_size < 0) {
                printf("Error: Failed to receive response from server\n");
                return client_cleanup(cli_socket, request_buff, response_buff, ERR_RDSH_COMMUNICATION);
            } else if (recv_size == 0) {
                printf("Server connection closed\n");
                return client_cleanup(cli_socket, request_buff, response_buff, OK);
            }

            // Check if this is the last chunk (ends with EOF character)
            is_last_chunk = (response_buff[recv_size - 1] == RDSH_EOF_CHAR) ? 1 : 0;
            
            if (is_last_chunk) {
                // Replace EOF with null for proper string handling
                response_buff[recv_size - 1] = '\0';
                recv_size--;
            } else {
                // Ensure null termination for printing
                response_buff[recv_size] = '\0';
            }

            // Print the response
            if (recv_size > 0) {
                printf("%.*s", recv_size, response_buff);
            }
        }

        // If the command was exit, break out of the loop
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            break;
        }
    }

    return client_cleanup(cli_socket, request_buff, response_buff, OK);
}

/*
 * start_client(server_ip, port)
 *
 * Creates and connects a socket to the server
 */
int start_client(char *server_ip, int port)
{
    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(client_socket);
        return ERR_RDSH_CLIENT;
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(client_socket);
        return ERR_RDSH_CLIENT;
    }

    return client_socket;
}

/*
 * client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc)
 *
 * Helper function to clean up resources
 */
int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc)
{
    // If a valid socket number close it.
    if (cli_socket > 0) {
        close(cli_socket);
    }

    // Free up the buffers
    free(cmd_buff);
    free(rsp_buff);

    // Echo the return value that was passed as a parameter
    return rc;
}