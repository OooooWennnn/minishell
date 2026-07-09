#include "../../inc/minishell.h"
#include <stdio.h>
#include <string.h>

int builtin_unset(char **args, t_env **env_list) {
    int i;

    if (!env_list || !args) return 0;

    i = 1;
    while (args[i]) {
        remove_env_node (args[i], env_list);
        i++;
    }
    return 1;
}