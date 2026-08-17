#include "../inc/minishell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

// builtin functions return 1 on success
t_builtin g_builtins[] = {
    {"echo", builtin_echo},
    {"cd", builtin_cd},
    {"pwd", builtin_pwd},
    {"export", builtin_export},
    {"unset", builtin_unset},
    {"env", builtin_env},
    {"exit", builtin_exit},
    {NULL, NULL}
};

int run_builtin (char **args, t_env **env_list) {
    if (args == NULL || args[0] == NULL) {
        return 0;
    }

    int i = 0;
    while (g_builtins[i].cmd_name != NULL) {
        if (strncmp(args[0], g_builtins[i].cmd_name, strlen(args[0]) + 1) == 0) {
            
            printf("command %s matched with %s\n", args[0], g_builtins[i].cmd_name);

            int exit_code = g_builtins[i].func(args, env_list);

            // Update global exit code for $?
            // 0 on success 1 on fail
            g_exit_code = exit_code;
            return 1;
        }
        i++;
    }
    return 0;
}

static char *join_path (const char *dir, const char *cmd) {
    t_builder sb;
    if (!sb_init(&sb)) return NULL;
    if (!sb_append_str(&sb, dir)) {
        free(sb.str);
        return NULL;
    }
    if (!sb_append_char(&sb, '/')) {
        free(sb.str);
        return NULL;
    }
    if (!sb_append_str(&sb, cmd)) {
        free(sb.str);
        return NULL;
    }
    return sb.str;
}

char *find_cmd_path (char *cmd, t_env **env_list) {
    // if cmd contains '/'
    if (strchr(cmd, '/'))
    {
        if (access(cmd, X_OK) == 0)
            return strdup(cmd);
        return NULL;
    }

    char *full_path = NULL;
    char *path_val = get_env_value("PATH", env_list);
    if (path_val == NULL) {
        return NULL;
    }
    
    int i = 0;
    t_builder sb;
    
    if (!sb_init(&sb))
        return NULL;
    
    // split, join path with cmd, and check if it is valid
    while (1) {
        if (path_val[i] != ':' && path_val[i] != '\0') {
            if (!sb_append_char(&sb, path_val[i])) {
                free(sb.str);
                return NULL;
            }
        }
        else {
            full_path = join_path(sb.str, cmd);
            free(sb.str);

            if (full_path && access(full_path, X_OK) == 0) {
                return full_path;
            }
            else {
                free(full_path);
                if (path_val[i] == '\0') break;
                if (!sb_init(&sb)) return NULL;
            }
        }
        
        i++;
    }
    return NULL;
}

void execute_cmd(t_ast_node *node, t_env **env_list) {
    char *path = NULL;
    char **envp = NULL;
    pid_t pid;

    if (node->args == NULL || node->args[0] == NULL) return;

    // if built-in, execute built-in cmd logic
    if (run_builtin(node->args, env_list) == 1) {
        return;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork error");
        return;
    }
    else if (pid == 0) {
        // child process
        path = find_cmd_path(node->args[0], env_list);
        if (path == NULL) {
            fprintf(stderr, "command not found: %s\n", node->args[0]);
            exit(127);  // command not found code
        }

        envp = env_list_to_array (*env_list);
        if (!envp) {
            free (path);
            fprintf(stderr, "envp error\n");
            exit(1);
        }

        execve(path, node->args, envp);
        perror("execve");
        free(path);
        envp_free(envp);
        exit(126);
        // printf("child process (pid: %d)\n", getpid());

    }
    else {
        int status;
        printf("parent process (child pid: %d)\n", pid);
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            g_exit_code = WEXITSTATUS(status);
        }
    }
}

static int get_target_fd (t_ast_node *node) {
    if (node->redir_type == TOKEN_REDIR_IN || node->redir_type == TOKEN_HEREDOC) {
        return STDIN_FILENO;
    }
    else if (node->redir_type == TOKEN_REDIR_OUT || node->redir_type == TOKEN_APPEND) {
        return STDOUT_FILENO;
    }
    return -1; // invalid redirection type
}

static int open_redir_file (t_ast_node *node) {
    int fd;
    if (node->redir_type == TOKEN_REDIR_IN) {
        fd = open(node->value, O_RDONLY);
    }
    else if (node->redir_type == TOKEN_REDIR_OUT) {
        fd = open(node->value, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    else if (node->redir_type == TOKEN_APPEND) {
        fd = open(node->value, O_WRONLY | O_CREAT | O_APPEND, 0644);
    }
    else {
        return -1;
    }
    return fd;
}

void execute_redir (t_ast_node *node, t_env **env_list) {
    int fd;
    int target_fd;
    int saved_fd;

    // determine the target file descriptor based on redirection type
    target_fd = get_target_fd (node);
    if (target_fd == -1) {
        fprintf(stderr, "minishell: invalid redirection type\n");
        g_exit_code = 1;
        return;
    }

    // save the current file descriptor (stdout or stdin) to restore later
    saved_fd = dup(target_fd);
    if (saved_fd == -1) {
        perror("dup:");
        g_exit_code = 1;
        return;
    }

    // open the file for redirection based on the redirection type (node->redir_type)
    fd = open_redir_file (node);
    if (fd == -1) {
        fprintf(stderr, "minishell %s: %s\n", node->value, strerror(errno));
        close(saved_fd);
        g_exit_code = 1;
        return;
    }

    // redirect the target file descriptor to the opened file descriptor
    if (dup2(fd, target_fd) == -1) {
        perror("dup2");
        close(fd);
        close(saved_fd);
        return;
    }

    // close the opened file to prevent resource leaks
    close(fd);

    // execute the command or pipeline in the left subtree of the AST
    execute_ast(node->left, env_list);

    // restore the original file descriptor to its previous state
    if (dup2(saved_fd, target_fd) == -1) {
        perror("dup2:");
        g_exit_code = 1;
    }
    // close the saved file descriptor to prevent resource leaks
    close(saved_fd);
}

void execute_ast (t_ast_node *node, t_env **env_list) {
    if (node == NULL) {
        return;
    }

    if (node->type == NODE_PIPE) {
        // execute_pipe(node, env_list);
    }
    else if (node->type == NODE_REDIR) {
        execute_redir(node, env_list);
    }
    else if (node->type == NODE_CMD) {
        execute_cmd(node, env_list);
    }

}