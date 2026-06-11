#include "../../inc/minishell.h"
#include <stdio.h>

int builtin_export(char **args, t_env **env_list) {
    

    printf("builtin_export function called\n");
    return 1;
}