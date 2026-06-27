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