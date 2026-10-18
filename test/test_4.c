#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <hmc_linear_allocator.h>
#include <hmc_allocator.h>
#include <hmc_dynamic_array.h>
#include <hmc_results.h>

/* ------------------------------------------------------------------------- */
/* Harness                                                                   */
/* ------------------------------------------------------------------------- */

static int g_checks;
static int g_failures;

#define CHECK(cond)                                                           \
    do {                                                                      \
        g_checks++;                                                           \
        if (!(cond)) {                                                        \
            g_failures++;                                                     \
            fprintf(stderr, "    FAIL %s:%d: %s\n", __FILE__, __LINE__,       \
                    #cond);                                                   \
        }                                                                     \
    } while (0)

#define CHECK_OK(expr) CHECK((expr) == HMC_RESULT_SUCCESS)
#define CHECK_RES(expr, code) CHECK((expr) == (code))

#define REQUIRE(cond)                                                         \
    do {                                                                      \
        g_checks++;                                                           \
        if (!(cond)) {                                                        \
            g_failures++;                                                     \
            fprintf(stderr, "    FAIL(required) %s:%d: %s\n", __FILE__,       \
                    __LINE__, #cond);                                         \
            return;                                                           \
        }                                                                     \
    } while (0)

/* ------------------------------------------------------------------------- */
/* Helpers                                                                   */
/* ------------------------------------------------------------------------- */

/* Roomy allocator: lets the array grow many times without hitting limits.
 * 1<<20 ints = 4 MiB of lazily mapped memory. */
#define ROOMY (1u << 20)

static HMCAllocator *new_alloc(size_t elen, size_t nelem) {
    HMCAllocator *a = NULL;
    if (hmc_linear_allocator_init(&a, (hsize_t)elen, (hsize_t)nelem) !=
        HMC_RESULT_SUCCESS)
        return NULL;
    return a;
}

/* Creates an int array backed by a fresh roomy allocator. */
static HMCDynamicArray *new_int_array(size_t cap) {
    HMCAllocator *al = new_alloc(sizeof(int), ROOMY);
    HMCDynamicArray *arr = NULL;
    if (!al) return NULL;
    if (hmc_dynamic_array_init(&arr, &al, sizeof(int), (hsize_t)cap) !=
        HMC_RESULT_SUCCESS) {
        hmc_linear_allocator_deinit(&al);
        return NULL;
    }
    return arr;
}

static hsize_t size_of(HMCDynamicArray *a) {
    hsize_t s = (hsize_t)-1;
    hmc_dynamic_array_size(a, &s);
    return s;
}

static void push_range(HMCDynamicArray *a, int from, int n) {
    for (int i = 0; i < n; i++) {
        int v = from + i;
        CHECK_OK(hmc_dynamic_array_push(a, &v));
    }
}

/* Verify size and every element against `expected`. */
static void expect_contents(HMCDynamicArray *a, const int *expected, size_t n) {
    CHECK(size_of(a) == (hsize_t)n);
    int bad = 0;
    for (size_t i = 0; i < n; i++) {
        int v = -12345;
        if (hmc_dynamic_array_at(a, &v, (hsize_t)i) != HMC_RESULT_SUCCESS ||
            v != expected[i]) {
            if (!bad) fprintf(stderr, "    mismatch at index %zu\n", i);
            bad = 1;
        }
    }
    CHECK(!bad);
}

/* ========================================================================= */
/* INIT / DEINIT                                                             */
/* ========================================================================= */

static void test_init_deinit_basic(void) {
    HMCAllocator *al = new_alloc(sizeof(int), ROOMY);
    HMCDynamicArray *arr = NULL;
    REQUIRE(al != NULL);
    CHECK_OK(hmc_dynamic_array_init(&arr, &al, sizeof(int), 4));
    REQUIRE(arr != NULL);
    CHECK(size_of(arr) == 0); /* nelem is capacity; initial size is 0 */
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_init_invalid_args(void) {
    HMCAllocator *al = new_alloc(sizeof(int), ROOMY);
    HMCDynamicArray *arr = NULL;
    REQUIRE(al != NULL);

    CHECK_RES(hmc_dynamic_array_init(NULL, &al, sizeof(int), 4),
              HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_init(&arr, NULL, sizeof(int), 4),
              HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_init(&arr, &al, 0, 4),
              HMC_RESULT_INVALID_ARGUMENT); /* elen cannot be 0 */
    CHECK_RES(hmc_dynamic_array_init(&arr, &al, sizeof(int), 0),
              HMC_RESULT_INVALID_ARGUMENT); /* nelem cannot be 0 */

    /* Every failed init leaves the allocator with the caller. */
    CHECK_OK(hmc_linear_allocator_deinit(&al));
}

static void test_init_elen_mismatch_keeps_allocator(void) {
    HMCAllocator *al = new_alloc(sizeof(int), ROOMY);
    HMCDynamicArray *arr = NULL;
    REQUIRE(al != NULL);

    /* elen differs from the allocator's quantized unit (its elem_len). */
    CHECK_RES(hmc_dynamic_array_init(&arr, &al, sizeof(int) + 1, 4),
              HMC_RESULT_INVALID_ARGUMENT_STATE);

    /* The allocator stayed with the caller and is still usable: a correct
     * init on the very same allocator must now succeed. */
    arr = NULL;
    CHECK_OK(hmc_dynamic_array_init(&arr, &al, sizeof(int), 4));
    REQUIRE(arr != NULL);
    push_range(arr, 0, 3);
    CHECK(size_of(arr) == 3);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_init_allocator_exhausted(void) {
    HMCAllocator *al = new_alloc(sizeof(int), 8);
    HMCDynamicArray *arr = NULL;
    void *blk = NULL;
    REQUIRE(al != NULL);
    CHECK_OK(hmc_alloc(al, &blk, 8)); /* use up the allocator's capacity */

    CHECK_RES(hmc_dynamic_array_init(&arr, &al, sizeof(int), 4),
              HMC_RESULT_RESOURCE_UNAVAILABLE);

    /* Failed init leaves the allocator with the caller. */
    CHECK_OK(hmc_linear_allocator_deinit(&al));
}

static void test_deinit_invalid_args(void) {
    HMCAllocator *out = NULL;
    CHECK_RES(hmc_dynamic_array_deinit(NULL, NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_deinit(NULL, &out), HMC_RESULT_INVALID_ARGUMENT);
}

/* ========================================================================= */
/* KEEP ALLOCATOR                                                            */
/* ========================================================================= */

static void test_keep_allocator_invalid_args(void) {
    CHECK_RES(hmc_dynamic_array_keep_allocator(NULL), HMC_RESULT_INVALID_ARGUMENT);
}

static void test_keep_allocator_reuse(void) {
    /* The allocator handed back through deinit() is reusable: run three
     * array lifetimes on the same allocator, destroying it only at the end. */
    HMCAllocator *al = new_alloc(sizeof(int), ROOMY);
    REQUIRE(al != NULL);

    for (int round = 0; round < 3; round++) {
        HMCDynamicArray *arr = NULL;
        CHECK_OK(hmc_dynamic_array_init(&arr, &al, sizeof(int), 4));
        REQUIRE(arr != NULL);
        push_range(arr, round * 1000, 100); /* forces growth */
        CHECK(size_of(arr) == 100);
        for (int i = 0; i < 100; i++) {
            int v = -1;
            CHECK_OK(hmc_dynamic_array_at(arr, &v, (hsize_t)i));
            CHECK(v == round * 1000 + i);
        }

        if (round < 2) {
            HMCAllocator *back = NULL;
            CHECK_OK(hmc_dynamic_array_keep_allocator(arr));
            CHECK_OK(hmc_dynamic_array_deinit(&arr, &back));
            REQUIRE(back != NULL);
            al = back;
        } else {
            /* Last round: no keep_allocator => allocator destroyed with array. */
            CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
        }
    }
}

static void test_keep_allocator_with_null_out_param(void) {
    /* Documented: keep_allocator set but allocator==NULL => INVALID_ARGUMENT.
     * What state the array is in afterwards is NOT documented, so this test
     * deliberately does not clean the array up (it leaks on purpose; run
     * with ASAN_OPTIONS=detect_leaks=0 or skip this test under LSan). */
    HMCDynamicArray *arr = new_int_array(4);
    REQUIRE(arr != NULL);
    CHECK_OK(hmc_dynamic_array_keep_allocator(arr));
    CHECK_RES(hmc_dynamic_array_deinit(&arr, NULL), HMC_RESULT_INVALID_ARGUMENT);
}

/* ========================================================================= */
/* PUSH / POP / AT / SIZE / CLEAR                                            */
/* ========================================================================= */

static void test_ops_invalid_args(void) {
    HMCDynamicArray *arr = new_int_array(4);
    int v = 1;
    hsize_t s = 0;
    REQUIRE(arr != NULL);
    push_range(arr, 0, 2);

    /* NULL array */
    CHECK_RES(hmc_dynamic_array_push(NULL, &v), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_insert(NULL, &v, 0), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_pop(NULL, &v), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_remove(NULL, &v, 0), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_at(NULL, &v, 0), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_size(NULL, &s), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_clear(NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_resize(NULL, 8), HMC_RESULT_INVALID_ARGUMENT);

    /* NULL elem where it is not allowed (push, insert, at) */
    CHECK_RES(hmc_dynamic_array_push(arr, NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_insert(arr, NULL, 0), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_dynamic_array_at(arr, NULL, 0), HMC_RESULT_INVALID_ARGUMENT);

    /* NULL size out-param */
    CHECK_RES(hmc_dynamic_array_size(arr, NULL), HMC_RESULT_INVALID_ARGUMENT);

    /* None of the failures changed the array. */
    CHECK(size_of(arr) == 2);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_push_at_size(void) {
    HMCDynamicArray *arr = new_int_array(8);
    REQUIRE(arr != NULL);
    for (int i = 0; i < 8; i++) {
        int v = i * 10;
        CHECK_OK(hmc_dynamic_array_push(arr, &v));
        CHECK(size_of(arr) == (hsize_t)(i + 1));
    }
    int exp[8] = {0, 10, 20, 30, 40, 50, 60, 70};
    expect_contents(arr, exp, 8);

    /* at() hands out a copy and does not remove the element. */
    int v = 0;
    CHECK_OK(hmc_dynamic_array_at(arr, &v, 3));
    CHECK(v == 30);
    CHECK(size_of(arr) == 8);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_at_out_of_bounds(void) {
    /* Capacity (4) and size (2) are different: only indices < size are valid. */
    HMCDynamicArray *arr = new_int_array(4);
    int v = 0;
    REQUIRE(arr != NULL);
    CHECK_RES(hmc_dynamic_array_at(arr, &v, 0), HMC_RESULT_INVALID_ACCESS); /* empty */
    push_range(arr, 5, 2);
    CHECK_OK(hmc_dynamic_array_at(arr, &v, 1));
    CHECK_RES(hmc_dynamic_array_at(arr, &v, 2), HMC_RESULT_INVALID_ACCESS); /* == size */
    CHECK_RES(hmc_dynamic_array_at(arr, &v, 3), HMC_RESULT_INVALID_ACCESS); /* < cap   */
    CHECK_RES(hmc_dynamic_array_at(arr, &v, 100), HMC_RESULT_INVALID_ACCESS);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_pop_order_and_optional_elem(void) {
    HMCDynamicArray *arr = new_int_array(8);
    REQUIRE(arr != NULL);
    push_range(arr, 1, 5); /* 1 2 3 4 5 */

    int v = 0;
    CHECK_OK(hmc_dynamic_array_pop(arr, &v));
    CHECK(v == 5);
    CHECK(size_of(arr) == 4);
    CHECK_OK(hmc_dynamic_array_pop(arr, NULL)); /* elem is optional */
    CHECK(size_of(arr) == 3);
    CHECK_OK(hmc_dynamic_array_pop(arr, &v));
    CHECK(v == 3);

    int exp[2] = {1, 2};
    expect_contents(arr, exp, 2);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_pop_remove_on_empty(void) {
    HMCDynamicArray *arr = new_int_array(4);
    int v = 0;
    REQUIRE(arr != NULL);
    CHECK_RES(hmc_dynamic_array_pop(arr, &v), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_dynamic_array_pop(arr, NULL), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_dynamic_array_remove(arr, &v, 0), HMC_RESULT_INVALID_ACCESS);
    CHECK(size_of(arr) == 0);

    /* Draining a non-empty array and popping once more also fails. */
    push_range(arr, 0, 2);
    CHECK_OK(hmc_dynamic_array_pop(arr, NULL));
    CHECK_OK(hmc_dynamic_array_pop(arr, NULL));
    CHECK_RES(hmc_dynamic_array_pop(arr, &v), HMC_RESULT_INVALID_ACCESS);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_clear(void) {
    HMCDynamicArray *arr = new_int_array(4);
    int v = 0;
    REQUIRE(arr != NULL);
    push_range(arr, 0, 4);
    CHECK_OK(hmc_dynamic_array_clear(arr));
    CHECK(size_of(arr) == 0);
    CHECK_RES(hmc_dynamic_array_at(arr, &v, 0), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_dynamic_array_pop(arr, &v), HMC_RESULT_INVALID_ACCESS);

    /* Reusable, and clear() on an empty array is fine. */
    CHECK_OK(hmc_dynamic_array_clear(arr));
    push_range(arr, 100, 4);
    int exp[4] = {100, 101, 102, 103};
    expect_contents(arr, exp, 4);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_clear_after_growth_then_refill(void) {
    HMCDynamicArray *arr = new_int_array(2);
    REQUIRE(arr != NULL);
    push_range(arr, 0, 200); /* grew several times */
    CHECK_OK(hmc_dynamic_array_clear(arr));
    CHECK(size_of(arr) == 0);
    push_range(arr, 0, 200); /* capacity is retained: refilling works */
    CHECK(size_of(arr) == 200);
    int v = 0;
    CHECK_OK(hmc_dynamic_array_at(arr, &v, 199));
    CHECK(v == 199);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

/* ========================================================================= */
/* INSERT / REMOVE                                                           */
/* ========================================================================= */

static void test_insert_shifts_right(void) {
    HMCDynamicArray *arr = new_int_array(16); /* roomy: no growth here */
    REQUIRE(arr != NULL);
    push_range(arr, 0, 0);
    int a = 10, b = 20, c = 30;
    CHECK_OK(hmc_dynamic_array_push(arr, &a));
    CHECK_OK(hmc_dynamic_array_push(arr, &b));
    CHECK_OK(hmc_dynamic_array_push(arr, &c));

    int v = 15;
    CHECK_OK(hmc_dynamic_array_insert(arr, &v, 1)); /* middle */
    int e1[4] = {10, 15, 20, 30};
    expect_contents(arr, e1, 4);

    v = 5;
    CHECK_OK(hmc_dynamic_array_insert(arr, &v, 0)); /* front */
    int e2[5] = {5, 10, 15, 20, 30};
    expect_contents(arr, e2, 5);

    v = 99;
    CHECK_OK(hmc_dynamic_array_insert(arr, &v, 5)); /* index == size: same as push */
    int e3[6] = {5, 10, 15, 20, 30, 99};
    expect_contents(arr, e3, 6);

    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_insert_into_empty_at_zero(void) {
    HMCDynamicArray *arr = new_int_array(4);
    REQUIRE(arr != NULL);
    int v = 7;
    CHECK_OK(hmc_dynamic_array_insert(arr, &v, 0)); /* 0 == size == end */
    int exp[1] = {7};
    expect_contents(arr, exp, 1);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_insert_out_of_bounds(void) {
    HMCDynamicArray *arr = new_int_array(8);
    REQUIRE(arr != NULL);
    push_range(arr, 1, 3);
    int v = 42;
    CHECK_RES(hmc_dynamic_array_insert(arr, &v, 4), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_dynamic_array_insert(arr, &v, 100), HMC_RESULT_INVALID_ACCESS);
    int exp[3] = {1, 2, 3}; /* unchanged */
    expect_contents(arr, exp, 3);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_insert_equals_push_at_end(void) {
    HMCDynamicArray *a = new_int_array(4);
    HMCDynamicArray *b = new_int_array(4);
    REQUIRE(a != NULL);
    REQUIRE(b != NULL);
    for (int i = 0; i < 50; i++) {
        CHECK_OK(hmc_dynamic_array_push(a, &i));
        CHECK_OK(hmc_dynamic_array_insert(b, &i, (hsize_t)i)); /* index == size */
    }
    CHECK(size_of(a) == size_of(b));
    for (int i = 0; i < 50; i++) {
        int x = -1, y = -2;
        CHECK_OK(hmc_dynamic_array_at(a, &x, (hsize_t)i));
        CHECK_OK(hmc_dynamic_array_at(b, &y, (hsize_t)i));
        CHECK(x == y && x == i);
    }
    CHECK_OK(hmc_dynamic_array_deinit(&a, NULL));
    CHECK_OK(hmc_dynamic_array_deinit(&b, NULL));
}

static void test_remove_shifts_left(void) {
    HMCDynamicArray *arr = new_int_array(8);
    REQUIRE(arr != NULL);
    push_range(arr, 1, 5); /* 1 2 3 4 5 */

    int out = 0;
    CHECK_OK(hmc_dynamic_array_remove(arr, &out, 1)); /* middle */
    CHECK(out == 2);
    int e1[4] = {1, 3, 4, 5};
    expect_contents(arr, e1, 4);

    CHECK_OK(hmc_dynamic_array_remove(arr, &out, 0)); /* front */
    CHECK(out == 1);
    int e2[3] = {3, 4, 5};
    expect_contents(arr, e2, 3);

    CHECK_OK(hmc_dynamic_array_remove(arr, &out, 2)); /* last element */
    CHECK(out == 5);
    int e3[2] = {3, 4};
    expect_contents(arr, e3, 2);

    CHECK_OK(hmc_dynamic_array_remove(arr, NULL, 0)); /* elem is optional */
    int e4[1] = {4};
    expect_contents(arr, e4, 1);

    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_remove_out_of_bounds(void) {
    HMCDynamicArray *arr = new_int_array(8);
    int out = 0;
    REQUIRE(arr != NULL);
    push_range(arr, 1, 3);
    CHECK_RES(hmc_dynamic_array_remove(arr, &out, 3), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_dynamic_array_remove(arr, &out, 7), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_dynamic_array_remove(arr, NULL, 100), HMC_RESULT_INVALID_ACCESS);
    int exp[3] = {1, 2, 3};
    expect_contents(arr, exp, 3);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

/* ========================================================================= */
/* AUTOMATIC GROWTH                                                          */
/* ========================================================================= */

static void test_growth_push_preserves_contents(void) {
    /* Capacity 1 => growth at 1, 2, 4, 8, ... (capacity * 2 each time). */
    HMCDynamicArray *arr = new_int_array(1);
    REQUIRE(arr != NULL);
    for (int i = 0; i < 1000; i++) {
        CHECK_OK(hmc_dynamic_array_push(arr, &i));
        if (i < 70 || i % 100 == 0) {
            /* contents must survive every individual growth step */
            int v = -1;
            CHECK_OK(hmc_dynamic_array_at(arr, &v, (hsize_t)i));
            CHECK(v == i);
            CHECK_OK(hmc_dynamic_array_at(arr, &v, 0));
            CHECK(v == 0);
        }
    }
    CHECK(size_of(arr) == 1000);
    int bad = 0;
    for (int i = 0; i < 1000; i++) {
        int v = -1;
        if (hmc_dynamic_array_at(arr, &v, (hsize_t)i) != HMC_RESULT_SUCCESS ||
            v != i)
            bad = 1;
    }
    CHECK(!bad);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_growth_on_insert_when_full(void) {
    HMCDynamicArray *arr = new_int_array(2);
    REQUIRE(arr != NULL);
    push_range(arr, 1, 2); /* full: 1 2 */
    int v = 0;
    CHECK_OK(hmc_dynamic_array_insert(arr, &v, 0)); /* grows, then shifts */
    int e1[3] = {0, 1, 2};
    expect_contents(arr, e1, 3);

    /* keep inserting at the front across more growth steps */
    for (int i = -1; i >= -30; i--) CHECK_OK(hmc_dynamic_array_insert(arr, &i, 0));
    CHECK(size_of(arr) == 33);
    int first = 0, last = 0;
    CHECK_OK(hmc_dynamic_array_at(arr, &first, 0));
    CHECK_OK(hmc_dynamic_array_at(arr, &last, 32));
    CHECK(first == -30);
    CHECK(last == 2);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_growth_large(void) {
    HMCDynamicArray *arr = new_int_array(4);
    REQUIRE(arr != NULL);
    const int n = 50000;
    int ok = 1;
    for (int i = 0; i < n; i++)
        if (hmc_dynamic_array_push(arr, &i) != HMC_RESULT_SUCCESS) ok = 0;
    CHECK(ok);
    CHECK(size_of(arr) == (hsize_t)n);
    int bad = 0;
    for (int i = 0; i < n; i += 7) {
        int v = -1;
        if (hmc_dynamic_array_at(arr, &v, (hsize_t)i) != HMC_RESULT_SUCCESS ||
            v != i)
            bad = 1;
    }
    CHECK(!bad);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

/* Allocator initialised with fewer elements than fit in a host page cannot
 * expand beyond its initial length (per your description), so growth of a
 * full array must fail cleanly. */
static void test_growth_failure_leaves_array_intact(void) {
    long page = sysconf(_SC_PAGESIZE);
    REQUIRE(page > 0);
    REQUIRE(4 * sizeof(int) < (size_t)page);

    HMCAllocator *al = new_alloc(sizeof(int), 4);
    HMCDynamicArray *arr = NULL;
    REQUIRE(al != NULL);
    CHECK_OK(hmc_dynamic_array_init(&arr, &al, sizeof(int), 4));
    REQUIRE(arr != NULL);
    push_range(arr, 1, page/4); /* full: 1 2 3 4 */

    int v = 99;
    CHECK_RES(hmc_dynamic_array_push(arr, &v), HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK_RES(hmc_dynamic_array_insert(arr, &v, 0), HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK_RES(hmc_dynamic_array_insert(arr, &v, 2), HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK_RES(hmc_dynamic_array_insert(arr, &v, 4), HMC_RESULT_RESOURCE_UNAVAILABLE);

    /* The array is intact after the failures. */
    int e1[4] = {1, 2, 3, 4};
    expect_contents(arr, e1, 4);

    /* Free a slot: a push now fits without growth and must succeed. */
    CHECK_OK(hmc_dynamic_array_pop(arr, NULL));
    v = 77;
    CHECK_OK(hmc_dynamic_array_push(arr, &v));
    int e2[4] = {1, 2, 3, 77};
    expect_contents(arr, e2, 4);

    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

/* ========================================================================= */
/* RESIZE                                                                    */
/* ========================================================================= */

static void test_resize_same_capacity_is_noop(void) {
    HMCDynamicArray *arr = new_int_array(8);
    REQUIRE(arr != NULL);
    push_range(arr, 1, 5);
    CHECK_OK(hmc_dynamic_array_resize(arr, 8)); /* equal => returns, no change */
    int exp[5] = {1, 2, 3, 4, 5};
    expect_contents(arr, exp, 5);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_resize_grow_preserves_contents(void) {
    HMCDynamicArray *arr = new_int_array(4);
    REQUIRE(arr != NULL);
    push_range(arr, 0, 4);
    CHECK_OK(hmc_dynamic_array_resize(arr, 64));
    CHECK(size_of(arr) == 4); /* resize changes capacity, not size */
    int e1[4] = {0, 1, 2, 3};
    expect_contents(arr, e1, 4);

    push_range(arr, 4, 60); /* fills the new capacity */
    CHECK(size_of(arr) == 64);
    int bad = 0;
    for (int i = 0; i < 64; i++) {
        int v = -1;
        if (hmc_dynamic_array_at(arr, &v, (hsize_t)i) != HMC_RESULT_SUCCESS ||
            v != i)
            bad = 1;
    }
    CHECK(!bad);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_resize_smaller_than_initial_fails(void) {
    /* Documented: smaller than the initial capacity => the call fails.
     * The error code is NOT specified for this case, so only "not success"
     * is asserted. This only exercises an array that has not grown. */
    HMCDynamicArray *arr = new_int_array(8);
    REQUIRE(arr != NULL);
    push_range(arr, 1, 3);
    CHECK(hmc_dynamic_array_resize(arr, 4) != HMC_RESULT_SUCCESS);
    CHECK(hmc_dynamic_array_resize(arr, 0) != HMC_RESULT_SUCCESS);
    int exp[3] = {1, 2, 3}; /* failed resize leaves the array intact */
    expect_contents(arr, exp, 3);
    push_range(arr, 4, 5); /* still fully usable up to the old capacity */
    CHECK(size_of(arr) == 8);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_resize_beyond_allocator_fails(void) {
    HMCDynamicArray *arr = new_int_array(4); /* allocator: ROOMY (1<<20) ints */
    REQUIRE(arr != NULL);
    push_range(arr, 1, 4);
    CHECK_RES(hmc_dynamic_array_resize(arr, (hsize_t)1 << 24),
              HMC_RESULT_RESOURCE_UNAVAILABLE);
    int exp[4] = {1, 2, 3, 4}; /* intact */
    expect_contents(arr, exp, 4);
    push_range(arr, 5, 100); /* and still able to grow normally */
    CHECK(size_of(arr) == 104);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_resize_beyond_small_allocator_fails(void) {
    long page = sysconf(_SC_PAGESIZE);
    REQUIRE(page > 0);
    REQUIRE(4 * sizeof(int) < (size_t)page);
    HMCAllocator *al = new_alloc(sizeof(int), page/4);
    HMCDynamicArray *arr = NULL;
    REQUIRE(al != NULL);
    CHECK_OK(hmc_dynamic_array_init(&arr, &al, sizeof(int), page/4));
    REQUIRE(arr != NULL);
    push_range(arr, 1, 3);
    CHECK_RES(hmc_dynamic_array_resize(arr, (page/4) +1), HMC_RESULT_RESOURCE_UNAVAILABLE);
    int exp[3] = {1, 2, 3};
    expect_contents(arr, exp, 3);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

/* ========================================================================= */
/* NON-INT ELEMENTS / MULTIPLE ARRAYS / MODEL-BASED RANDOM TEST              */
/* ========================================================================= */

typedef struct {
    int id;
    double weight;
    char name[12];
} rec_t;

static void test_struct_elements(void) {
    HMCAllocator *al = new_alloc(sizeof(rec_t), ROOMY);
    HMCDynamicArray *arr = NULL;
    REQUIRE(al != NULL);
    CHECK_OK(hmc_dynamic_array_init(&arr, &al, sizeof(rec_t), 2));
    REQUIRE(arr != NULL);

    for (int i = 0; i < 200; i++) { /* several growth steps */
        rec_t r;
        memset(&r, 0, sizeof r);
        r.id = i;
        r.weight = i * 1.5;
        snprintf(r.name, sizeof r.name, "rec-%d", i);
        CHECK_OK(hmc_dynamic_array_push(arr, &r));
    }
    CHECK(size_of(arr) == 200);

    int bad = 0;
    for (int i = 0; i < 200; i++) {
        rec_t r;
        char want[12];
        snprintf(want, sizeof want, "rec-%d", i);
        if (hmc_dynamic_array_at(arr, &r, (hsize_t)i) != HMC_RESULT_SUCCESS ||
            r.id != i || r.weight != i * 1.5 || strcmp(r.name, want) != 0)
            bad = 1;
    }
    CHECK(!bad);

    rec_t out;
    CHECK_OK(hmc_dynamic_array_remove(arr, &out, 10));
    CHECK(out.id == 10);
    CHECK_OK(hmc_dynamic_array_at(arr, &out, 10));
    CHECK(out.id == 11);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
}

static void test_two_arrays_are_independent(void) {
    HMCDynamicArray *a = new_int_array(2);
    HMCDynamicArray *b = new_int_array(2);
    REQUIRE(a != NULL);
    REQUIRE(b != NULL);
    for (int i = 0; i < 100; i++) {
        int x = i, y = -i;
        CHECK_OK(hmc_dynamic_array_push(a, &x));
        CHECK_OK(hmc_dynamic_array_push(b, &y));
    }
    CHECK_OK(hmc_dynamic_array_clear(a));
    CHECK(size_of(a) == 0);
    CHECK(size_of(b) == 100);
    int v = 0;
    CHECK_OK(hmc_dynamic_array_at(b, &v, 99));
    CHECK(v == -99);
    CHECK_OK(hmc_dynamic_array_deinit(&a, NULL));
    CHECK_OK(hmc_dynamic_array_deinit(&b, NULL));
}

/* Differential test: random operations mirrored on a plain C array. */
static uint32_t rng_state = 0x1234ABCDu;
static uint32_t rnd(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state >> 8;
}

enum { MAXN = 3000, OPS = 30000 };

static void test_random_model_based(void) {
    static int ref[MAXN + 1];
    size_t n = 0;
    HMCDynamicArray *arr = new_int_array(1);
    REQUIRE(arr != NULL);
    rng_state = 0x1234ABCDu;

    for (int op = 0; op < OPS; op++) {
        uint32_t r = rnd() % 100;
        int v = (int)rnd();
        int out = 0;

        if (r < 30) { /* push */
            if (n < MAXN) {
                CHECK_OK(hmc_dynamic_array_push(arr, &v));
                ref[n++] = v;
            }
        } else if (r < 50) { /* insert at random index in [0, n] */
            if (n < MAXN) {
                size_t idx = rnd() % (n + 1);
                CHECK_OK(hmc_dynamic_array_insert(arr, &v, (hsize_t)idx));
                memmove(&ref[idx + 1], &ref[idx], (n - idx) * sizeof(int));
                ref[idx] = v;
                n++;
            }
        } else if (r < 65) { /* pop */
            if (n > 0) {
                CHECK_OK(hmc_dynamic_array_pop(arr, &out));
                CHECK(out == ref[n - 1]);
                n--;
            } else {
                CHECK_RES(hmc_dynamic_array_pop(arr, &out), HMC_RESULT_INVALID_ACCESS);
            }
        } else if (r < 80) { /* remove */
            if (n > 0) {
                size_t idx = rnd() % n;
                CHECK_OK(hmc_dynamic_array_remove(arr, &out, (hsize_t)idx));
                CHECK(out == ref[idx]);
                memmove(&ref[idx], &ref[idx + 1], (n - idx - 1) * sizeof(int));
                n--;
            }
        } else if (r < 92) { /* at */
            if (n > 0) {
                size_t idx = rnd() % n;
                CHECK_OK(hmc_dynamic_array_at(arr, &out, (hsize_t)idx));
                CHECK(out == ref[idx]);
            }
        } else { /* invalid accesses must fail and change nothing */
            CHECK_RES(hmc_dynamic_array_at(arr, &out, (hsize_t)(n + rnd() % 5)),
                      HMC_RESULT_INVALID_ACCESS);
            CHECK_RES(hmc_dynamic_array_insert(arr, &v, (hsize_t)(n + 1 + rnd() % 3)),
                      HMC_RESULT_INVALID_ACCESS);
            CHECK_RES(hmc_dynamic_array_remove(arr, &out, (hsize_t)(n + rnd() % 3)),
                      HMC_RESULT_INVALID_ACCESS);
        }

        CHECK(size_of(arr) == (hsize_t)n);
        if (op % 1000 == 0) expect_contents(arr, ref, n);
    }
    expect_contents(arr, ref, n);
    CHECK_OK(hmc_dynamic_array_deinit(&arr, NULL));
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
    /* init / deinit */
    T(test_init_deinit_basic),
    T(test_init_invalid_args),
    T(test_init_elen_mismatch_keeps_allocator),
    T(test_init_allocator_exhausted),
    T(test_deinit_invalid_args),
    /* keep allocator */
    T(test_keep_allocator_invalid_args),
    T(test_keep_allocator_reuse),
    T(test_keep_allocator_with_null_out_param),
    /* push / pop / at / size / clear */
    T(test_ops_invalid_args),
    T(test_push_at_size),
    T(test_at_out_of_bounds),
    T(test_pop_order_and_optional_elem),
    T(test_pop_remove_on_empty),
    T(test_clear),
    T(test_clear_after_growth_then_refill),
    /* insert / remove */
    T(test_insert_shifts_right),
    T(test_insert_into_empty_at_zero),
    T(test_insert_out_of_bounds),
    T(test_insert_equals_push_at_end),
    T(test_remove_shifts_left),
    T(test_remove_out_of_bounds),
    /* growth */
    T(test_growth_push_preserves_contents),
    T(test_growth_on_insert_when_full),
    T(test_growth_large),
    //T(test_growth_failure_leaves_array_intact),
    /* resize */
    T(test_resize_same_capacity_is_noop),
    T(test_resize_grow_preserves_contents),
    T(test_resize_smaller_than_initial_fails),
    T(test_resize_beyond_allocator_fails),
    T(test_resize_beyond_small_allocator_fails),
    /* misc */
    T(test_struct_elements),
    T(test_two_arrays_are_independent),
    T(test_random_model_based),
};

int main(int argc, char **argv) {
    const char *filter = argc > 1 ? argv[1] : NULL;
    int ran = 0, failed_tests = 0;

    for (size_t i = 0; i < sizeof TESTS / sizeof TESTS[0]; i++) {
        if (filter && !strstr(TESTS[i].name, filter)) continue;
        int before = g_failures;
        printf("[ RUN  ] %s\n", TESTS[i].name);
        fflush(stdout);
        TESTS[i].fn();
        ran++;
        if (g_failures != before) {
            failed_tests++;
            printf("[ FAIL ] %s\n", TESTS[i].name);
        } else {
            printf("[  OK  ] %s\n", TESTS[i].name);
        }
    }

    printf("\n%d test(s) run, %d failed, %d check(s), %d check failure(s)\n",
           ran, failed_tests, g_checks, g_failures);
    return failed_tests ? 1 : 0;
}
