#include "philo.h"

static int check_death(t_data *data)
{
    int     i;
    long    since;

    i = 0;
    while (i < data->num_philos) {
        pthread_mutex_lock(&data->philos[i].meal_lock);
        since = get_time_ms() - data->philos[i].last_meal;
        pthread_mutex_unlock(&data->philos[i].meal_lock);
        if (since > data->time_to_die) {
            pthread_mutex_lock(&data->print_lock);
            printf("%ld %d died\n", get_time_ms() - data->start_time,
                data->philos[i].id);
            fflush(stdout);
            pthread_mutex_unlock(&data->print_lock);
            stop_simulation(data);
            return (1);
        }
        i++;
    }
    return (0);
}

static int all_fed(t_data *data)
{
    int i;

    if (data->must_eat_count < 0)
        return (0);
    i = 0;
    while (i < data->num_philos) {
        pthread_mutex_lock(&data->philos[i].meal_lock);
        if (data->philos[i].meals_eaten < data->must_eat_count) {
            pthread_mutex_unlock(&data->philos[i].meal_lock);
            return (0);
        }
        pthread_mutex_unlock(&data->philos[i].meal_lock);
        i++;
    }
    return (1);
}

void    run_monitor(t_data *data)
{
    while (!simulation_stopped(data)) {
        if (check_death(data))
            return ;
        if (all_fed(data)) {
            stop_simulation(data);
            return ;
        }
        usleep(1000);
    }
}
