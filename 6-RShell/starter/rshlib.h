#ifndef __RSHLIB_H__
#define __RSHLIB_H__

#include "dshlib.h"

//
// Protocol-specific constants
//

// The EOF character used to signal the end of command output
static const char RDSH_EOF_CHAR = 0x04;  

// Define default network-related constants
#define RDSH_DEF_PORT           1234        // Default port #
#define RDSH_DEF_SVR_INTFACE    "0.0.0.0"   // Default start all interfaces
#define RDSH_DEF_CLI_CONNECT    "127.0.0.1" // Default server is running on localhost
#define RDSH_COMM_BUFF_SZ       4096        // Default communication buffer size

//
// Remote shell error codes
//
#define WARN_RDSH_NOT_IMPL    100   // Function not implemented warning
#define ERR_RDSH_CLIENT       101   // Client error
#define ERR_RDSH_SERVER       102   // Server error
#define ERR_RDSH_COMMUNICATION 103  // Communication error between client/server

//
// Remote shell client function prototypes
//
int exec_remote_cmd_loop(char *address, int port);
int start_client(char *server_ip, int port);
int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc);

//
// Remote shell server function prototypes
//
int start_server(char *ifaces, int port, int is_threaded);
int boot_server(char *ifaces, int port);
int process_cli_requests(int svr_socket);
int stop_server(int svr_socket);
int exec_client_requests(int cli_socket);
int send_message_eof(int cli_socket);
int send_message_string(int cli_socket, char *buff);
int rsh_execute_pipeline(int cli_sock, command_list_t *clist);

//
// Remote shell built-in command function prototypes
//
Built_In_Cmds rsh_match_command(const char *input);
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd);

#endif // __RSHLIB_H__