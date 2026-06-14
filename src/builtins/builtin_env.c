#include "../../inc/minishell.h"
#include <stdio.h>

int builtin_env(char **args, t_env **env_list) {
    if (args[1] != NULL) {
        fprintf(stderr, "env : options are not supported\n");
        return 1;
    }

    t_env *curr = *env_list;
    while (curr != NULL) {
        if (curr->value != NULL) {
            printf("%s=%s\n", curr->key, curr->value);
        }
        curr = curr->next;
    }
    return 0;
}