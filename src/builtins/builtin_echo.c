#include "../../inc/minishell.h"
#include <stdio.h>

int builtin_echo(char **args, t_env **env_list) {

    printf("builtin_echo function called\n");
    return 1;

}