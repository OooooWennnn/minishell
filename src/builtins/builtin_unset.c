#include "../../inc/minishell.h"
#include <stdio.h>

int builtin_unset(char **args, t_env **env_list) {
    

    printf("builtin_unset function called\n");
    return 1;
}