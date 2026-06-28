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