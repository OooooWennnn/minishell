#include "../../inc/minishell.h"
#include <stdio.h>

int builtin_echo(char **args, t_env **env_list) {
    int i = 1;

    // TODO: add -n option (if -n, no \n at the end of loop)

    while (args[i] != NULL) {
        if (i != 1) {
            printf(" ");
        }

        printf("%s", args[i]);
        i++;
    }
    
    printf("\n");
    return 0;
}