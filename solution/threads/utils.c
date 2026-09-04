#include "philo.h"

long    get_time_ms(void)
{
    struct timeval  tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L + tv.tv_usec / 1000L);
}

int     simulation_stopped(t_data *data)
{
    int stop;

    pthread_mutex_lock(&data->stop_lock);
    stop = data->stop;
    pthread_mutex_unlock(&data->stop_lock);
    return (stop);
}

void    stop_simulation(t_data *data)
{
    pthread_mutex_lock(&data->stop_lock);
    data->stop = 1;
    pthread_mutex_unlock(&data->stop_lock);
}

void    print_state(t_philo *philo, char const *msg)
{
    pthread_mutex_lock(&philo->data->print_lock);
    if (!simulation_stopped(philo->data)) {
        printf("%ld %d %s\n", get_time_ms() - philo->data->start_time,
            philo->id, msg);
        fflush(stdout);
    }
    pthread_mutex_unlock(&philo->data->print_lock);
}
