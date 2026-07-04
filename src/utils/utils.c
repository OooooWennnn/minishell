#include "../inc/minishell.h"
#include <stdio.h>
#include <stdlib.h>

void free_tokens (t_token *head) {
    if (head == NULL) return;

    t_token *next = head;
    while (next != NULL) {
        next = next->next;
        if (head->value) {
            free(head->value);
        }
        free(head);
        head = next;
    }
}

int update_quote_state(char c, int current_state) {
    if (c == '\"') {
        if (current_state == 0) {
            return 2; 
        } else if (current_state == 2) {
            return 0; 
        }
    } else if (c == '\'') {
        if (current_state == 0) {
            return 1; 
        } else if (current_state == 1) {
            return 0; 
        }
    }
    return current_state;
}

int update_string (char *str, char *new_str) {
    int len = 0;
    while (new_str[len] != '\0') {
        len++;
    }

    char *tmp = (char*)malloc(sizeof(char) * (len + 1));
    if (tmp) {
        return NULL;
    }
    new_token->value = (char*)malloc(sizeof(char) * (len + 1));
    if (!new_token->value) {
        free(new_token);
        return NULL;
    }

    int j = 0;
    while (j < len) {
        new_token->value[j] = input[start + j];
        j++;
    }
    new_token->value[j] = '\0';
}