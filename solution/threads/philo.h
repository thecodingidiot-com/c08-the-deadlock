#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>

typedef struct s_data   t_data;

typedef struct s_philo
{
    int             id;
    int             meals_eaten;
    long            last_meal;
    pthread_t       thread;
    pthread_mutex_t meal_lock;
    t_data          *data;
}   t_philo;

struct s_data
{
    int             num_philos;
    long            time_to_die;
    long            time_to_eat;
    long            time_to_sleep;
    int             must_eat_count;
    long            start_time;
    int             stop;
    pthread_mutex_t stop_lock;
    pthread_mutex_t print_lock;
    pthread_mutex_t *forks;
    t_philo         *philos;
};

int     parse_args(int argc, char **argv, t_data *data);
int     init_data(t_data *data);
void    free_data(t_data *data);
void    *philo_routine(void *arg);
void    run_monitor(t_data *data);
long    get_time_ms(void);
void    print_state(t_philo *philo, char const *msg);
int     simulation_stopped(t_data *data);
void    stop_simulation(t_data *data);

#endif
