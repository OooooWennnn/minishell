#include "../../inc/minishell.h"
#include <stdio.h>

int g_should_exit = 0;

int builtin_exit(char **args, t_env **env_list) {

    if (args[1] != NULL) {
        fprintf(stderr, "minishell: exit: arguments not supported\n");
        g_exit_code = 1;
        return 1;
    }

    g_should_exit = 1;
    return g_exit_code;
}