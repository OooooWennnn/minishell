#include "../include/minishell.h"

int is_env_char(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_') {
        return 1;
    }
    return 0;
}

char *expand_string(char *arg, t_env **env_list) {
    int quote_state = NORMAL;
    int i = 0;
    
    t_builder sb;
    if (!sb_init(&sb)) return NULL;
    
    while (arg[i] != '\0') {
        
        // TODO: 이거 split 함수에 옯기셈 split -> quote 제거 
        // skip quotes 
        if (arg[i] == '\"' || arg[i] == '\'') {
            quote_state = update_quote_state(arg[i], quote_state);
            i++;
            continue;
        }
 
        // no quote, double quote -> expand env
        if (arg[i] == '$' && quote_state != SINGLE_QUOTE) {

            // extract env variable key
            int len = 0;
            char *start = &arg[i + 1];
            while (is_env_char(start[len])) {
                len++;
            }

            if (len == 0) {
                sb_append_char (&sb, '$');
                i++;
                continue;
            }

            //TODO: implement $? (exit status)
            
            char temp = start[len];
            start[len] = '\0';
            char *env_val = get_env_value (start, env_list);
            start[len] = temp;
            
            // append expaned env value to string
            if (env_val != NULL) {
                sb_append_str(&sb, env_val);
            }
            
            i += len + 1;   // skip $ too
        } else {
            // append each char
            sb_append_char(&sb, arg[i]);
            i++;
        }
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

char **split_string (char* str, char dil) {
    int words = 0;
    // for (int i = 0; )


    char **tokens;
    int len = 0;

    int tok_count = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        len++;
        if (str[i] == dil) {
            char *tok = (char *)malloc(sizeof(char) * (len + 1));
            for (int j = 0; j < len; j++) {
                tok[j] = str[j];
            }
            tok[j] = '\0';
            tokens[tok_count] = tok;
            tok_count++;
            i++;
            len = 0;
        }
    }
    return tokens;
}


int expand_cmd_args (char **args, t_env **env_list) {
    for (int i = 0; args[i] != NULL; i++) {
        char *expanded = expand_string(args[i], env_list);
        if (expanded == NULL) {
            return 1;
        }

        if (space_exists(expanded)) {
            char **split_tokens = split_string(expanded, ' ');
            args = insert_split_tokens (args, split_tokens, i);
            i += count_tokens(split_tokens) - 1;
            free(expanded);
        }
        else {
            free(args[i]);
            args[i] = expanded;
        }
    }
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
        if (expand_cmd_args(node->args, env_list) != 0) {
            return 1;
        }
    }
    
    return 0;
}