#ifndef __DSHLIB_H__
#define __DSHLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define CMD_ARGV_MAX (CMD_MAX + 1)
#define SH_CMD_MAX (EXE_MAX + ARG_MAX)

typedef struct cmd_buff {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
} cmd_buff_t;

// Special character #defines
#define SPACE_CHAR ' '
#define PIPE_CHAR '|'
#define PIPE_STRING "|"
#define SH_PROMPT "dsh2> "
#define EXIT_CMD "exit"

// Return codes
#define OK 0
#define WARN_NO_CMDS -1
#define ERR_TOO_MANY_COMMANDS -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3
#define ERR_CMD_ARGS_BAD -4
#define ERR_MEMORY -5
#define ERR_EXEC_CMD -6
#define OK_EXIT -7

// Output messages
#define CMD_OK_HEADER "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT "error: piping limited to %d commands\n"
#define CMD_ERR_EXECUTE "error: command execution failed\n"

// Built-in commands enum
typedef enum {
    BI_CMD_EXIT,
    BI_CMD_CD,
    BI_NOT_BI,
    BI_EXECUTED,
    BI_RC
} Built_In_Cmds;

// Function prototypes
int alloc_cmd_buff(cmd_buff_t *cmd_buff);
int free_cmd_buff(cmd_buff_t *cmd_buff);
int clear_cmd_buff(cmd_buff_t *cmd_buff);
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff);
Built_In_Cmds match_command(const char *input);
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd);
int exec_local_cmd_loop(void);

#endif