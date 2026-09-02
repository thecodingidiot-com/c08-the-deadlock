# c08-the-deadlock

Companion repository for **c08 — The Deadlock** at
[thecodingidiot.com](https://thecodingidiot.com).

---

## Follow my journey

Working through c08 alongside the implementation pages? Build `philo` and
`philo_procs` step by step in your own `threads/` and `processes/`
directories, then run the tester.

Clone this repository and copy `test.sh` into your working directory:

```bash
git clone https://github.com/thecodingidiot-com/c08-the-deadlock.git
cp c08-the-deadlock/test.sh ~/c08-practice/
cd ~/c08-practice
bash test.sh
```

`test.sh` expects `./threads/` and `./processes/` by default — set
`THREADS_DIR`/`PROCESSES_DIR` if yours are named differently. All tests must
pass before the chapter is complete.

---

## Follow your journey

Building the dining philosophers independently? Here is the full project
brief.

The dining philosophers problem, built two ways — same rules both times: `N`
philosophers, `N` forks, two forks needed to eat, no philosopher may starve,
no deadlock.

- **`philo`** — one thread per philosopher, one `pthread_mutex_t` per fork.
- **`philo_procs`** — one process per philosopher (`fork()`), one named
  POSIX semaphore per fork (`sem_open`/`sem_wait`/`sem_post`).

Both take the same five arguments:

```
philo <num_philos> <time_to_die> <time_to_eat> <time_to_sleep> [must_eat_count]
```

All in milliseconds except `num_philos` and the optional `must_eat_count`. A
philosopher that goes longer than `time_to_die` without starting to eat dies,
prints `<timestamp> <id> died`, and the whole simulation stops. If
`must_eat_count` is given, the simulation stops cleanly once every
philosopher has eaten that many times.

| File (threads/) | Contents |
| --- | --- |
| `main.c` | parses args, spawns philosopher threads, runs the monitor |
| `init.c` | allocates and initialises the shared state and forks |
| `routine.c` | one philosopher's think → take forks → eat → sleep cycle |
| `monitor.c` | watches every philosopher's last meal time and the must-eat count |
| `utils.c` | timestamps and the print-lock-guarded logger |

| File (processes/) | Contents |
| --- | --- |
| `main.c` | parent: creates named semaphores, forks children, reaps them |
| `args.c` | argument parsing (identical logic to the threaded version) |
| `child.c` | one philosopher process's cycle, plus its own starvation watcher thread |
| `utils.c` | timestamps, semaphore naming, the print-semaphore-guarded logger |

Both use a resource-hierarchy fork order (always take the lower-numbered
fork first) to make deadlock structurally impossible, not just unlikely.

Build and test your own version first. Use `solution/` to compare once you
are done, not before.

---

## Building the solution

```bash
cd c08-the-deadlock/solution/threads && make
cd ../processes && make
```

Neither binary links against `libtci`/`libtciutil` — this chapter is pure
POSIX (`pthread.h`, `semaphore.h`), no prior chapter's library required.

---

## What the tester checks

- **Build** — both binaries compile with zero warnings.
- **Deterministic death** — a single philosopher (only one fork ever exists)
  dies of starvation at the expected time, in both implementations.
- **Must-eat-count** — with a count given, the simulation exits cleanly
  (exit code 0, no death message) once every philosopher has eaten at least
  that many times.
- **No starvation** — over a longer run at generous timing, every
  philosopher eats at least once and nothing dies.
- **Clean external stop** — sending the process-based solution's parent
  `SIGTERM` (the same as Ctrl+C on a real terminal) leaves no orphaned
  philosopher processes running and no named semaphores behind in
  `/dev/shm`.
- **Race-free** — the threaded solution runs clean under `valgrind --tool=helgrind`.

---

## License

MIT License. See [LICENSE](LICENSE).
