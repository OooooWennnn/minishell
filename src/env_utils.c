#include "../inc/minishell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Parses a string of the form "KEY=VALUE" and returns a pointer to a newly allocated t_env struct.
 * The struct contains separate key and value strings.
 * The caller is responsible for freeing the returned t_env struct and its key/value members.
 *
 * @param str The input string in the format "KEY=VALUE".
 * @return A pointer to a newly allocated t_env struct, or NULL on failure.
 */
t_env *parse_env_node(const char* str) {
    int i = 0;
    int j = 0;
    char *key;
    char *value;

    // find the index of the '='
    while (str[i] != '\0' && str[i] != '=') {
        i++;
    }

    key = (char*)malloc((i + 1) * sizeof(char));
    if (key == NULL) {
        return NULL;
    }

    while (j < i) {
        key[j] = str[j];
        j++;
    }
    key[i] = '\0';

    if (str[i] == '\0') {
        value = NULL;
    }
    else {
        i++;
        int val_len = 0;
        while (str[i + val_len] != '\0') {
            val_len++;
        }

        value = (char*)malloc(sizeof(char) * (val_len + 1));
        if (value == NULL) {
            free(key);
            return NULL;
        }

        int k = 0;
        while (k < val_len) {
            value[k] = str[i + k];
            k++;
        }
        value[k] = '\0';
    }

    t_env *node = envnew(key, value);
    if (node == NULL) {
        free(key);
        if (value != NULL) {
            free(value);
        }
        return NULL;
    }

    return node;
}

/**
 * Allocates and initializes a new t_env struct with the given key and value.
 * The key and value pointers are assigned directly; caller must ensure their validity.
 *
 * @param key The environment variable key (string).
 * @param value The environment variable value (string).
 * @return A pointer to the newly allocated t_env struct, or NULL on failure.
 */
t_env *envnew(char *key, char *value) {
    t_env *new_node;

    // allocate memory 
    new_node = (t_env*)malloc(sizeof(t_env));
    if(new_node == NULL) {
        return NULL;
    }

    // assign key and value
    new_node->key = key;
    new_node->value = value;
    new_node->next = NULL;

    return new_node;
}

/**
 * Adds a new t_env node to the end of the environment variable linked list.
 *
 * @param env_list Pointer to the head pointer of the environment variable list.
 * @param new_node Pointer to the new t_env node to be added.
 */
void envadd_back(t_env **env_list, t_env *new_node) {
    if (env_list == NULL || new_node == NULL) {
        return;
    }

    if (*env_list == NULL) {
        *env_list = new_node;
        return;
    }

    t_env *curr = *env_list;
    while (curr->next != NULL) {
        // printf("%s=%s\n", curr->key, curr->value);
        curr = curr->next;
    }

    curr->next = new_node;
}

void free_env_node (t_env *node) {
    if (!node) return;
    free(node->key);
    free(node->value);
    free(node);
}

char *get_env_value(const char *key, t_env **env_list) {
    if (env_list == NULL || *env_list == NULL) {
        return NULL;
    }

    t_env *curr = *env_list;
    while (curr != NULL) {
        if (strncmp(curr->key, key, strlen(key) + 1) == 0) {
            return curr->value;
        }
        curr = curr->next;
    }
    return NULL;
}

void update_env_value(const char *key, const char *value, t_env **env_list) {
    t_env *curr = *env_list;

    while (curr != NULL) {
        if (strncmp(curr->key, key, strlen(key) + 1) == 0) {
            if (curr->value != NULL) {
                free(curr->value);
            }

            curr->value = strdup(value);
            return;
        }
        curr = curr->next;
    }
}

// remove a node with matching key from env list, return 1 on success
int remove_env_node (char *key, t_env **env_list) {
    t_env *curr;
    t_env *prev;

    if (!env_list || !*env_list || !key) return 0;

    curr = *env_list;
    prev = NULL;

    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            if (prev == NULL) {
                *env_list = curr->next;
            }
            else {
                prev->next = curr->next;
            }

            free_env_node(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
}

// Count total env list
int env_count (t_env *env) {
    int count = 0;
    while (env) {
        count++;
        env = env->next;
    }
    return count;
}

// Consolidate key and value of a node and return as a string
char *env_to_string (t_env *node) {
    if (!node) return NULL;

    size_t key_len = strlen(node->key);
    size_t val_len = strlen(node->value);

    int len = key_len + val_len + 2;
    char *str = (char*)malloc(sizeof(char) * len);
    if (!str) return NULL;

    size_t i = 0;
    while (i < key_len) {
        str[i] = node->key[i];
        i++;
    }
    str[i++] = '=';

    size_t j = 0;
    while (j < val_len) {
        str[i] = node->value[j];
        i++;
        j++;
    }
    str[i] = '\0';

    return str;
}

void envp_free (char **envp) {
    if (!envp) return;
    for (int i = 0; envp[i] != NULL; i++) {
        free(envp[i]);
    }
    free(envp);
}

// Convert environment list to array
char **env_list_to_array (t_env *env_list) {
    int count;
    char **envp;
    int i;

    if (!env_list) return NULL;

    count = env_count (env_list);
    envp = (char**)calloc((count + 1), sizeof(char*));
    if (!envp) return NULL;

    i = 0;
    while (i < count) {
        envp[i] = env_to_string(env_list);
        if (!envp[i]) {
            envp_free (envp);
            return NULL;
        } 
        env_list = env_list->next;
        i++;
    }
    envp[i] = NULL;

    return envp;
}