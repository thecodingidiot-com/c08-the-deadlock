#include "philo_procs.h"

static long ft_atol(char const *s)
{
    long    n;
    int     i;

    n = 0;
    i = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        n = n * 10 + (s[i] - '0');
        i++;
    }
    return (n);
}

static int all_digits(char const *s)
{
    int i;

    if (!s[0])
        return (0);
    i = 0;
    while (s[i]) {
        if (s[i] < '0' || s[i] > '9')
            return (0);
        i++;
    }
    return (1);
}

int     parse_args(int argc, char **argv, t_ctx *ctx)
{
    if (argc != 5 && argc != 6) {
        fprintf(stderr,
            "usage: %s num_philos time_to_die time_to_eat "
            "time_to_sleep [must_eat_count]\n", argv[0]);
        return (0);
    }
    if (!all_digits(argv[1]) || !all_digits(argv[2])
        || !all_digits(argv[3]) || !all_digits(argv[4])
        || (argc == 6 && !all_digits(argv[5])))
    {
        fprintf(stderr, "error: all arguments must be positive integers\n");
        return (0);
    }
    ctx->num_philos = (int)ft_atol(argv[1]);
    ctx->time_to_die = ft_atol(argv[2]);
    ctx->time_to_eat = ft_atol(argv[3]);
    ctx->time_to_sleep = ft_atol(argv[4]);
    if (argc == 6)
        ctx->must_eat_count = (int)ft_atol(argv[5]);
    else
        ctx->must_eat_count = -1;
    if (ctx->num_philos < 1) {
        fprintf(stderr, "error: num_philos must be at least 1\n");
        return (0);
    }
    return (1);
}
