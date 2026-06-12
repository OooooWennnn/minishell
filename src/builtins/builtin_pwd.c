#include "../../inc/minishell.h"
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int builtin_pwd(char **args, t_env **env_list) {
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    }
    else {
        perror("minishell: pwd");
        return 1;
    }
}