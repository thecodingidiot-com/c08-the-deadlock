#include "philo_procs.h"
#include <errno.h>

static volatile sig_atomic_t g_shutdown = 0;

static void handle_shutdown(int sig)
{
    (void)sig;
    g_shutdown = 1;
}

static sem_t    *open_fork(pid_t ppid, int idx, int create)
{
    char    name[64];
    sem_t   *s;

    fork_name(name, ppid, "fork", idx);
    if (create) {
        sem_unlink(name);
        s = sem_open(name, O_CREAT | O_EXCL, 0644, 1);
    }
    else
        s = sem_open(name, 0);
    if (s == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    return (s);
}

static sem_t    *open_print(pid_t ppid, int create)
{
    char    name[64];
    sem_t   *s;

    fork_name(name, ppid, "print", 0);
    if (create) {
        sem_unlink(name);
        s = sem_open(name, O_CREAT | O_EXCL, 0644, 1);
    }
    else
        s = sem_open(name, 0);
    if (s == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    return (s);
}

static sem_t    *open_death_gate(pid_t ppid, int create)
{
    char    name[64];
    sem_t   *s;

    fork_name(name, ppid, "death", 0);
    if (create) {
        sem_unlink(name);
        s = sem_open(name, O_CREAT | O_EXCL, 0644, 1);
    }
    else
        s = sem_open(name, 0);
    if (s == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    return (s);
}

static pid_t    spawn_child(t_ctx const *base, int id)
{
    t_ctx   ctx;
    pid_t   pid;

    ctx = *base;
    ctx.id = id;
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    if (pid == 0) {
        signal(SIGTERM, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        ctx.left_fork = open_fork(ctx.parent_pid, id - 1, 0);
        ctx.right_fork = open_fork(ctx.parent_pid, id % ctx.num_philos, 0);
        ctx.print_sem = open_print(ctx.parent_pid, 0);
        ctx.death_gate = open_death_gate(ctx.parent_pid, 0);
        exit(run_child(&ctx));
    }
    return (pid);
}

static void reap_and_stop(pid_t *pids, int n, int died_at)
{
    int i;
    int status;

    i = 0;
    while (i < n) {
        if (i != died_at && pids[i] > 0)
            kill(pids[i], SIGTERM);
        i++;
    }
    i = 0;
    while (i < n) {
        if (pids[i] > 0)
            waitpid(pids[i], &status, 0);
        i++;
    }
}

static void cleanup_names(pid_t ppid, int num_philos)
{
    char    name[64];
    int     i;

    i = 0;
    while (i < num_philos) {
        fork_name(name, ppid, "fork", i);
        sem_unlink(name);
        i++;
    }
    fork_name(name, ppid, "print", 0);
    sem_unlink(name);
    fork_name(name, ppid, "death", 0);
    sem_unlink(name);
}

int main(int argc, char **argv)
{
    t_ctx           base;
    pid_t           *pids;
    int             i;
    int             done;
    int             status;
    pid_t           w;
    struct sigaction sa;

    if (!parse_args(argc, argv, &base))
        return (1);
    signal(SIGPIPE, SIG_IGN);
    sa.sa_handler = handle_shutdown;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    base.parent_pid = getpid();
    base.start_time = get_time_ms();
    i = 0;
    while (i < base.num_philos)
        open_fork(base.parent_pid, i++, 1);
    open_print(base.parent_pid, 1);
    open_death_gate(base.parent_pid, 1);
    pids = malloc(sizeof(pid_t) * base.num_philos);
    i = 0;
    while (i < base.num_philos) {
        pids[i] = spawn_child(&base, i + 1);
        i++;
    }
    done = 0;
    while (done < base.num_philos) {
        w = waitpid(-1, &status, 0);
        if (w < 0 && errno == EINTR) {
            if (g_shutdown) {
                reap_and_stop(pids, base.num_philos, -1);
                break ;
            }
            continue ;
        }
        i = 0;
        while (i < base.num_philos && pids[i] != w)
            i++;
        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
            pids[i] = -1;
            reap_and_stop(pids, base.num_philos, i);
            break ;
        }
        pids[i] = -1;
        done++;
    }
    cleanup_names(base.parent_pid, base.num_philos);
    free(pids);
    return (0);
}
