#include "../inc/minishell.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <readline/readline.h>

volatile int heredoc_interrupted = 0;

void handle_prompt_sigint(int signal_number) {

    g_exit_code = 130;

    // write a newline to move the cursor to the next line
    write(STDOUT_FILENO, "\n", 1);

    // clear the current input line
    rl_replace_line("",0);
    // inform readline cursor is at the new line
    rl_on_new_line();
    // redisplay the prompt and the current input line
    rl_redisplay();

}

void setup_prompt_signals(void) {
    struct sigaction sigint_action = {0};
    struct sigaction sigquit_action = {0};

    // empty the list of blocked signals while sigint action is being executed
    sigemptyset(&sigint_action.sa_mask);

    // set the handler for SIGINT to handle_prompt_sigint
    sigint_action.sa_handler = handle_prompt_sigint;
    sigint_action.sa_flags = SA_RESTART;

    // replace the default SIGINT action with the new one
    if (sigaction(SIGINT, &sigint_action, NULL) == -1) {
        perror("sigaction");
    }

    // ignore SIGQUIT as CTRL+\ should not quit the shell
    sigemptyset(&sigquit_action.sa_mask);
    sigquit_action.sa_handler = SIG_IGN; // ignore SIGQUIT
    sigquit_action.sa_flags = 0;

    if (sigaction(SIGQUIT, &sigquit_action, NULL) == -1) {
        perror("sigaction");
    }
}

void setup_parent_signals(void) {
    struct sigaction ignore_action = {0};

    sigemptyset(&ignore_action.sa_mask);
    ignore_action.sa_handler = SIG_IGN; // set the handler to ignore any action
    ignore_action.sa_flags = 0;

    // set both SIGINT and SIGQUIT with ignore action
    if (sigaction(SIGINT, &ignore_action, NULL) == -1) {
        perror("sigaction");
    }

    if (sigaction(SIGQUIT, &ignore_action, NULL) == -1) {
        perror("sigaction");
    }
}

void setup_child_signals(void) {
    struct sigaction default_action = {0};

    sigemptyset(&default_action.sa_mask);
    default_action.sa_handler = SIG_DFL; // set the handler to default action
    default_action.sa_flags = 0;

    if (sigaction(SIGINT, &default_action, NULL) == -1) {
        perror("sigaction");
    }

    if (sigaction(SIGQUIT, &default_action, NULL) == -1) {
        perror("sigaction");
    }
}

void handle_heredoc_sigint(int signal_number) {
    heredoc_interrupted = 1;

    write(STDOUT_FILENO, "\n", 1);
}

void setup_heredoc_signals(void) {
    struct sigaction sigint_action = {0};
    struct sigaction sigquit_action = {0};

    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_handler = handle_heredoc_sigint;
    sigint_action.sa_flags = 0;

    if (sigaction(SIGINT, &sigint_action, NULL) == -1) {
        perror("sigaction");
    }

    sigemptyset(&sigquit_action.sa_mask);
    sigquit_action.sa_handler = SIG_IGN;
    sigquit_action.sa_flags = 0;

    if (sigaction(SIGQUIT, &sigquit_action, NULL) == -1) {
        perror("sigaction");
    }
}