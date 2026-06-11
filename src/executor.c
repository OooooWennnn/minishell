#include "../inc/minishell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

// builtin functions return 1 on success
t_builtin g_builtins[] = {
    {"echo", builtin_echo},
    {"cd", builtin_cd},
    {"pwd", builtin_pwd},
    {"export", builtin_export},
    {"unset", builtin_unset},
    {"env", builtin_env},
    {"exit", builtin_exit},
    {NULL, NULL}
};

int run_builtin (char **args, t_env **env_list) {
    if (args == NULL || args[0] == NULL) {
        return 0;
    }

    int i = 0;
    while (g_builtins[i].cmd_name != NULL) {
        if (strncmp(args[0], g_builtins[i].cmd_name, strlen(args[0]) + 1) == 0) {
            
            printf("command %s matched with %s\n", args[0], g_builtins[i].cmd_name);

            // int exit_code = g_builtins[i].func(args, env_list);

            // Update global exit code for $?
            // 0 on success 1 on fail
            // g_exit_code = exit_code;
            return 1;
        }
        i++;
    }
    return 0;
}

void execute_cmd(t_ast_node *node, t_env **env_list) {
    if (node->args == NULL || node->args[0] == NULL) return;

    // if built-in, execute built-in cmd logic
    if (run_builtin(node->args, env_list) == 1) {
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork error");
        return;
    }
    else if (pid == 0) {
        // child process
        printf("child process (pid: %d)\n", getpid());

    }
    else {
        int status;
        printf("parent process (child pid: %d)\n", pid);
        waitpid(pid, &status, 0);
    }
}

void execute_ast (t_ast_node *node, t_env *env_list) {
    if (node == NULL) {
        return;
    }

    if (node->type == NODE_PIPE) {
        // execute_pipe(node, env_list);
    }
    else if (node->type == NODE_REDIR) {
        // execute_redir(node, env_list);
    }
    else if (node->type == NODE_CMD) {
        execute_cmd(node, &env_list);
    }

}