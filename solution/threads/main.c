#include "philo.h"

int main(int argc, char **argv)
{
    t_data  data;
    int     i;

    if (!parse_args(argc, argv, &data))
        return (1);
    signal(SIGPIPE, SIG_IGN);
    if (!init_data(&data))
    {
        fprintf(stderr, "error: allocation failed\n");
        return (1);
    }
    i = 0;
    while (i < data.num_philos)
    {
        pthread_create(&data.philos[i].thread, NULL, philo_routine,
            &data.philos[i]);
        i++;
    }
    run_monitor(&data);
    i = 0;
    while (i < data.num_philos)
    {
        pthread_join(data.philos[i].thread, NULL);
        i++;
    }
    free_data(&data);
    return (0);
}
