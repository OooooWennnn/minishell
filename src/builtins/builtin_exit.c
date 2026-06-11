#include "../../inc/minishell.h"
#include <stdio.h>

int builtin_exit(char **args, t_env **env_list) {
    

    printf("builtin_exit function called\n");
    return 1;
}