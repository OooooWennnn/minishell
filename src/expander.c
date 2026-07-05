#include "../inc/minishell.h"
#include <stdlib.h>

#define PROTECTED_SPACE '\x01'

int is_env_char(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_') {
        return 1;
    }
    return 0;
}

static void restore_protected_space(char *s)
{
    int i = 0;

    while (s[i])
    {
        if (s[i] == PROTECTED_SPACE)
            s[i] = ' ';
        i++;
    }
}

char *expand_string(char *arg, t_env **env_list)
{
    int quote_state = NORMAL;
    int i = 0;
    t_builder sb;

    if (!sb_init(&sb))
        return NULL;

    while (arg[i])
    {
        if (arg[i] == '\'' || arg[i] == '"')
        {
            quote_state = update_quote_state(arg[i], quote_state);
            i++;
            continue;
        }

        if (arg[i] == ' ' && quote_state != NORMAL)
        {
            if (!sb_append_char(&sb, PROTECTED_SPACE))
            {
                free(sb.str);
                return NULL;
            }
            i++;
            continue;
        }

        if (arg[i] == '$' && quote_state != SINGLE_QUOTE)
        {
            int len = 0;
            char *start = &arg[i + 1];

            while (is_env_char(start[len]))
                len++;

            if (len == 0)
            {
                if (!sb_append_char(&sb, '$'))
                {
                    free(sb.str);
                    return NULL;
                }
                i++;
                continue;
            }

            char temp = start[len];
            start[len] = '\0';

            char *env_val = get_env_value(start, env_list);

            start[len] = temp;

            if (env_val)
            {
                int j = 0;
                while (env_val[j])
                {
                    if (env_val[j] == ' ' && quote_state != NORMAL)
                    {
                        if (!sb_append_char(&sb, PROTECTED_SPACE))
                        {
                            free(sb.str);
                            return NULL;
                        }
                    }
                    else
                    {
                        if (!sb_append_char(&sb, env_val[j]))
                        {
                            free(sb.str);
                            return NULL;
                        }
                    }
                    j++;
                }
            }

            i += len + 1;
            continue;
        }

        if (!sb_append_char(&sb, arg[i]))
        {
            free(sb.str);
            return NULL;
        }
        i++;
    }

    return sb.str;
}

int space_exists (char *arg) {
    for (int i = 0; arg[i] != '\0'; i++) {
        if (arg[i] == ' ') {
            return 1;
        }
    }
    return 0;
}

static void free_str_arr(char **tokens, int len) {
    if (!tokens) return;
    for (int i = 0; i < len; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

char **split_string(char *str, char dil)
{
    int i = 0;
    int tok_len = 0;
    char **tokens = NULL;
    t_builder sb;

    if (!sb_init(&sb))
        return NULL;

    while (str[i])
    {
        if (str[i] == dil)
        {
            char **tmp;

            if (sb.len > 0)
            {
                tmp = realloc(tokens, sizeof(char *) * (tok_len + 2));
                if (!tmp)
                {
                    free_str_arr(tokens, tok_len);
                    free(sb.str);
                    return NULL;
                }
                tokens = tmp;
                tokens[tok_len++] = sb.str;

                if (!sb_init(&sb))
                {
                    free_str_arr(tokens, tok_len);
                    return NULL;
                }
            }
            i++;
            continue;
        }

        if (!sb_append_char(&sb, str[i]))
        {
            free(sb.str);
            free_str_arr(tokens, tok_len);
            return NULL;
        }
        i++;
    }

    if (sb.len > 0)
    {
        char **tmp = realloc(tokens, sizeof(char *) * (tok_len + 2));
        if (!tmp)
        {
            free(sb.str);
            free_str_arr(tokens, tok_len);
            return NULL;
        }
        tokens = tmp;
        tokens[tok_len++] = sb.str;
    }
    else
        free(sb.str);

    if (!tokens)
    {
        tokens = malloc(sizeof(char *) * 1);
        if (!tokens)
            return NULL;
    }

    tokens[tok_len] = NULL;
    return tokens;
}

int count_tokens (char **args) {
    int count = 0;
    for (int i = 0; args[i] != NULL; i++) {
        count++;
    }
    return count;
}

char **splice_tokens (char **args, char **split_tokens, int i) {
    int tok_len = count_tokens (split_tokens);
    int arg_len = count_tokens (args);

    char **new_args = (char**)malloc(sizeof(char*) * (arg_len + tok_len));
    if (!new_args) return NULL;

    int j = 0;
    while (j < i) {
        new_args[j] = args[j];
        j++;
    }

    int k = 0;
    while (k < tok_len) {
        new_args[j++] = split_tokens[k++];
    }

    i++;
    while (args[i] != NULL) {
        new_args[j++] = args[i++];
    }

    new_args[j] = NULL;

    free(args);
    free(split_tokens);

    return new_args;
}


int expand_cmd_args(t_ast_node *node, t_env **env_list)
{
    char **args = node->args;
    int i = 0;

    while (args[i])
    {
        char *expanded = expand_string(args[i], env_list);
        if (!expanded)
            return 1;

        if (space_exists(expanded))
        {
            char **split_tokens = split_string(expanded, ' ');
            if (!split_tokens)
            {
                free(expanded);
                return 1;
            }

            int tok_len = count_tokens(split_tokens);

            for (int k = 0; split_tokens[k]; k++)
                restore_protected_space(split_tokens[k]);

            char *old_arg = args[i];
            char **new_args = splice_tokens(args, split_tokens, i);
            if (!new_args)
            {
                free(expanded);
                return 1;   
            }

            args = new_args;
            free(old_arg);
            free(expanded);

            i += tok_len;
        }
        else
        {
            restore_protected_space(expanded);
            free(args[i]);
            args[i] = expanded;
            i++;
        }
    }

    node->args = args;
    return 0;
}

int expand_ast (t_ast_node *node, t_env **env_list) {
    if (node == NULL) {
        return 0;
    }

    if (expand_ast(node->left, env_list) != 0) {
        return 1;
    }
    if (expand_ast(node->right, env_list) != 0) {
        return 1;
    }
    
    if (node->type == NODE_CMD) {
        if (expand_cmd_args(node, env_list) != 0) {
            return 1;
        }
    }
    
    return 0;
}