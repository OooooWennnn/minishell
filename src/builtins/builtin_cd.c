#include "../../inc/minishell.h"
#include <stdio.h>

int builtin_cd(char **args, t_env **env_list) {

    printf("builtin_cd function called\n");
    return 1;

}