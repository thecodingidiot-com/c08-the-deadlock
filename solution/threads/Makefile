NAME        = philo
CC          = gcc
CFLAGS      = -Wall -Wextra -g -std=c99 -D_GNU_SOURCE -pthread

SRCS        = main.c init.c routine.c monitor.c utils.c
OBJS        = $(SRCS:.c=.o)

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -pthread -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
