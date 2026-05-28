# ifndef MINISHELL_H
# define MINISHELL_H

// 1. Environment variables key-value linked list
typedef struct s_env {
    char* key;
    char* value;
    struct s_env* next;
} t_env;

// 2. Lexer
typedef enum e_token_type {
    TOKEN_WORD,
    TOKEN_PIPE,         // |
    TOKEN_REDIR_IN,     // <
    TOKEN_REDIR_OUT,    // >
    TOKEN_APPEND,       // >>
    TOKEN_HEREDOC       // <<
} t_token_type;

typedef struct s_token {
    t_token_type type;
    char *value;
    struct s_token *next;
} t_token;


// env_utils functions
t_env *parse_env_node(const char* str);
t_env *envnew(char *key, char *value);
void envadd_back(t_env **env_list, t_env *new_node);

t_token *tokenize(char *input);

# endif