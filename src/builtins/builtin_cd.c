#include "../../inc/minishell.h"
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int builtin_cd(char **args, t_env **env_list) {
    char *target_path;

    if (args[1] == NULL) {
        target_path = get_env_value("HOME", env_list);

        if (target_path == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    }
    else if (args[2] != NULL) {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
    }
    else {
        target_path = args[1];
    }

    if (chdir(target_path) != 0) {
        perror("minishell: cd");
        return 1;
    }

    char cwd[PATH_MAX];
    char *prev_pwd = get_env_value("PWD", env_list);

    if (prev_pwd != NULL) {
        update_env_value("OLDPWD", prev_pwd, env_list);
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        update_env_value("PWD", cwd, env_list);
    }
    else {
        perror("minishell: getcwd failed");
    }

    return 0;
}