NAME = minishell

CC = gcc
CFLAGS = -Wall
INCLUDES = -I inc
LIBS = -lreadline

SRCS = $(shell find src -type f -name "*.c")

all: $(NAME)

$(NAME): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(INCLUDES) $(LIBS) -o $(NAME)

clean:
	rm -f $(NAME)

re: clean all