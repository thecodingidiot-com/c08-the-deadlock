#include "philo.h"

static void take_forks(t_philo *philo)
{
    int left;
    int right;

    left = philo->id - 1;
    right = philo->id % philo->data->num_philos;
    if (left == right)
    {
        pthread_mutex_lock(&philo->data->forks[left]);
        print_state(philo, "has taken a fork");
        return ;
    }
    if (left < right)
    {
        pthread_mutex_lock(&philo->data->forks[left]);
        print_state(philo, "has taken a fork");
        pthread_mutex_lock(&philo->data->forks[right]);
        print_state(philo, "has taken a fork");
    }
    else
    {
        pthread_mutex_lock(&philo->data->forks[right]);
        print_state(philo, "has taken a fork");
        pthread_mutex_lock(&philo->data->forks[left]);
        print_state(philo, "has taken a fork");
    }
}

static void put_forks(t_philo *philo)
{
    int left;
    int right;

    left = philo->id - 1;
    right = philo->id % philo->data->num_philos;
    pthread_mutex_unlock(&philo->data->forks[left]);
    if (left != right)
        pthread_mutex_unlock(&philo->data->forks[right]);
}

static void eat(t_philo *philo)
{
    take_forks(philo);
    if (philo->data->num_philos == 1)
    {
        /* Only one fork exists. It is held, never released, and a
         * second fork never arrives — the philosopher starves holding
         * it, exactly like the textbook edge case says. */
        while (!simulation_stopped(philo->data))
            usleep(1000);
        pthread_mutex_unlock(&philo->data->forks[0]);
        return ;
    }
    pthread_mutex_lock(&philo->meal_lock);
    philo->last_meal = get_time_ms();
    philo->meals_eaten++;
    pthread_mutex_unlock(&philo->meal_lock);
    print_state(philo, "is eating");
    usleep(philo->data->time_to_eat * 1000);
    put_forks(philo);
}

void    *philo_routine(void *arg)
{
    t_philo *philo;

    philo = (t_philo *)arg;
    if (philo->id % 2 == 0)
        usleep(1000);
    while (!simulation_stopped(philo->data))
    {
        eat(philo);
        if (simulation_stopped(philo->data))
            break ;
        print_state(philo, "is sleeping");
        usleep(philo->data->time_to_sleep * 1000);
        print_state(philo, "is thinking");
    }
    return (NULL);
}
