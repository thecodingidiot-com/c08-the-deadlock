#include "philo.h"

static long ft_atol(char const *s)
{
    long    n;
    int     i;

    n = 0;
    i = 0;
    while (s[i] >= '0' && s[i] <= '9')
    {
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
    while (s[i])
    {
        if (s[i] < '0' || s[i] > '9')
            return (0);
        i++;
    }
    return (1);
}

int     parse_args(int argc, char **argv, t_data *data)
{
    if (argc != 5 && argc != 6)
    {
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
    data->num_philos = (int)ft_atol(argv[1]);
    data->time_to_die = ft_atol(argv[2]);
    data->time_to_eat = ft_atol(argv[3]);
    data->time_to_sleep = ft_atol(argv[4]);
    if (argc == 6)
        data->must_eat_count = (int)ft_atol(argv[5]);
    else
        data->must_eat_count = -1;
    if (data->num_philos < 1)
    {
        fprintf(stderr, "error: num_philos must be at least 1\n");
        return (0);
    }
    return (1);
}

int     init_data(t_data *data)
{
    int i;

    data->stop = 0;
    pthread_mutex_init(&data->stop_lock, NULL);
    pthread_mutex_init(&data->print_lock, NULL);
    data->forks = malloc(sizeof(pthread_mutex_t) * data->num_philos);
    data->philos = malloc(sizeof(t_philo) * data->num_philos);
    if (!data->forks || !data->philos)
        return (0);
    i = 0;
    while (i < data->num_philos)
    {
        pthread_mutex_init(&data->forks[i], NULL);
        i++;
    }
    data->start_time = get_time_ms();
    i = 0;
    while (i < data->num_philos)
    {
        data->philos[i].id = i + 1;
        data->philos[i].meals_eaten = 0;
        data->philos[i].last_meal = data->start_time;
        data->philos[i].data = data;
        pthread_mutex_init(&data->philos[i].meal_lock, NULL);
        i++;
    }
    return (1);
}

void    free_data(t_data *data)
{
    int i;

    i = 0;
    while (i < data->num_philos)
    {
        pthread_mutex_destroy(&data->forks[i]);
        pthread_mutex_destroy(&data->philos[i].meal_lock);
        i++;
    }
    pthread_mutex_destroy(&data->stop_lock);
    pthread_mutex_destroy(&data->print_lock);
    free(data->forks);
    free(data->philos);
}
