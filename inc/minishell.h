# ifndef MINISHELL_H
# define MINISHELL_H

// string builder
typedef struct s_builder {
    char *str;
    int len;
    int capacity;
} t_builder;

typedef enum e_quote {
    NORMAL,
    SINGLE_QUOTE,
    DOUBLE_QUOTE
} t_quote;

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

// builtin cmd functions
int builtin_echo(char **args, t_env **env_list);
int builtin_cd(char **args, t_env **env_list);
int builtin_pwd(char **args, t_env **env_list);
int builtin_export(char **args, t_env **env_list);
int builtin_unset(char **args, t_env **env_list);
int builtin_env(char **args, t_env **env_list);
int builtin_exit(char **args, t_env **env_list);

// utils functions
void free_tokens (t_token *head);
int update_quote_state(char c, int current_state);
int update_string (char **str, char *new_str);


// env_utils functions
t_env *parse_env_node(const char* str);
t_env *envnew(char *key, char *value);
int append_env_node(t_env **env_list, t_env *new_node);
char *get_env_value(const char *key, t_env **env_list);
t_env *find_env_node(t_env *env, char *key);
int update_env_value(t_env *node, char *value);
void envp_free (char **envp);
char **env_list_to_array (t_env *env_list);
int remove_env_node (char *key, t_env **env_list);

// lexer functions
t_token *tokenize(char *input);

// parser functions
t_ast_node *generate_ast (t_token *head);
void free_ast (t_ast_node *node);
void print_ast(t_ast_node *node, int depth);

// expander functions
int expand_ast (t_ast_node *node, t_env **env_list);

// executor functions
void execute_ast (t_ast_node *node, t_env **env_list);

// string builder functions
int sb_init (t_builder *sb);
int sb_append_char (t_builder *sb, char c);
int sb_append_str (t_builder *sb, const char* str);

# endif