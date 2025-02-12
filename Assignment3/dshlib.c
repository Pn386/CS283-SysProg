#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

static char* trim(char* str) {
    if (!str) return NULL;
    
    // Trim leading spaces
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return str;
    
    // Trim trailing spaces
    char* end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    
    return str;
}

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    char *cmd_copy, *cmd_ptr;
    char *token;
    char *saveptr1;
    int cmd_count = 0;
    
    memset(clist, 0, sizeof(command_list_t));
    
    if (!cmd_line || !(cmd_ptr = trim(cmd_line)) || strlen(cmd_ptr) == 0) {
        return WARN_NO_CMDS;
    }
    
    cmd_copy = strdup(cmd_ptr);
    if (!cmd_copy) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }
    
    token = strtok_r(cmd_copy, PIPE_STRING, &saveptr1);
    while (token) {
        char *cmd = trim(token);
        char *args = strchr(cmd, SPACE_CHAR);
        
        if (cmd_count >= CMD_MAX) {
            free(cmd_copy);
            return ERR_TOO_MANY_COMMANDS;
        }
        
        if (args) {
            *args++ = '\0';
            args = trim(args);
            
            if (strlen(cmd) >= EXE_MAX || strlen(args) >= ARG_MAX) {
                free(cmd_copy);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            
            strcpy(clist->commands[cmd_count].exe, cmd);
            strcpy(clist->commands[cmd_count].args, args);
        } else {
            if (strlen(cmd) >= EXE_MAX) {
                free(cmd_copy);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            
            strcpy(clist->commands[cmd_count].exe, cmd);
            clist->commands[cmd_count].args[0] = '\0';
        }
        
        cmd_count++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr1);
    }
    
    clist->num = cmd_count;
    free(cmd_copy);
    return OK;
}