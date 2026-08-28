#include "../inc/minishell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

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
            
            // printf("command %s matched with %s\n", args[0], g_builtins[i].cmd_name);

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
        // printf("parent process (child pid: %d)\n", pid);
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            g_exit_code = WEXITSTATUS(status);
        }
    }
}

// create temp file path for heredoc
static char* create_heredoc_path() {
    static int index = 0;
    char *path;
    int length;
    int written;

    length = snprintf(NULL, 0, "/tmp/.minishell_heredoc_%ld_%u", (long)getpid(), index);

    if (length < 0) {
        return (NULL);
    }

    path = malloc((size_t)length + 1);
    if (path == NULL) {
        return (NULL);
    }

    written = snprintf(path, (size_t)length + 1, "/tmp/.minishell_heredoc_%ld_%u", (long)getpid(), index);

    if (written != length){
        free(path);
        return (NULL);
    }

    index++;
    return path;
}

// open temp heredoc file
static int create_and_open_heredoc_temp (char** res_path) {
    char* path;
    int fd;
    int saved_errno;

    while(1) {
        path = create_heredoc_path();
        if (path == NULL) {
            return -1;
        }

        fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
        if (fd != -1) {
            *res_path = path;
            return fd;
        }

        saved_errno = errno;
        free(path);

        if (saved_errno != EEXIST) {
            errno = saved_errno;
            return -1;
        }
    }
}

// receive inputs, write it in temp file, return read fd
static int collect_heredoc (const char* dil) {
    char* path;
    char* line;
    int write_fd;
    int read_fd;

    path = NULL;
    write_fd = create_and_open_heredoc_temp(&path);
    if (write_fd == -1) {
        perror("heredoc");
        return -1;
    }

    while (1) {
        line = readline("> ");

        if (line == NULL) {
            fprintf(stderr, "minishell: warning: here-document delimited by end-of-file (wanted `%s')\n", dil);
            break;
        }

        if (strcmp(line, dil) == 0) {
            free(line);
            break;
        }

        if (write(write_fd, line, strlen(line)) == -1 || write(write_fd, "\n", 1) == -1) {
            perror("heredoc");
            free(line);
            close(write_fd);
            unlink(path);
            free(path);
            return -1;
        }
        free(line);
    }
    close(write_fd);

    read_fd = open(path, O_RDONLY);
    if (read_fd == -1) {
        perror("heredoc");
        unlink(path);
        free(path);
        return -1;
    }
    
    unlink(path);
    free(path);
    
    return read_fd;
}

static int prepare_heredocs (t_ast_node *node) {
    if (node == NULL) {
        return 0;
    }

    if (node->type == NODE_PIPE) {
        if (prepare_heredocs(node->left) == -1) {
            return -1;
        }
        if (prepare_heredocs(node->right) == -1) {
            return -1;
        }
        return 0;
    }

    if (node->type == NODE_REDIR) {
        if (prepare_heredocs(node->left) == -1) {
            return -1;
        }
        if (node->redir_type == TOKEN_HEREDOC) {
            node->heredoc_fd = collect_heredoc(node->value);
            if (node->heredoc_fd == -1) {
                return -1;
            }
        }
    }
    return 0;
}

static void close_prepared_heredocs (t_ast_node *node) {
    if (node == NULL) {
        return;
    }

    close_prepared_heredocs(node->left);
    close_prepared_heredocs(node->right);

    if (node->heredoc_fd != -1) {
        close(node->heredoc_fd);
        node->heredoc_fd = -1;
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

// open redir file or call heredoc operation and return fd
static int get_redir_fd (t_ast_node *node) {
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
    else if (node->redir_type == TOKEN_HEREDOC) {
        if (node->heredoc_fd != -1) {
            fd = node->heredoc_fd;
        }
        else {
            errno = 0;
            return -1;
        }
        node->heredoc_fd = -1;
    }
    else {
        return -1;
    }
    return fd;
}

// skips over redir nodes to find the next cmd or pipe node to execute
static t_ast_node *get_exec_node (t_ast_node *node) {
    while (node != NULL && node->type == NODE_REDIR) {
        node = node->left;
    }
    return node;
}

// applies a single redir node
static int apply_redir (t_ast_node *node) {
    int target_fd;
    int redir_fd;

    // determine the target file descriptor based on redirection type
    target_fd = get_target_fd(node);
    if (target_fd == -1) {
        fprintf(stderr, "minishell: invalid redirection type\n");
        g_exit_code = 1;
        return -1;
    }
    
    // open the file for redirection based on the redirection type (node->redir_type)
    redir_fd = get_redir_fd(node);
    if (redir_fd == -1) {
        fprintf(stderr, "minishell: %s: %s\n", node->value, strerror(errno));
        g_exit_code = 1;
        return -1;
    }

    // redirect the target file descriptor to the opened file descriptor
    if (dup2(redir_fd, target_fd) == -1) {
        perror("dup2");
        close(redir_fd);
        g_exit_code = 1;
        return -1;
    }

    // close the opened file to prevent resource leaks
    close(redir_fd);
    return 0;
}

// recursively applies redir nodes
static int chain_redirs (t_ast_node *node) {
    if (node == NULL || node->type != NODE_REDIR) {
        return 0;
    }

    if (chain_redirs(node->left) == -1) {
        return -1;
    }

    return apply_redir(node);
}

void execute_redir (t_ast_node *node, t_env **env_list) {
    t_ast_node *exec_node;
    int saved_stdin_fd;
    int saved_stdout_fd;

    // save stdin fd to restore later
    saved_stdin_fd = dup(STDIN_FILENO);
    if (saved_stdin_fd == -1) {
        perror("dup:");
        g_exit_code = 1;
        return;
    }

    // save stdout fd to restore later
    saved_stdout_fd = dup(STDOUT_FILENO);
    if (saved_stdout_fd == -1) {
        perror("dup:");
        close(saved_stdin_fd);
        g_exit_code = 1;
        return;
    }

    // find executable node 
    exec_node = get_exec_node(node);

    // apply redirections recursively and execute exec_node if it exists
    if (exec_node != NULL && chain_redirs(node) == 0) {
        execute_ast(exec_node, env_list);
    }

    // restore og stdin and stdout fds
    if (dup2(saved_stdin_fd, STDIN_FILENO) == -1) {
        perror("dup2");
        g_exit_code = 1;
    }
    if (dup2(saved_stdout_fd, STDOUT_FILENO) == -1) {
        perror("dup2");
        g_exit_code = 1;
    }

    // close saved fds
    close(saved_stdin_fd);
    close(saved_stdout_fd);
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

void execute_command (t_ast_node *root, t_env **env_list) {
    if (root == NULL) {
        return;
    }

    if (prepare_heredocs(root) == -1) {
        close_prepared_heredocs(root);

        if (g_exit_code == 0) {
            g_exit_code = 1;
        }
        return;
    }
    
    execute_ast(root, env_list);
    close_prepared_heredocs(root);
}