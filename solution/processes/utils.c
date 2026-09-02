#include "philo_procs.h"

long    get_time_ms(void)
{
    struct timeval  tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L + tv.tv_usec / 1000L);
}

void    fork_name(char *buf, pid_t ppid, char const *kind, int idx)
{
    sprintf(buf, "/tci_philo_%s_%d_%d", kind, (int)ppid, idx);
}

void    log_state(t_ctx *ctx, char const *msg)
{
    sem_wait(ctx->print_sem);
    printf("%ld %d %s\n", get_time_ms() - ctx->start_time, ctx->id, msg);
    fflush(stdout);
    sem_post(ctx->print_sem);
}
