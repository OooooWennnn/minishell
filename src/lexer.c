#include "../inc/minishell.h"
#include <stdio.h>
#include <stdlib.h>

void skip_whitespaces (char *input, int *i) {
    while (input[*i] == ' ' || input[*i] == '\t') {
        (*i)++;
    }
}

int is_operator (char c) {
    return (c == '|' || c == '<' || c == '>');
}

t_token *extract_word (char *input, int *i) {
    int len = 0;
    int start = *i;

    while (input[*i] != '\0' && input[*i] != ' ' && input[*i] != '\t' && !is_operator(input[*i])) {
        len++;
        (*i)++;
    }

    t_token *new_token = (t_token*)malloc(sizeof (t_token));
    if (!new_token) {
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
    new_token->type = TOKEN_WORD;
    new_token->next = NULL;

    return new_token;
}

t_token *extract_operator (char *input, int *i) {
    t_token *new_token = (t_token*)malloc(sizeof(t_token));
    if (!new_token) {
        return NULL;
    }

    if ((input[*i] == '>' && input[*i + 1] == '>') || (input[*i] == '<' && input[*i + 1] == '<')) {
        new_token->value = (char*)malloc(sizeof(char) * 3);
        if (!new_token->value) {
            free(new_token);
            return NULL;
        }
        
        if (input[*i] == '>' && input[*i + 1] == '>') {
            new_token->type = TOKEN_APPEND;
            new_token->value[0] = '>';
            new_token->value[1] = '>';
        }
        else if (input[*i] == '<' && input[*i + 1] == '<') {
            new_token->type = TOKEN_HEREDOC;
            new_token->value[0] = '<';
            new_token->value[1] = '<';
        }
        new_token->value[2] = '\0';
        (*i) += 2;
    }
    else {
        new_token->value = (char*)malloc(sizeof(char) * 2);
        if (!new_token->value) {
            free(new_token);
            return NULL;
        }

        if (input[*i] == '>') {
            new_token->type = TOKEN_REDIR_OUT;
            new_token->value[0] = '>';
        }
        else if (input[*i] == '<') {
            new_token->type = TOKEN_REDIR_IN;
            new_token->value[0] = '<';
        }
        else if (input[*i] == '|') {
            new_token->type = TOKEN_PIPE;
            new_token->value[0] = '|';
        }
        new_token->value[1] = '\0';
        (*i)++;
    }
    new_token->next = NULL;
    return new_token;
}

void token_add_back (t_token **head, t_token *new_token) {
    if (head == NULL || new_token == NULL) {
        return;
    }

    if (*head == NULL) {
        *head = new_token;
        return;
    }

    t_token *curr = *head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new_token;
}

t_token *tokenize(char *input) {
    t_token *head = NULL;
    int i = 0;

    while (input[i] != '\0') {
        skip_whitespaces (input, &i);

        t_token *new_token = NULL;

        if (is_operator(input[i])) {
            new_token = extract_operator (input, &i);
        } 
        else {
            new_token = extract_word (input, &i);
        }

        token_add_back(&head, new_token);
        printf("TYPE: %d | VALUE: \"%s\"\n", new_token->type, new_token->value);
    }
    return head;
} 
