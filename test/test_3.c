#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <hmc_global_allocator.h>
#include <hmc_thread.h>
#include <hmc_condvar.h>
#include <hmc_lock.h>
#include <hmc_sema.h>
#include <hmc_results.h>

/* ------------------------------------------------------------------------- */
/* Tunables                                                                  */
/* ------------------------------------------------------------------------- */

#define WATCHDOG_SECONDS 120
#define NTHREADS 8
#define MUTEX_ITERS 20000
#define SEMA_ITERS 300
#define SETTLE_MS 150 /* how long we wait to prove "nothing happened"      */
#define WAIT_MS 5000  /* max time we wait for "something to happen"        */
#define QCAP 8
#define RING_CAP 4
#define ROUNDS 200

/* ------------------------------------------------------------------------- */
/* Tiny test harness (thread-safe counters)                                  */
/* ------------------------------------------------------------------------- */

static atomic_int g_checks;
static atomic_int g_failures;

#define CHECK(cond)                                                           \
    do {                                                                      \
        atomic_fetch_add(&g_checks, 1);                                       \
        if (!(cond)) {                                                        \
            atomic_fetch_add(&g_failures, 1);                                 \
            fprintf(stderr, "    FAIL %s:%d: %s\n", __FILE__, __LINE__,       \
                    #cond);                                                   \
        }                                                                     \
    } while (0)

#define CHECK_OK(expr) CHECK((expr) == HMC_RESULT_SUCCESS)
#define CHECK_RES(expr, code) CHECK((expr) == (code))

/* Abort the current test (not the process) if a precondition fails. */
#define REQUIRE(cond)                                                         \
    do {                                                                      \
        atomic_fetch_add(&g_checks, 1);                                       \
        if (!(cond)) {                                                        \
            atomic_fetch_add(&g_failures, 1);                                 \
            fprintf(stderr, "    FAIL(required) %s:%d: %s\n", __FILE__,       \
                    __LINE__, #cond);                                         \
            return;                                                           \
        }                                                                     \
    } while (0)

static void sleep_ms(int ms) {
    struct timespec ts = {ms / 1000, (long)(ms % 1000) * 1000000L};
    nanosleep(&ts, NULL);
}

/* Poll until *v >= want or timeout. Returns 1 on success. */
static int wait_until(atomic_int *v, int want, int timeout_ms) {
    for (int t = 0; t < timeout_ms; t++) {
        if (atomic_load(v) >= want) return 1;
        sleep_ms(1);
    }
    return atomic_load(v) >= want;
}

static void watchdog(int sig) {
    (void)sig;
    static const char msg[] =
        "\nWATCHDOG: test run exceeded time limit (probable deadlock)\n";
    ssize_t r = write(STDERR_FILENO, msg, sizeof msg - 1);
    (void)r;
    _exit(124);
}

/* ------------------------------------------------------------------------- */
/* Thread helpers                                                            */
/*                                                                           */
/* Per hmc_thread.h, a thread's argument must come from the global allocator */
/* and is freed by the thread function itself. Every worker therefore gets   */
/* its own small heap-allocated targ_t that points at the shared context.    */
/* ------------------------------------------------------------------------- */

typedef struct {
    void *shared;
    int id;
} targ_t;

static targ_t take_arg(void *arg) {
    targ_t a = *(targ_t *)arg;
    void *p = arg;
    hmc_g_free(&p);
    return a;
}

/* Allocate result memory of `n` bytes from the global allocator. */
static void *make_result(size_t n) {
    void *p = NULL;
    if (hmc_g_alloc(&p, (hsize_t)n) != HMC_RESULT_SUCCESS) return NULL;
    return p;
}

/* Init + launch a thread. Returns 0 on success, -1 on failure. */
static int spawn_ex(HMCThread **t, hmcthreadexec_t fn, void *shared, int id,
                    int detached) {
    void *mem = NULL;
    if (hmc_thread_init(t) != HMC_RESULT_SUCCESS) return -1;
    if (hmc_g_alloc(&mem, sizeof(targ_t)) != HMC_RESULT_SUCCESS) {
        hmc_thread_deinit(t);
        return -1;
    }
    targ_t *a = mem;
    a->shared = shared;
    a->id = id;
    hmcResult_t r = detached ? hmc_thread_launch_detached(*t, fn, a)
                             : hmc_thread_launch(*t, fn, a);
    if (r != HMC_RESULT_SUCCESS) {
        hmc_g_free(&mem);
        hmc_thread_deinit(t);
        return -1;
    }
    return 0;
}

static int spawn(HMCThread **t, hmcthreadexec_t fn, void *shared, int id) {
    return spawn_ex(t, fn, shared, id, 0);
}

/* Join, fetch the result, deinit. Caller owns (and must free) *res. */
static void join_result(HMCThread **t, void **res) {
    void *r = NULL;
    CHECK_OK(hmc_thread_join(*t));
    CHECK_OK(hmc_thread_get_result(*t, &r));
    CHECK_OK(hmc_thread_deinit(t));
    if (res) {
        *res = r;
    } else if (r) {
        hmc_g_free(&r);
    }
}

/* Join + discard result. */
static void join_all(HMCThread **ts, int n) {
    for (int i = 0; i < n; i++) join_result(&ts[i], NULL);
}

/* ========================================================================= */
/* MUTEX                                                                     */
/* ========================================================================= */

static void test_mutex_init_deinit(void) {
    HMCMutexLock *l = NULL;
    CHECK_OK(hmc_mutex_init(&l));
    CHECK(l != NULL);
    CHECK_OK(hmc_mutex_deinit(&l));
}

static void test_mutex_invalid_args(void) {
    CHECK_RES(hmc_mutex_init(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_mutex_deinit(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_mutex_lock(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_mutex_unlock(NULL), HMC_RESULT_INVALID_ARGUMENT);
}

static void test_mutex_double_init(void) {
    HMCMutexLock *l = NULL;
    REQUIRE(hmc_mutex_init(&l) == HMC_RESULT_SUCCESS);
    CHECK_RES(hmc_mutex_init(&l), HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK_OK(hmc_mutex_deinit(&l));
}

static void test_mutex_deinit_uninitialized(void) {
    HMCMutexLock *l = NULL;
    hmcResult_t r = hmc_mutex_deinit(&l);
    CHECK(r == HMC_RESULT_INVALID_ARGUMENT_STATE ||
          r == HMC_RESULT_INVALID_ARGUMENT);
}

static void test_mutex_lock_unlock_basic(void) {
    HMCMutexLock *l = NULL;
    REQUIRE(hmc_mutex_init(&l) == HMC_RESULT_SUCCESS);
    for (int i = 0; i < 100; i++) {
        CHECK_OK(hmc_mutex_lock(l));
        CHECK_OK(hmc_mutex_unlock(l));
    }
    CHECK_OK(hmc_mutex_deinit(&l));
}

static void test_mutex_two_independent_locks(void) {
    HMCMutexLock *a = NULL, *b = NULL;
    REQUIRE(hmc_mutex_init(&a) == HMC_RESULT_SUCCESS);
    REQUIRE(hmc_mutex_init(&b) == HMC_RESULT_SUCCESS);
    CHECK_OK(hmc_mutex_lock(a));
    CHECK_OK(hmc_mutex_lock(b)); /* must not block: different mutex */
    CHECK_OK(hmc_mutex_unlock(b));
    CHECK_OK(hmc_mutex_unlock(a));
    CHECK_OK(hmc_mutex_deinit(&a));
    CHECK_OK(hmc_mutex_deinit(&b));
}

typedef struct {
    HMCMutexLock *lock;
    long counter; /* deliberately non-atomic: protected only by `lock` */
} counter_ctx;

static void *counter_worker(void *arg) {
    targ_t a = take_arg(arg);
    counter_ctx *c = a.shared;
    for (int i = 0; i < MUTEX_ITERS; i++) {
        hmc_mutex_lock(c->lock);
        c->counter++;
        hmc_mutex_unlock(c->lock);
    }
    return NULL;
}

static void test_mutex_mutual_exclusion(void) {
    counter_ctx c = {0};
    HMCThread *ts[NTHREADS];
    REQUIRE(hmc_mutex_init(&c.lock) == HMC_RESULT_SUCCESS);
    for (int i = 0; i < NTHREADS; i++)
        REQUIRE(spawn(&ts[i], counter_worker, &c, i) == 0);
    join_all(ts, NTHREADS);
    CHECK(c.counter == (long)NTHREADS * MUTEX_ITERS);
    CHECK_OK(hmc_mutex_deinit(&c.lock));
}

typedef struct {
    HMCMutexLock *lock;
    atomic_int started;
    atomic_int acquired;
} block_ctx;

static void *block_worker(void *arg) {
    targ_t a = take_arg(arg);
    block_ctx *c = a.shared;
    atomic_store(&c->started, 1);
    hmc_mutex_lock(c->lock);
    atomic_store(&c->acquired, 1);
    hmc_mutex_unlock(c->lock);
    return NULL;
}

static void test_mutex_blocks_until_released(void) {
    block_ctx c;
    HMCThread *t = NULL;
    atomic_init(&c.started, 0);
    atomic_init(&c.acquired, 0);
    REQUIRE(hmc_mutex_init(&c.lock) == HMC_RESULT_SUCCESS);
    CHECK_OK(hmc_mutex_lock(c.lock));
    REQUIRE(spawn(&t, block_worker, &c, 0) == 0);
    CHECK(wait_until(&c.started, 1, WAIT_MS));
    sleep_ms(SETTLE_MS);
    CHECK(atomic_load(&c.acquired) == 0); /* still blocked */
    CHECK_OK(hmc_mutex_unlock(c.lock));
    CHECK(wait_until(&c.acquired, 1, WAIT_MS));
    join_result(&t, NULL);
    CHECK_OK(hmc_mutex_deinit(&c.lock));
}

/* ========================================================================= */
/* CONDITION VARIABLE                                                        */
/* ========================================================================= */

static void test_condvar_init_deinit(void) {
    HMCConditionVariable *cv = NULL;
    CHECK_OK(hmc_condvar_init(&cv));
    CHECK(cv != NULL);
    CHECK_OK(hmc_condvar_deinit(&cv));
}

static void test_condvar_invalid_args(void) {
    HMCMutexLock *l = NULL;
    HMCConditionVariable *cv = NULL;
    REQUIRE(hmc_mutex_init(&l) == HMC_RESULT_SUCCESS);
    REQUIRE(hmc_condvar_init(&cv) == HMC_RESULT_SUCCESS);

    CHECK_RES(hmc_condvar_init(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_condvar_deinit(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_condvar_signal(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_condvar_broadcast(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_condvar_wait(NULL, l), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_condvar_wait(cv, NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_condvar_wait(NULL, NULL), HMC_RESULT_INVALID_ARGUMENT);

    CHECK_OK(hmc_condvar_deinit(&cv));
    CHECK_OK(hmc_mutex_deinit(&l));
}

static void test_condvar_double_init(void) {
    HMCConditionVariable *cv = NULL;
    REQUIRE(hmc_condvar_init(&cv) == HMC_RESULT_SUCCESS);
    CHECK_RES(hmc_condvar_init(&cv), HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK_OK(hmc_condvar_deinit(&cv));
}

static void test_condvar_deinit_uninitialized(void) {
    HMCConditionVariable *cv = NULL;
    hmcResult_t r = hmc_condvar_deinit(&cv);
    CHECK(r == HMC_RESULT_INVALID_ARGUMENT_STATE ||
          r == HMC_RESULT_INVALID_ARGUMENT);
}

static void test_condvar_signal_broadcast_no_waiters(void) {
    HMCConditionVariable *cv = NULL;
    REQUIRE(hmc_condvar_init(&cv) == HMC_RESULT_SUCCESS);
    for (int i = 0; i < 10; i++) {
        CHECK_OK(hmc_condvar_signal(cv));
        CHECK_OK(hmc_condvar_broadcast(cv));
    }
    CHECK_OK(hmc_condvar_deinit(&cv));
}

typedef struct {
    HMCMutexLock *lock;
    HMCConditionVariable *cv;
    int go;     /* predicate for broadcast tests   (guarded by lock) */
    int tokens; /* predicate for signal tests      (guarded by lock) */
    atomic_int waiting;
    atomic_int woke;
} cv_ctx;

static int cv_ctx_init(cv_ctx *c) {
    memset(c, 0, sizeof *c);
    atomic_init(&c->waiting, 0);
    atomic_init(&c->woke, 0);
    if (hmc_mutex_init(&c->lock) != HMC_RESULT_SUCCESS) return -1;
    if (hmc_condvar_init(&c->cv) != HMC_RESULT_SUCCESS) return -1;
    return 0;
}

static void cv_ctx_destroy(cv_ctx *c) {
    CHECK_OK(hmc_condvar_deinit(&c->cv));
    CHECK_OK(hmc_mutex_deinit(&c->lock));
}

static void *cv_waiter_go(void *arg) {
    targ_t a = take_arg(arg);
    cv_ctx *c = a.shared;
    hmc_mutex_lock(c->lock);
    atomic_fetch_add(&c->waiting, 1);
    while (!c->go) hmc_condvar_wait(c->cv, c->lock);
    atomic_fetch_add(&c->woke, 1);
    hmc_mutex_unlock(c->lock);
    return NULL;
}

static void *cv_waiter_token(void *arg) {
    targ_t a = take_arg(arg);
    cv_ctx *c = a.shared;
    hmc_mutex_lock(c->lock);
    atomic_fetch_add(&c->waiting, 1);
    while (c->tokens == 0) hmc_condvar_wait(c->cv, c->lock);
    c->tokens--;
    atomic_fetch_add(&c->woke, 1);
    hmc_mutex_unlock(c->lock);
    return NULL;
}

static void test_condvar_broadcast_wakes_all(void) {
    cv_ctx c;
    HMCThread *ts[NTHREADS];
    REQUIRE(cv_ctx_init(&c) == 0);
    for (int i = 0; i < NTHREADS; i++)
        REQUIRE(spawn(&ts[i], cv_waiter_go, &c, i) == 0);
    CHECK(wait_until(&c.waiting, NTHREADS, WAIT_MS));
    sleep_ms(SETTLE_MS);
    CHECK(atomic_load(&c.woke) == 0); /* nobody leaves without a wake-up */

    CHECK_OK(hmc_mutex_lock(c.lock));
    c.go = 1;
    CHECK_OK(hmc_condvar_broadcast(c.cv));
    CHECK_OK(hmc_mutex_unlock(c.lock));

    CHECK(wait_until(&c.woke, NTHREADS, WAIT_MS));
    join_all(ts, NTHREADS);
    CHECK(atomic_load(&c.woke) == NTHREADS);
    cv_ctx_destroy(&c);
}

static void test_condvar_signal_wakes_one(void) {
    const int n = 4;
    cv_ctx c;
    HMCThread *ts[4];
    REQUIRE(cv_ctx_init(&c) == 0);
    for (int i = 0; i < n; i++)
        REQUIRE(spawn(&ts[i], cv_waiter_token, &c, i) == 0);
    CHECK(wait_until(&c.waiting, n, WAIT_MS));
    sleep_ms(SETTLE_MS);
    CHECK(atomic_load(&c.woke) == 0);

    /* One token + one signal => exactly one waiter proceeds. */
    CHECK_OK(hmc_mutex_lock(c.lock));
    c.tokens = 1;
    CHECK_OK(hmc_condvar_signal(c.cv));
    CHECK_OK(hmc_mutex_unlock(c.lock));
    CHECK(wait_until(&c.woke, 1, WAIT_MS));
    sleep_ms(SETTLE_MS);
    CHECK(atomic_load(&c.woke) == 1);

    /* Release the rest. */
    CHECK_OK(hmc_mutex_lock(c.lock));
    c.tokens = n - 1;
    CHECK_OK(hmc_condvar_broadcast(c.cv));
    CHECK_OK(hmc_mutex_unlock(c.lock));
    CHECK(wait_until(&c.woke, n, WAIT_MS));
    join_all(ts, n);
    cv_ctx_destroy(&c);
}

static void test_condvar_wait_releases_lock(void) {
    /* If wait() failed to release the mutex, the lock() below would deadlock
     * and the watchdog would flag it. */
    cv_ctx c;
    HMCThread *t = NULL;
    REQUIRE(cv_ctx_init(&c) == 0);
    REQUIRE(spawn(&t, cv_waiter_go, &c, 0) == 0);
    CHECK(wait_until(&c.waiting, 1, WAIT_MS));

    CHECK_OK(hmc_mutex_lock(c.lock)); /* only possible if waiter released it */
    c.go = 1;
    CHECK_OK(hmc_condvar_signal(c.cv));
    CHECK_OK(hmc_mutex_unlock(c.lock));

    CHECK(wait_until(&c.woke, 1, WAIT_MS));
    join_result(&t, NULL);
    cv_ctx_destroy(&c);
}

static void test_condvar_wait_reacquires_lock(void) {
    cv_ctx c;
    HMCThread *t = NULL;
    REQUIRE(cv_ctx_init(&c) == 0);
    REQUIRE(spawn(&t, cv_waiter_go, &c, 0) == 0);
    CHECK(wait_until(&c.waiting, 1, WAIT_MS));

    CHECK_OK(hmc_mutex_lock(c.lock));
    c.go = 1;
    CHECK_OK(hmc_condvar_signal(c.cv));
    sleep_ms(SETTLE_MS);
    /* We still hold the mutex, so the woken thread must not have returned
     * from wait() yet. */
    CHECK(atomic_load(&c.woke) == 0);
    CHECK_OK(hmc_mutex_unlock(c.lock));

    CHECK(wait_until(&c.woke, 1, WAIT_MS));
    join_result(&t, NULL);
    cv_ctx_destroy(&c);
}

/* Ping-pong: two threads strictly alternate using one condvar. */
typedef struct {
    HMCMutexLock *lock;
    HMCConditionVariable *cv;
    int turn;
    int last;
    long seq;
    atomic_int errors;
} pingpong_ctx;

static void *pingpong_worker(void *arg) {
    targ_t a = take_arg(arg);
    pingpong_ctx *c = a.shared;
    for (int i = 0; i < 1000; i++) {
        hmc_mutex_lock(c->lock);
        while (c->turn != a.id) hmc_condvar_wait(c->cv, c->lock);
        if (c->last == a.id) atomic_fetch_add(&c->errors, 1);
        c->last = a.id;
        c->seq++;
        c->turn = 1 - a.id;
        hmc_condvar_broadcast(c->cv);
        hmc_mutex_unlock(c->lock);
    }
    return NULL;
}

static void test_condvar_ping_pong(void) {
    pingpong_ctx c;
    HMCThread *ts[2];
    memset(&c, 0, sizeof c);
    atomic_init(&c.errors, 0);
    c.last = -1;
    REQUIRE(hmc_mutex_init(&c.lock) == HMC_RESULT_SUCCESS);
    REQUIRE(hmc_condvar_init(&c.cv) == HMC_RESULT_SUCCESS);
    REQUIRE(spawn(&ts[0], pingpong_worker, &c, 0) == 0);
    REQUIRE(spawn(&ts[1], pingpong_worker, &c, 1) == 0);
    join_all(ts, 2);
    CHECK(c.seq == 2000);
    CHECK(atomic_load(&c.errors) == 0);
    CHECK_OK(hmc_condvar_deinit(&c.cv));
    CHECK_OK(hmc_mutex_deinit(&c.lock));
}

/* Bounded buffer: multiple producers / consumers, two condvars. */
typedef struct {
    HMCMutexLock *lock;
    HMCConditionVariable *not_full;
    HMCConditionVariable *not_empty;
    int buf[QCAP];
    int head, tail, count;
    int done;
    int per_producer;
    long long consumed_sum;
    long consumed_n;
} queue_ctx;

static void *queue_producer(void *arg) {
    targ_t a = take_arg(arg);
    queue_ctx *q = a.shared;
    for (int v = 1; v <= q->per_producer; v++) {
        hmc_mutex_lock(q->lock);
        while (q->count == QCAP) hmc_condvar_wait(q->not_full, q->lock);
        q->buf[q->tail] = v;
        q->tail = (q->tail + 1) % QCAP;
        q->count++;
        hmc_condvar_signal(q->not_empty);
        hmc_mutex_unlock(q->lock);
    }
    return NULL;
}

static void *queue_consumer(void *arg) {
    targ_t a = take_arg(arg);
    queue_ctx *q = a.shared;
    for (;;) {
        hmc_mutex_lock(q->lock);
        while (q->count == 0 && !q->done)
            hmc_condvar_wait(q->not_empty, q->lock);
        if (q->count == 0 && q->done) {
            hmc_mutex_unlock(q->lock);
            break;
        }
        int v = q->buf[q->head];
        q->head = (q->head + 1) % QCAP;
        q->count--;
        q->consumed_sum += v;
        q->consumed_n++;
        hmc_condvar_signal(q->not_full);
        hmc_mutex_unlock(q->lock);
    }
    return NULL;
}

static void test_condvar_bounded_queue(void) {
    enum { P = 3, C = 3 };
    queue_ctx q;
    HMCThread *prod[P], *cons[C];
    memset(&q, 0, sizeof q);
    q.per_producer = 2000;
    REQUIRE(hmc_mutex_init(&q.lock) == HMC_RESULT_SUCCESS);
    REQUIRE(hmc_condvar_init(&q.not_full) == HMC_RESULT_SUCCESS);
    REQUIRE(hmc_condvar_init(&q.not_empty) == HMC_RESULT_SUCCESS);

    for (int i = 0; i < C; i++)
        REQUIRE(spawn(&cons[i], queue_consumer, &q, i) == 0);
    for (int i = 0; i < P; i++)
        REQUIRE(spawn(&prod[i], queue_producer, &q, i) == 0);

    join_all(prod, P);

    CHECK_OK(hmc_mutex_lock(q.lock));
    q.done = 1;
    CHECK_OK(hmc_condvar_broadcast(q.not_empty));
    CHECK_OK(hmc_mutex_unlock(q.lock));

    join_all(cons, C);

    long long n = q.per_producer;
    CHECK(q.consumed_n == (long)P * q.per_producer);
    CHECK(q.consumed_sum == (long long)P * (n * (n + 1) / 2));
    CHECK(q.count == 0);

    CHECK_OK(hmc_condvar_deinit(&q.not_full));
    CHECK_OK(hmc_condvar_deinit(&q.not_empty));
    CHECK_OK(hmc_mutex_deinit(&q.lock));
}

/* Barrier built from mutex + condvar: nobody may leave round r early. */
typedef struct {
    HMCMutexLock *lock;
    HMCConditionVariable *cv;
    int n, count, gen;
    atomic_int arrived[ROUNDS];
    atomic_int errors;
} barrier_ctx;

static void barrier_wait(barrier_ctx *b) {
    hmc_mutex_lock(b->lock);
    int g = b->gen;
    if (++b->count == b->n) {
        b->count = 0;
        b->gen++;
        hmc_condvar_broadcast(b->cv);
    } else {
        while (g == b->gen) hmc_condvar_wait(b->cv, b->lock);
    }
    hmc_mutex_unlock(b->lock);
}

static void *barrier_worker(void *arg) {
    targ_t a = take_arg(arg);
    barrier_ctx *b = a.shared;
    for (int r = 0; r < ROUNDS; r++) {
        atomic_fetch_add(&b->arrived[r], 1);
        barrier_wait(b);
        if (atomic_load(&b->arrived[r]) != b->n)
            atomic_fetch_add(&b->errors, 1);
    }
    return NULL;
}

static void test_condvar_barrier(void) {
    barrier_ctx b;
    HMCThread *ts[NTHREADS];
    memset(&b, 0, sizeof b);
    b.n = NTHREADS;
    atomic_init(&b.errors, 0);
    for (int i = 0; i < ROUNDS; i++) atomic_init(&b.arrived[i], 0);
    REQUIRE(hmc_mutex_init(&b.lock) == HMC_RESULT_SUCCESS);
    REQUIRE(hmc_condvar_init(&b.cv) == HMC_RESULT_SUCCESS);
    for (int i = 0; i < NTHREADS; i++)
        REQUIRE(spawn(&ts[i], barrier_worker, &b, i) == 0);
    join_all(ts, NTHREADS);
    CHECK(atomic_load(&b.errors) == 0);
    CHECK(b.gen == ROUNDS);
    CHECK_OK(hmc_condvar_deinit(&b.cv));
    CHECK_OK(hmc_mutex_deinit(&b.lock));
}

/* ========================================================================= */
/* SEMAPHORE                                                                 */
/* ========================================================================= */

static void test_sema_invalid_args(void) {
    HMCSema *s = NULL;
    CHECK_RES(hmc_sema_init(NULL, 1), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_sema_init(&s, 0), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_sema_deinit(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_sema_post(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_sema_wait(NULL), HMC_RESULT_INVALID_ARGUMENT);
}

static void test_sema_init_deinit(void) {
    HMCSema *s = NULL;
    CHECK_OK(hmc_sema_init(&s, 1));
    CHECK(s != NULL);
    CHECK_OK(hmc_sema_deinit(&s));
}

static void test_sema_double_init(void) {
    HMCSema *s = NULL;
    REQUIRE(hmc_sema_init(&s, 2) == HMC_RESULT_SUCCESS);
    CHECK_RES(hmc_sema_init(&s, 2), HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK_OK(hmc_sema_deinit(&s));
}

static void test_sema_deinit_uninitialized(void) {
    HMCSema *s = NULL;
    hmcResult_t r = hmc_sema_deinit(&s);
    CHECK(r == HMC_RESULT_INVALID_ARGUMENT_STATE ||
          r == HMC_RESULT_INVALID_ARGUMENT);
}

static void test_sema_wait_post_basic(void) {
    HMCSema *s = NULL;
    REQUIRE(hmc_sema_init(&s, 1) == HMC_RESULT_SUCCESS);
    for (int i = 0; i < 100; i++) {
        CHECK_OK(hmc_sema_wait(s));
        CHECK_OK(hmc_sema_post(s));
    }
    CHECK_OK(hmc_sema_deinit(&s));
}

static void test_sema_initial_count_nonblocking(void) {
    /* With init_size == 3, three waits in a row must not block. */
    HMCSema *s = NULL;
    REQUIRE(hmc_sema_init(&s, 3) == HMC_RESULT_SUCCESS);
    CHECK_OK(hmc_sema_wait(s));
    CHECK_OK(hmc_sema_wait(s));
    CHECK_OK(hmc_sema_wait(s));
    CHECK_OK(hmc_sema_post(s));
    CHECK_OK(hmc_sema_post(s));
    CHECK_OK(hmc_sema_post(s));
    CHECK_OK(hmc_sema_deinit(&s));
}

typedef struct {
    HMCSema *sem;
    atomic_int started;
    atomic_int acquired;
} sema_block_ctx;

static void *sema_block_worker(void *arg) {
    targ_t a = take_arg(arg);
    sema_block_ctx *c = a.shared;
    atomic_store(&c->started, 1);
    hmc_sema_wait(c->sem);
    atomic_store(&c->acquired, 1);
    hmc_sema_post(c->sem);
    return NULL;
}

static void test_sema_wait_blocks_when_exhausted(void) {
    sema_block_ctx c;
    HMCThread *t = NULL;
    atomic_init(&c.started, 0);
    atomic_init(&c.acquired, 0);
    REQUIRE(hmc_sema_init(&c.sem, 1) == HMC_RESULT_SUCCESS);
    CHECK_OK(hmc_sema_wait(c.sem)); /* exhaust */
    REQUIRE(spawn(&t, sema_block_worker, &c, 0) == 0);
    CHECK(wait_until(&c.started, 1, WAIT_MS));
    sleep_ms(SETTLE_MS);
    CHECK(atomic_load(&c.acquired) == 0);
    CHECK_OK(hmc_sema_post(c.sem));
    CHECK(wait_until(&c.acquired, 1, WAIT_MS));
    join_result(&t, NULL);
    CHECK_OK(hmc_sema_deinit(&c.sem));
}

/* A semaphore cannot be created with 0, but it can be drained to 0 and then
 * used as a "wake-up" signal between threads. */
static void test_sema_drained_then_signalled(void) {
    sema_block_ctx c;
    HMCThread *t = NULL;
    atomic_init(&c.started, 0);
    atomic_init(&c.acquired, 0);
    REQUIRE(hmc_sema_init(&c.sem, 1) == HMC_RESULT_SUCCESS);
    CHECK_OK(hmc_sema_wait(c.sem));
    REQUIRE(spawn(&t, sema_block_worker, &c, 0) == 0);
    CHECK(wait_until(&c.started, 1, WAIT_MS));
    sleep_ms(SETTLE_MS);
    CHECK(atomic_load(&c.acquired) == 0);
    CHECK_OK(hmc_sema_post(c.sem));
    CHECK(wait_until(&c.acquired, 1, WAIT_MS));
    join_result(&t, NULL);
    CHECK_OK(hmc_sema_deinit(&c.sem));
}

typedef struct {
    HMCSema *sem;
    atomic_int current;
    atomic_int max_seen;
} limit_ctx;

static void *sema_limit_worker(void *arg) {
    targ_t a = take_arg(arg);
    limit_ctx *c = a.shared;
    for (int i = 0; i < SEMA_ITERS; i++) {
        hmc_sema_wait(c->sem);
        int cur = atomic_fetch_add(&c->current, 1) + 1;
        int prev = atomic_load(&c->max_seen);
        while (cur > prev &&
               !atomic_compare_exchange_weak(&c->max_seen, &prev, cur)) {
        }
        struct timespec ts = {0, 50000}; /* 50us of "work" */
        nanosleep(&ts, NULL);
        atomic_fetch_sub(&c->current, 1);
        hmc_sema_post(c->sem);
    }
    return NULL;
}

static void test_sema_limits_concurrency(void) {
    enum { K = 3, N = 10 };
    limit_ctx c;
    HMCThread *ts[N];
    atomic_init(&c.current, 0);
    atomic_init(&c.max_seen, 0);
    REQUIRE(hmc_sema_init(&c.sem, K) == HMC_RESULT_SUCCESS);
    for (int i = 0; i < N; i++)
        REQUIRE(spawn(&ts[i], sema_limit_worker, &c, i) == 0);
    join_all(ts, N);
    CHECK(atomic_load(&c.max_seen) <= K);
    CHECK(atomic_load(&c.max_seen) >= 1);
    CHECK(atomic_load(&c.current) == 0);
    CHECK_OK(hmc_sema_deinit(&c.sem));
}

typedef struct {
    HMCSema *sem;
    long counter; /* non-atomic: protected only by the binary semaphore */
} sema_counter_ctx;

static void *sema_counter_worker(void *arg) {
    targ_t a = take_arg(arg);
    sema_counter_ctx *c = a.shared;
    for (int i = 0; i < MUTEX_ITERS; i++) {
        hmc_sema_wait(c->sem);
        c->counter++;
        hmc_sema_post(c->sem);
    }
    return NULL;
}

static void test_sema_binary_as_mutex(void) {
    sema_counter_ctx c = {0};
    HMCThread *ts[NTHREADS];
    REQUIRE(hmc_sema_init(&c.sem, 1) == HMC_RESULT_SUCCESS);
    for (int i = 0; i < NTHREADS; i++)
        REQUIRE(spawn(&ts[i], sema_counter_worker, &c, i) == 0);
    join_all(ts, NTHREADS);
    CHECK(c.counter == (long)NTHREADS * MUTEX_ITERS);
    CHECK_OK(hmc_sema_deinit(&c.sem));
}

/* Single producer / single consumer ring buffer using two semaphores + mutex. */
typedef struct {
    HMCSema *empty;
    HMCSema *full;
    HMCMutexLock *lock;
    int buf[RING_CAP];
    int head, tail;
    int items;
    atomic_int errors;
} ring_ctx;

static void *ring_producer(void *arg) {
    targ_t a = take_arg(arg);
    ring_ctx *r = a.shared;
    for (int i = 0; i < r->items; i++) {
        hmc_sema_wait(r->empty);
        hmc_mutex_lock(r->lock);
        r->buf[r->tail] = i;
        r->tail = (r->tail + 1) % RING_CAP;
        hmc_mutex_unlock(r->lock);
        hmc_sema_post(r->full);
    }
    return NULL;
}

static void *ring_consumer(void *arg) {
    targ_t a = take_arg(arg);
    ring_ctx *r = a.shared;
    for (int i = 0; i < r->items; i++) {
        hmc_sema_wait(r->full);
        hmc_mutex_lock(r->lock);
        int v = r->buf[r->head];
        r->head = (r->head + 1) % RING_CAP;
        hmc_mutex_unlock(r->lock);
        if (v != i) atomic_fetch_add(&r->errors, 1);
        hmc_sema_post(r->empty);
    }
    return NULL;
}

static void test_sema_producer_consumer_ring(void) {
    ring_ctx r;
    HMCThread *p = NULL, *c = NULL;
    memset(&r, 0, sizeof r);
    atomic_init(&r.errors, 0);
    r.items = 5000;
    REQUIRE(hmc_sema_init(&r.empty, RING_CAP) == HMC_RESULT_SUCCESS);
    REQUIRE(hmc_sema_init(&r.full, 1) == HMC_RESULT_SUCCESS);
    CHECK_OK(hmc_sema_wait(r.full)); /* drain to 0: no items yet */
    REQUIRE(hmc_mutex_init(&r.lock) == HMC_RESULT_SUCCESS);

    REQUIRE(spawn(&c, ring_consumer, &r, 0) == 0);
    REQUIRE(spawn(&p, ring_producer, &r, 1) == 0);
    join_result(&p, NULL);
    join_result(&c, NULL);

    CHECK(atomic_load(&r.errors) == 0);
    CHECK_OK(hmc_sema_deinit(&r.empty));
    CHECK_OK(hmc_sema_deinit(&r.full));
    CHECK_OK(hmc_mutex_deinit(&r.lock));
}

/* ========================================================================= */
/* THREAD                                                                    */
/* ========================================================================= */

typedef struct {
    atomic_int started;
    atomic_int release;
} gate_ctx;

static void *gate_worker(void *arg) {
    targ_t a = take_arg(arg);
    gate_ctx *g = a.shared;
    atomic_store(&g->started, 1);
    wait_until(&g->release, 1, 10000);
    return NULL;
}

static void *square_worker(void *arg) {
    targ_t a = take_arg(arg);
    long *r = make_result(sizeof(long));
    if (r) *r = (long)a.id * a.id;
    return r;
}

static void *null_worker(void *arg) {
    (void)arg; /* NULL argument: nothing to free */
    return NULL;
}

static void *flag_worker(void *arg) {
    targ_t a = take_arg(arg);
    gate_ctx *g = a.shared;
    atomic_store(&g->started, 1);
    return NULL;
}

static void test_thread_init_deinit(void) {
    HMCThread *t = NULL;
    CHECK_OK(hmc_thread_init(&t));
    CHECK(t != NULL);
    CHECK_OK(hmc_thread_deinit(&t));
}

static void test_thread_invalid_args(void) {
    HMCThread *t = NULL;
    REQUIRE(hmc_thread_init(&t) == HMC_RESULT_SUCCESS);
    void *res = NULL;

    CHECK_RES(hmc_thread_init(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_thread_deinit(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_thread_launch(NULL, null_worker, NULL),
              HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_thread_launch(t, NULL, NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_thread_launch_detached(NULL, null_worker, NULL),
              HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_thread_launch_detached(t, NULL, NULL),
              HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_thread_join(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_thread_get_result(NULL, &res), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_thread_get_result(t, NULL), HMC_RESULT_INVALID_ARGUMENT);

    CHECK_OK(hmc_thread_deinit(&t));
}

static void test_thread_double_init(void) {
    HMCThread *t = NULL;
    REQUIRE(hmc_thread_init(&t) == HMC_RESULT_SUCCESS);
    CHECK_RES(hmc_thread_init(&t), HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK_OK(hmc_thread_deinit(&t));
}

static void test_thread_init_does_not_launch(void) {
    /* get_result on a never-launched thread succeeds and yields NULL. */
    HMCThread *t = NULL;
    void *res = (void *)0x1;
    REQUIRE(hmc_thread_init(&t) == HMC_RESULT_SUCCESS);
    CHECK_OK(hmc_thread_get_result(t, &res));
    CHECK(res == NULL);
    CHECK_OK(hmc_thread_deinit(&t));
}

static void test_thread_join_never_launched(void) {
    HMCThread *t = NULL;
    REQUIRE(hmc_thread_init(&t) == HMC_RESULT_SUCCESS);
    CHECK_RES(hmc_thread_join(t), HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK_OK(hmc_thread_deinit(&t));
}

static void test_thread_launch_join_result(void) {
    HMCThread *t = NULL;
    void *res = NULL;
    REQUIRE(spawn(&t, square_worker, NULL, 12) == 0);
    join_result(&t, &res);
    REQUIRE(res != NULL);
    CHECK(*(long *)res == 144);
    hmc_g_free(&res);
}

static void test_thread_null_arg_null_result(void) {
    HMCThread *t = NULL;
    void *res = (void *)0x1;
    REQUIRE(hmc_thread_init(&t) == HMC_RESULT_SUCCESS);
    CHECK_OK(hmc_thread_launch(t, null_worker, NULL));
    CHECK_OK(hmc_thread_join(t));
    CHECK_OK(hmc_thread_get_result(t, &res));
    CHECK(res == NULL);
    CHECK_OK(hmc_thread_deinit(&t));
}

static void test_thread_state_while_running(void) {
    gate_ctx g;
    HMCThread *t = NULL;
    void *res = NULL;
    atomic_init(&g.started, 0);
    atomic_init(&g.release, 0);
    REQUIRE(spawn(&t, gate_worker, &g, 0) == 0);
    CHECK(wait_until(&g.started, 1, WAIT_MS));

    /* Running thread: these must all be rejected. */
    CHECK_RES(hmc_thread_get_result(t, &res), HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK_RES(hmc_thread_deinit(&t), HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK(t != NULL); /* failed deinit must leave the handle intact */
    CHECK_RES(hmc_thread_launch(t, null_worker, NULL),
              HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK_RES(hmc_thread_launch_detached(t, null_worker, NULL),
              HMC_RESULT_INVALID_ARGUMENT_STATE);

    atomic_store(&g.release, 1);
    join_result(&t, NULL);
}

static void test_thread_join_twice(void) {
    HMCThread *t = NULL;
    REQUIRE(spawn(&t, square_worker, NULL, 3) == 0);
    CHECK_OK(hmc_thread_join(t));
    CHECK_RES(hmc_thread_join(t), HMC_RESULT_INVALID_ARGUMENT_STATE);
    void *res = NULL;
    CHECK_OK(hmc_thread_get_result(t, &res));
    if (res) hmc_g_free(&res);
    CHECK_OK(hmc_thread_deinit(&t));
}

static void test_thread_detached(void) {
    gate_ctx g;
    HMCThread *t = NULL;
    atomic_init(&g.started, 0);
    atomic_init(&g.release, 0);
    REQUIRE(spawn_ex(&t, flag_worker, &g, 0, 1) == 0);
    CHECK(wait_until(&g.started, 1, WAIT_MS));
    CHECK_RES(hmc_thread_join(t), HMC_RESULT_INVALID_ARGUMENT_STATE);
    sleep_ms(SETTLE_MS); /* let the detached thread fully exit */
    CHECK_OK(hmc_thread_deinit(&t));
}

static void test_thread_relaunch_after_join(void) {
    /* Assumes a joined HMCThread can be launched again (docs only forbid
     * launching while running). */
    HMCThread *t = NULL;
    REQUIRE(hmc_thread_init(&t) == HMC_RESULT_SUCCESS);
    for (int round = 1; round <= 3; round++) {
        void *mem = NULL;
        void *res = NULL;
        REQUIRE(hmc_g_alloc(&mem, sizeof(targ_t)) == HMC_RESULT_SUCCESS);
        targ_t *a = mem;
        a->shared = NULL;
        a->id = round;
        CHECK_OK(hmc_thread_launch(t, square_worker, a));
        CHECK_OK(hmc_thread_join(t));
        CHECK_OK(hmc_thread_get_result(t, &res));
        REQUIRE(res != NULL);
        CHECK(*(long *)res == (long)round * round);
        hmc_g_free(&res);
    }
    CHECK_OK(hmc_thread_deinit(&t));
}

static void test_thread_many_threads_results(void) {
    enum { N = 32 };
    HMCThread *ts[N];
    for (int i = 0; i < N; i++) REQUIRE(spawn(&ts[i], square_worker, NULL, i) == 0);
    for (int i = 0; i < N; i++) {
        void *res = NULL;
        join_result(&ts[i], &res);
        REQUIRE(res != NULL);
        CHECK(*(long *)res == (long)i * i);
        hmc_g_free(&res);
    }
}

static void *nested_worker(void *arg) {
    targ_t a = take_arg(arg);
    HMCThread *child = NULL;
    void *res = NULL;
    if (spawn(&child, square_worker, NULL, a.id) != 0) return NULL;
    join_result(&child, &res);
    return res; /* pass child's result up to our joiner */
}

static void test_thread_nested_launch(void) {
    HMCThread *t = NULL;
    void *res = NULL;
    REQUIRE(spawn(&t, nested_worker, NULL, 7) == 0);
    join_result(&t, &res);
    REQUIRE(res != NULL);
    CHECK(*(long *)res == 49);
    hmc_g_free(&res);
}

/* ========================================================================= */
/* GLOBAL ALLOCATOR UNDER CONCURRENCY                                        */
/* ========================================================================= */

#define ALLOC_ITERS 2000

static int all_bytes_equal(const unsigned char *p, size_t n, unsigned char v) {
    for (size_t i = 0; i < n; i++)
        if (p[i] != v) return 0;
    return 1;
}

static void *galloc_worker(void *arg) {
    targ_t a = take_arg(arg);
    int *errors = make_result(sizeof(int));
    if (!errors) return NULL;
    *errors = 0;

    for (int i = 0; i < ALLOC_ITERS; i++) {
        size_t n = 1 + (size_t)((i * 31 + a.id * 17) % 512);
        unsigned char pat = (unsigned char)((a.id * 7 + i) & 0xFF);
        void *p = NULL;

        /* alloc + fill + realloc grow + verify preserved prefix */
        if (hmc_g_alloc(&p, n) != HMC_RESULT_SUCCESS || !p) {
            (*errors)++;
            continue;
        }
        memset(p, pat, n);
        if (hmc_g_realloc(&p, n * 2) != HMC_RESULT_SUCCESS || !p) {
            (*errors)++;
            hmc_g_free(&p);
            continue;
        }
        if (!all_bytes_equal(p, n, pat)) (*errors)++;
        if (hmc_g_free(&p) != HMC_RESULT_SUCCESS) (*errors)++;

        /* ialloc: every byte must be initialised to the requested value */
        void *q = NULL;
        if (hmc_g_ialloc(&q, n, pat) != HMC_RESULT_SUCCESS || !q) {
            (*errors)++;
            continue;
        }
        if (!all_bytes_equal(q, n, pat)) (*errors)++;
        if (hmc_g_free(&q) != HMC_RESULT_SUCCESS) (*errors)++;
    }
    return errors;
}

static void test_global_allocator_concurrent(void) {
    HMCThread *ts[NTHREADS];
    for (int i = 0; i < NTHREADS; i++)
        REQUIRE(spawn(&ts[i], galloc_worker, NULL, i) == 0);
    for (int i = 0; i < NTHREADS; i++) {
        void *res = NULL;
        join_result(&ts[i], &res);
        REQUIRE(res != NULL);
        CHECK(*(int *)res == 0);
        hmc_g_free(&res);
    }
}

static void test_global_allocator_invalid_args(void) {
    void *p = NULL;
    CHECK_RES(hmc_g_alloc(NULL, 16), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_g_ialloc(NULL, 16, 0), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_g_realloc(NULL, 16), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_g_free(NULL), HMC_RESULT_INVALID_ARGUMENT);
    (void)p;
}

/* ========================================================================= */
/* COMBINED STRESS                                                           */
/* ========================================================================= */

typedef struct {
    HMCMutexLock *lock;
    HMCConditionVariable *cv;
    HMCSema *sem;
    long shared_total;
    int turnstile_open;
    atomic_int inside;
    atomic_int max_inside;
} mix_ctx;

static void *mix_worker(void *arg) {
    targ_t a = take_arg(arg);
    mix_ctx *m = a.shared;

    /* Wait for the start gate (condvar). */
    hmc_mutex_lock(m->lock);
    while (!m->turnstile_open) hmc_condvar_wait(m->cv, m->lock);
    hmc_mutex_unlock(m->lock);

    for (int i = 0; i < 500; i++) {
        hmc_sema_wait(m->sem); /* at most 2 threads in here */
        int cur = atomic_fetch_add(&m->inside, 1) + 1;
        int prev = atomic_load(&m->max_inside);
        while (cur > prev &&
               !atomic_compare_exchange_weak(&m->max_inside, &prev, cur)) {
        }
        hmc_mutex_lock(m->lock); /* mutex nested inside the semaphore */
        m->shared_total += a.id + 1;
        hmc_mutex_unlock(m->lock);
        atomic_fetch_sub(&m->inside, 1);
        hmc_sema_post(m->sem);
    }
    return NULL;
}

static void test_stress_mutex_condvar_sema_together(void) {
    mix_ctx m;
    HMCThread *ts[NTHREADS];
    memset(&m, 0, sizeof m);
    atomic_init(&m.inside, 0);
    atomic_init(&m.max_inside, 0);
    REQUIRE(hmc_mutex_init(&m.lock) == HMC_RESULT_SUCCESS);
    REQUIRE(hmc_condvar_init(&m.cv) == HMC_RESULT_SUCCESS);
    REQUIRE(hmc_sema_init(&m.sem, 2) == HMC_RESULT_SUCCESS);

    for (int i = 0; i < NTHREADS; i++)
        REQUIRE(spawn(&ts[i], mix_worker, &m, i) == 0);
    sleep_ms(50);
    CHECK_OK(hmc_mutex_lock(m.lock));
    m.turnstile_open = 1;
    CHECK_OK(hmc_condvar_broadcast(m.cv));
    CHECK_OK(hmc_mutex_unlock(m.lock));
    join_all(ts, NTHREADS);

    long expected = 0;
    for (int i = 0; i < NTHREADS; i++) expected += 500L * (i + 1);
    CHECK(m.shared_total == expected);
    CHECK(atomic_load(&m.max_inside) <= 2);

    CHECK_OK(hmc_sema_deinit(&m.sem));
    CHECK_OK(hmc_condvar_deinit(&m.cv));
    CHECK_OK(hmc_mutex_deinit(&m.lock));
}

/* ========================================================================= */
/* Runner                                                                    */
/* ========================================================================= */

typedef struct {
    const char *name;
    void (*fn)(void);
} test_case;

#define T(fn) {#fn, fn}

static const test_case TESTS[] = {
    /* mutex */
    T(test_mutex_init_deinit),
    T(test_mutex_invalid_args),
    T(test_mutex_double_init),
    T(test_mutex_deinit_uninitialized),
    T(test_mutex_lock_unlock_basic),
    T(test_mutex_two_independent_locks),
    T(test_mutex_mutual_exclusion),
    T(test_mutex_blocks_until_released),
    /* condvar */
    T(test_condvar_init_deinit),
    T(test_condvar_invalid_args),
    T(test_condvar_double_init),
    T(test_condvar_deinit_uninitialized),
    T(test_condvar_signal_broadcast_no_waiters),
    T(test_condvar_broadcast_wakes_all),
    T(test_condvar_signal_wakes_one),
    T(test_condvar_wait_releases_lock),
    T(test_condvar_wait_reacquires_lock),
    T(test_condvar_ping_pong),
    T(test_condvar_bounded_queue),
    T(test_condvar_barrier),
    /* semaphore */
    T(test_sema_invalid_args),
    T(test_sema_init_deinit),
    T(test_sema_double_init),
    T(test_sema_deinit_uninitialized),
    T(test_sema_wait_post_basic),
    T(test_sema_initial_count_nonblocking),
    T(test_sema_wait_blocks_when_exhausted),
    T(test_sema_drained_then_signalled),
    T(test_sema_limits_concurrency),
    T(test_sema_binary_as_mutex),
    T(test_sema_producer_consumer_ring),
    /* thread */
    T(test_thread_init_deinit),
    T(test_thread_invalid_args),
    T(test_thread_double_init),
    T(test_thread_init_does_not_launch),
    T(test_thread_join_never_launched),
    T(test_thread_launch_join_result),
    T(test_thread_null_arg_null_result),
    T(test_thread_state_while_running),
    T(test_thread_join_twice),
    T(test_thread_detached),
    T(test_thread_relaunch_after_join),
    T(test_thread_many_threads_results),
    T(test_thread_nested_launch),
    /* global allocator + threads */
    T(test_global_allocator_invalid_args),
    T(test_global_allocator_concurrent),
    /* combined */
    T(test_stress_mutex_condvar_sema_together),
};

int main(int argc, char **argv) {
    const char *filter = argc > 1 ? argv[1] : NULL;
    int ran = 0, failed_tests = 0;

    signal(SIGALRM, watchdog);
    alarm(WATCHDOG_SECONDS);

    for (size_t i = 0; i < sizeof TESTS / sizeof TESTS[0]; i++) {
        if (filter && !strstr(TESTS[i].name, filter)) continue;
        int before = atomic_load(&g_failures);
        printf("[ RUN  ] %s\n", TESTS[i].name);
        fflush(stdout);
        TESTS[i].fn();
        ran++;
        if (atomic_load(&g_failures) != before) {
            failed_tests++;
            printf("[ FAIL ] %s\n", TESTS[i].name);
        } else {
            printf("[  OK  ] %s\n", TESTS[i].name);
        }
    }

    printf("\n%d test(s) run, %d failed, %d check(s), %d check failure(s)\n",
           ran, failed_tests, atomic_load(&g_checks),
           atomic_load(&g_failures));
    return failed_tests ? 1 : 0;
}
