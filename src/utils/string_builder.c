#include "../../inc/minishell.h"

int sb_init (t_builder *sb) {
    sb->capacity = 64;
    sb->len = 0;

    sb->str = malloc(sizeof(char) * sb->capacity);
    if (!sb->str) return 0;

    sb->str[0] = '\0';
    return 1;
}

static int sb_grow (t_builder *sb) {
    char *tmp;
    int new_capacity = sb->capacity * 2;

    tmp = realloc (sb->str, new_capacity);
    if (!tmp) return 0;

    sb->str = tmp;
    sb->capacity = new_capacity;
    return 1;
}

int sb_append_char (t_builder *sb, char c) {
    if (!sb || !sb->str) return 0;

    if (sb->len >= sb->capacity - 1) {
        if (sb_grow(sb) == 0) return 0;
    }
    sb->str[sb->len] = c;
    sb->len++;
    sb->str[sb->len] = '\0';

    return 1;
}

int sb_append_str (t_builder *sb, const char* str) {
    if (!str || !sb || !sb->str) return 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (!sb_append_char(sb, str[i])) return 0;
    }
    return 1;
}
