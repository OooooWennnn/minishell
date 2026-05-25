# ifndef MINISHELL_H
# define MINISHELL_H

// 1. Environment variables key-value linked list
typedef struct s_env {
    char* key;
    char* value;
    struct s_env* next;
} t_env;

// 2. Lexer
typedef struct s_token {

} t_token;

t_env *parse_env_node(const char* str);
t_env *envnew(char *key, char *value);
void envadd_back(t_env **env_list, t_env *new_node);


# endif