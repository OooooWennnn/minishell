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

// 3. Parser (AST)
typedef enum e_node_type {
    NODE_PIPE,
    NODE_CMD,
    NODE_REDIR
} t_node_type;

typedef struct s_ast_node {
    t_node_type type;

    // used by NODE_CMD
    char **args;

    // used by NODE_REDIR
    t_token_type redir_type;
    char *value;

    struct s_ast_node *left;
    struct s_ast_node *right;
} t_ast_node;

typedef int (*t_builtin_func)(char **args, t_env **env_list);

typedef struct s_builtin {
    char *cmd_name;
    t_builtin_func func;
} t_builtin;


// env_utils functions
t_env *parse_env_node(const char* str);
t_env *envnew(char *key, char *value);
void envadd_back(t_env **env_list, t_env *new_node);
char *get_env_value(const char *key, t_env **env_list);
void update_env_value(const char *key, const char *value, t_env **env_list);

// lexer functions
t_token *tokenize(char *input);

// parser functions
t_ast_node *generate_ast (t_token *head);
void free_ast (t_ast_node *node);
void print_ast(t_ast_node *node, int depth);

// executor functions
void execute_ast (t_ast_node *node, t_env *env_list);

// builtin cmd functions
int builtin_echo(char **args, t_env **env_list);
int builtin_cd(char **args, t_env **env_list);
int builtin_pwd(char **args, t_env **env_list);
int builtin_export(char **args, t_env **env_list);
int builtin_unset(char **args, t_env **env_list);
int builtin_env(char **args, t_env **env_list);
int builtin_exit(char **args, t_env **env_list);

# endif