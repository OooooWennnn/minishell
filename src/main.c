#include "../inc/minishell.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

// gcc -Wall $(find src -name "*.c") -I inc -lreadline -o minishell

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

        if (*input != '\0') {
            add_history(input);

            t_token *tokens = tokenize(input);
            if (tokens != NULL) {
                t_ast_node *ast = generate_ast(tokens);

                if (ast != NULL) {
                    printf("\n=== 🛠️ AST DEBUG 🛠️ ===\n");
                    print_ast(ast, 0);
                    printf("========================\n\n");

                    // expand_ast(ast, )
                    execute_ast(ast, &env_list);

                    free_ast(ast);
                }
            }
        }
        free(input);
    }
}