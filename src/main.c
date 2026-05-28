#include "../inc/minishell.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

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
        char *input;
        input = readline("minishell> ");

        if (input == NULL) {
            printf("exit\n");
            break;
        }

        if (*input != '\n') {
            t_token *tokens;

            tokens = tokenize(input);
        }

        free(input);

    }
}