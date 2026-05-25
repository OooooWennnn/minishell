#include "../inc/minishell.h"
#include <stdio.h>

int main(int argc, char **argv, char **envp)
{
    t_env *env_list = NULL;
    int i = 0;

    while (envp[i] != NULL) {
        t_env *node = parse_env_node(envp[i]);

        if (node != NULL) {
            envadd_back(&env_list, node);
        }
        i++;
    }

    while(1) {

    }
}