#!/bin/bash
# c08 — The Deadlock / test.sh
#
# Builds both solutions (threads+mutexes, processes+semaphores) and checks:
#   - both build with zero warnings
#   - a single philosopher dies of starvation, deterministically, in both
#   - a must-eat-count run finishes cleanly with every philosopher fed exactly
#     that many times, in both
#   - a longer run with generous timing shows no death and every philosopher
#     ate at least once (no starvation) in both
#   - the threaded solution is race-free under helgrind
#   - the process solution leaves no orphaned children or leaked named
#     semaphores behind after SIGTERM (Ctrl+C-style external stop)
#   - under timing tight enough that multiple philosophers starve almost
#     simultaneously, the process solution still reports exactly one death
#     line, never more (a real race found and fixed: independent watcher
#     threads across separate processes can otherwise all "win" at once)
#
# Copy this file into your working directory (next to your own threads/ and
# processes/ solutions), then run:
#
#   bash test.sh
#
# Pass THREADS_DIR/PROCESSES_DIR as env vars to point at your own working
# directories instead of ./threads and ./processes:
#
#   THREADS_DIR=~/c08-practice/threads PROCESSES_DIR=~/c08-practice/processes bash test.sh

set -o pipefail

THREADS_DIR="${THREADS_DIR:-./threads}"
PROCESSES_DIR="${PROCESSES_DIR:-./processes}"

# ── colour ────────────────────────────────────────────────────────────────────

if [[ ! -t 1 ]]; then
    C_GREEN=""
    C_RED=""
    C_BOLD=""
    C_RESET=""
else
    C_GREEN="\033[0;32m"
    C_RED="\033[0;31m"
    C_BOLD="\033[1m"
    C_RESET="\033[0m"
fi

pass_count=0
fail_count=0

hr() {
    echo "────────────────────────────────────────────────────────────────"
}

banner() {
    hr
    echo "  c08 — The Deadlock / test.sh"
    hr
}

pass() {
    printf "  ${C_GREEN}PASS${C_RESET}  %s\n" "$1"
    pass_count=$((pass_count + 1))
}

fail() {
    printf "  ${C_RED}FAIL${C_RESET}  %s\n" "$1"
    if [[ -n "${2:-}" ]]; then
        echo "        $2"
    fi
    fail_count=$((fail_count + 1))
}

count_eats() {
    local out="$1"
    local id="$2"
    echo "$out" | grep -cE "^[0-9]+ $id is eating"
}

banner

# ── build ─────────────────────────────────────────────────────────────────────

echo "Building threads/philo..."
t_log=$(cd "$THREADS_DIR" && make re 2>&1)
t_status=$?
if [[ "$t_status" -ne 0 ]]; then
    fail "threads/philo builds" "make re failed:"
    echo "$t_log"
    exit 1
fi
pass "threads/philo builds"
if echo "$t_log" | grep -qi "warning"; then
    fail "threads/philo builds with no warnings" "$(echo "$t_log" | grep -i warning)"
else
    pass "threads/philo builds with no warnings"
fi

echo "Building processes/philo_procs..."
p_log=$(cd "$PROCESSES_DIR" && make re 2>&1)
p_status=$?
if [[ "$p_status" -ne 0 ]]; then
    fail "processes/philo_procs builds" "make re failed:"
    echo "$p_log"
    exit 1
fi
pass "processes/philo_procs builds"
if echo "$p_log" | grep -qi "warning"; then
    fail "processes/philo_procs builds with no warnings" "$(echo "$p_log" | grep -i warning)"
else
    pass "processes/philo_procs builds with no warnings"
fi

THREADS_BIN="$THREADS_DIR/philo"
PROCS_BIN="$PROCESSES_DIR/philo_procs"

# ── single philosopher: deterministic death ────────────────────────────────────

echo
echo "Running single-philosopher death case (both)..."
t_die=$("$THREADS_BIN" 1 800 200 200)
if echo "$t_die" | grep -qE "^[0-9]+ 1 died$"; then
    pass "threads: single philosopher dies of starvation"
else
    fail "threads: single philosopher dies of starvation" "got: $t_die"
fi

p_die=$("$PROCS_BIN" 1 800 200 200)
if echo "$p_die" | grep -qE "^[0-9]+ 1 died$"; then
    pass "processes: single philosopher dies of starvation"
else
    fail "processes: single philosopher dies of starvation" "got: $p_die"
fi

# ── simultaneous starvation: exactly one death line, never more ────────────────

echo
echo "Running simultaneous-starvation check (processes, x5 runs)..."
p_single_death_ok=1
for _ in 1 2 3 4 5; do
    p_race_out=$("$PROCS_BIN" 3 200 300 100)
    p_death_lines=$(echo "$p_race_out" | grep -cE "^[0-9]+ [0-9]+ died$")
    if [[ "$p_death_lines" -ne 1 ]]; then
        p_single_death_ok=0
        fail "processes: exactly one death line under simultaneous starvation" \
            "got $p_death_lines death lines: $(echo "$p_race_out" | grep died)"
        break
    fi
done
[[ "$p_single_death_ok" -eq 1 ]] && \
    pass "processes: exactly one death line under simultaneous starvation (5 runs)"

# ── must-eat-count: clean finish, exact counts ─────────────────────────────────

echo
echo "Running must-eat-count=3 case (5 philosophers, both)..."
t_out=$("$THREADS_BIN" 5 1000 200 200 3)
t_exit=$?
t_ok=1
[[ "$t_exit" -eq 0 ]] || t_ok=0
echo "$t_out" | grep -q died && t_ok=0
for i in 1 2 3 4 5; do
    [[ "$(count_eats "$t_out" "$i")" -ge 3 ]] || t_ok=0
done
if [[ "$t_ok" -eq 1 ]]; then
    pass "threads: must-eat-count finishes clean, every philosopher fed 3+ times"
else
    fail "threads: must-eat-count finishes clean, every philosopher fed 3+ times"
fi

p_out=$("$PROCS_BIN" 5 1000 200 200 3)
p_exit=$?
p_ok=1
[[ "$p_exit" -eq 0 ]] || p_ok=0
echo "$p_out" | grep -q died && p_ok=0
for i in 1 2 3 4 5; do
    [[ "$(count_eats "$p_out" "$i")" -ge 3 ]] || p_ok=0
done
if [[ "$p_ok" -eq 1 ]]; then
    pass "processes: must-eat-count finishes clean, every philosopher fed 3+ times"
else
    fail "processes: must-eat-count finishes clean, every philosopher fed 3+ times"
fi

# ── longer run: no starvation, clean external stop ─────────────────────────────

echo
echo "Running 3s no-starvation check + external SIGTERM cleanup (both)..."

"$THREADS_BIN" 5 1000 200 200 > /tmp/c08_t_long.txt 2>&1 &
t_pid=$!
sleep 3
kill -TERM "$t_pid" 2>/dev/null
wait "$t_pid" 2>/dev/null
t_ok=1
grep -q died /tmp/c08_t_long.txt && t_ok=0
for i in 1 2 3 4 5; do
    [[ "$(count_eats "$(cat /tmp/c08_t_long.txt)" "$i")" -ge 1 ]] || t_ok=0
done
if [[ "$t_ok" -eq 1 ]]; then
    pass "threads: no starvation over 3s at generous timing"
else
    fail "threads: no starvation over 3s at generous timing"
fi
rm -f /tmp/c08_t_long.txt

rm -f /dev/shm/sem.tci_philo_* 2>/dev/null
"$PROCS_BIN" 5 1000 200 200 > /tmp/c08_p_long.txt 2>&1 &
p_pid=$!
sleep 3
kill -TERM "$p_pid" 2>/dev/null
wait "$p_pid" 2>/dev/null
sleep 0.3
p_ok=1
grep -q died /tmp/c08_p_long.txt && p_ok=0
for i in 1 2 3 4 5; do
    [[ "$(count_eats "$(cat /tmp/c08_p_long.txt)" "$i")" -ge 1 ]] || p_ok=0
done
if [[ "$p_ok" -eq 1 ]]; then
    pass "processes: no starvation over 3s at generous timing"
else
    fail "processes: no starvation over 3s at generous timing"
fi
rm -f /tmp/c08_p_long.txt

if ls /dev/shm/sem.tci_philo_* >/dev/null 2>&1; then
    fail "processes: no leaked named semaphores after external SIGTERM" \
        "$(ls /dev/shm/sem.tci_philo_* 2>/dev/null)"
    rm -f /dev/shm/sem.tci_philo_* 2>/dev/null
else
    pass "processes: no leaked named semaphores after external SIGTERM"
fi

if pgrep -f "$(realpath "$PROCS_BIN")" >/dev/null 2>&1; then
    fail "processes: no orphaned children survive external SIGTERM to the parent"
else
    pass "processes: no orphaned children survive external SIGTERM to the parent"
fi

# ── helgrind: threaded solution is race-free ────────────────────────────────────

echo
if command -v valgrind >/dev/null 2>&1; then
    echo "Running helgrind on threads/philo (this takes a while)..."
    helgrind_log=$(timeout 40 valgrind --tool=helgrind --error-exitcode=42 \
        "$THREADS_BIN" 4 1000 200 200 3 2>&1)
    if [[ "$?" -eq 42 ]]; then
        fail "threads: helgrind reports no data races" \
            "$(echo "$helgrind_log" | grep -A3 "^==.*ERROR SUMMARY")"
    else
        pass "threads: helgrind reports no data races"
    fi
else
    echo "  (valgrind not installed — skipping helgrind check)"
fi

# ── summary ───────────────────────────────────────────────────────────────────

echo
hr
printf "  ${C_BOLD}%d passed, %d failed${C_RESET}\n" "$pass_count" "$fail_count"
hr

if [[ "$fail_count" -gt 0 ]]; then
    exit 1
fi
exit 0
