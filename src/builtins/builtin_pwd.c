#include "../../inc/minishell.h"
#include <stdio.h>

int builtin_pwd(char **args, t_env **env_list) {
    

    printf("builtin_pwd function called\n");
    return 1;
}