NAME        = philo
CC          = gcc
CFLAGS      = -Wall -Wextra -Werror -std=c99 -D_GNU_SOURCE -pthread
BUILD       = -g -O0
RELEASE     = -O2

SRCS        = main.c init.c routine.c monitor.c utils.c
OBJS        = $(SRCS:.c=.o)

.PHONY: all release clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(BUILD) $(OBJS) -pthread -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(BUILD) -c $< -o $@

release:
	@$(MAKE) --no-print-directory fclean
	@$(MAKE) --no-print-directory BUILD="$(RELEASE)"
	strip $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
