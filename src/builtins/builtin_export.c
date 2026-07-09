#include "../../inc/minishell.h"
#include <stdio.h>
#include <stdlib.h>

int is_valid_identifier(char *key);
char *find_equal(char *arg);
static char *extract_key(char *arg, char* eq);
static char *extract_value(char *arg);

int builtin_export(char **args, t_env **env_list) {
    char *key;
    char *value;
    char *eq;
    t_env *node;
    int i;
    int status = 0;

    if (!env_list || !args) return 0;

    i = 1;
    while(args[i]) {
        eq = find_equal(args[i]);
        // if equal sign is missing (ex. export TEST), skip
        // TODO: upgrade to handle export variables
        if (!eq) {
            i++;
            continue;
        }

        key = extract_key(args[i], eq);
        if (!key) {
            status = 1;
            i++;
            continue;
        }
        if (!is_valid_identifier(key)) {
            fprintf(stderr, "export: '%s' is not a valid identifier\n", args[i]);
            free(key);
            status = 1;
            i++;
            continue;
        }

        value = extract_value(eq);
        if (!value) {
            free(key);
            status = 1;
            i++;
            continue;
        }

        // receives null if not found
        node = find_env_node(*env_list, key);

        if (node) {
            if (!update_env_value(node, value)) {
                status = 1;
            }
            free(key);
            free(value);
        } else {
            node = envnew(key, value);
            if (!node) {
                free(key);
                free(value);
                status = 1;
                i++;
                continue;
            }
            if (!append_env_node(env_list, node)) {
                status = 1;
            }
        }
        i++;
    }

    return status;
}

int is_valid_identifier(char *key) {
    int i;

    if (!((key[0] >= 'a' && key[0] <= 'z') || (key[0] >= 'A' && key[0] <= 'Z') || (key[0] == '_')))
        return 0;

    i = 1;
    while(key[i]) {
        if (!((key[i] >= 'a' && key[i] <= 'z') || (key[i] >= 'A' && key[i] <= 'Z') || (key[i] == '_') || (key[i] >= '0' && key[i] <= '9'))) {
            return 0;
        }
        i++;
    }
    
    return 1;
}

char *find_equal(char *arg) {
    int i = 0;

    while (arg[i] != '\0') {
        if (arg[i] == '=') {
            return &arg[i];
        }
        i++;
    }
    return NULL;
}

static char *extract_key(char *arg, char *eq) {
    char *key;
    int len;
    int i;

    len = eq - arg;
    key = (char *)malloc(sizeof(char) * (len + 1));
    if (!key) return NULL;

    i = 0;
    while (i < len) {
        key[i] = arg[i];
        i++;
    }
    key[i] = '\0';

    return key;
}

// takes arg starting from where '=' is
static char *extract_value(char *arg) {
    char *value;
    int len;
    int i;

    len = 0;
    while (arg[len + 1] != '\0') {
        len++;
    }

    value = (char *)malloc(sizeof(char) * (len + 1));
    if (!value) return NULL;

    i = 0;
    while (i < len) {
        value[i] = arg[i + 1];
        i++;
    }
    value[i] = '\0';

    return value;
}