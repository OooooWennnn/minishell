#include "../../inc/minishell.h"
#include <stdio.h>

int builtin_echo(char **args, t_env **env_list) {

    // loop through the arguments
    // trim " from the arguments
    // if an argument is env variable, replace with the env value

    printf("builtin_echo function called\n");
    return 1;

}