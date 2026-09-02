#ifndef PHILO_PROCS_H
# define PHILO_PROCS_H

# include <pthread.h>
# include <semaphore.h>
# include <fcntl.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>
# include <sys/time.h>

typedef struct s_ctx
{
    int             id;
    int             num_philos;
    long            time_to_die;
    long            time_to_eat;
    long            time_to_sleep;
    int             must_eat_count;
    long            start_time;
    pid_t           parent_pid;
    sem_t           *left_fork;
    sem_t           *right_fork;
    sem_t           *print_sem;
    sem_t           *death_gate;
    pthread_mutex_t meal_lock;
    long            last_meal;
    int             meals_eaten;
}   t_ctx;

int     parse_args(int argc, char **argv, t_ctx *ctx);
long    get_time_ms(void);
void    fork_name(char *buf, pid_t ppid, char const *kind, int idx);
int     run_child(t_ctx *ctx);
void    log_state(t_ctx *ctx, char const *msg);

#endif
