#include "philo_procs.h"

static void *watcher(void *arg)
{
    t_ctx   *ctx;
    long    since;

    ctx = (t_ctx *)arg;
    while (1) {
        pthread_mutex_lock(&ctx->meal_lock);
        since = get_time_ms() - ctx->last_meal;
        pthread_mutex_unlock(&ctx->meal_lock);
        if (since > ctx->time_to_die) {
            if (sem_trywait(ctx->death_gate) == 0) {
                sem_wait(ctx->print_sem);
                printf("%ld %d died\n", get_time_ms() - ctx->start_time,
                    ctx->id);
                fflush(stdout);
                sem_post(ctx->print_sem);
            }
            _exit(1);
        }
        usleep(1000);
    }
    return (NULL);
}

static void take_forks(t_ctx *ctx, int left, int right)
{
    if (left < right) {
        sem_wait(ctx->left_fork);
        log_state(ctx, "has taken a fork");
        sem_wait(ctx->right_fork);
        log_state(ctx, "has taken a fork");
    }
    else
    {
        sem_wait(ctx->right_fork);
        log_state(ctx, "has taken a fork");
        sem_wait(ctx->left_fork);
        log_state(ctx, "has taken a fork");
    }
}

static void eat(t_ctx *ctx)
{
    int left;
    int right;

    left = ctx->id - 1;
    right = ctx->id % ctx->num_philos;
    if (left == right) {
        sem_wait(ctx->left_fork);
        log_state(ctx, "has taken a fork");
        while (1)
            usleep(1000);
    }
    take_forks(ctx, left, right);
    pthread_mutex_lock(&ctx->meal_lock);
    ctx->last_meal = get_time_ms();
    ctx->meals_eaten++;
    pthread_mutex_unlock(&ctx->meal_lock);
    log_state(ctx, "is eating");
    usleep(ctx->time_to_eat * 1000);
    sem_post(ctx->left_fork);
    if (left != right)
        sem_post(ctx->right_fork);
}

int     run_child(t_ctx *ctx)
{
    pthread_t   watch;

    ctx->last_meal = ctx->start_time;
    ctx->meals_eaten = 0;
    pthread_mutex_init(&ctx->meal_lock, NULL);
    pthread_create(&watch, NULL, watcher, ctx);
    pthread_detach(watch);
    if (ctx->id % 2 == 0)
        usleep(1000);
    while (1) {
        eat(ctx);
        log_state(ctx, "is sleeping");
        usleep(ctx->time_to_sleep * 1000);
        log_state(ctx, "is thinking");
        if (ctx->must_eat_count >= 0 && ctx->meals_eaten >= ctx->must_eat_count)
            return (0);
    }
}
