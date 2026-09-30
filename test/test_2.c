#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hmc_allocator.h>
#include <hmc_linear_allocator.h>
#include <hmc_static_array.h>
#include <hmc_results.h>

/* ------------------------------------------------------------------ */
/* Mini harness                                                        */
/* ------------------------------------------------------------------ */

static int g_checks = 0;
static int g_failed = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        g_checks++;                                                          \
        if (!(cond)) {                                                       \
            g_failed++;                                                      \
            fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                    \
    } while (0)

#define CHECK_OK(expr)      CHECK((expr) == HMC_RESULT_SUCCESS)
#define CHECK_RES(expr, r)  CHECK((expr) == (r))
#define CHECK_ERR(expr)     CHECK((expr) != HMC_RESULT_SUCCESS)

#define RUN(fn)                                                              \
    do {                                                                     \
        int before = g_failed;                                               \
        fn();                                                                \
        printf("[%s] %s\n", g_failed == before ? " OK " : "FAIL", #fn);      \
    } while (0)

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

/* Builds a linear allocator sized for (elen, cap) and an array on top of it. */
static hmcResult_t make_array(HMCStaticArray **arr, hsize_t elen, hsize_t cap)
{
    HMCAllocator *a = NULL;
    hmcResult_t r = hmc_linear_allocator_init(&a, elen, cap);
    if (r != HMC_RESULT_SUCCESS)
        return r;
    r = hmc_static_array_init(arr, &a, elen, cap);
    /* ASSUMPTION: on failed init the caller still owns the allocator. */
    if (r != HMC_RESULT_SUCCESS && a != NULL)
        hmc_linear_allocator_deinit(&a);
    return r;
}

static void destroy_array(HMCStaticArray **arr)
{
    CHECK_OK(hmc_static_array_deinit(arr, NULL));
}

static hsize_t get_size(HMCStaticArray *arr)
{
    hsize_t s = (hsize_t)-1;
    CHECK_OK(hmc_static_array_size(arr, &s));
    return s;
}

static void push_range(HMCStaticArray *arr, int from, int to)
{
    for (int i = from; i < to; i++) {
        int v = i;
        CHECK_OK(hmc_static_array_push(arr, &v));
    }
}

static void expect_contents(HMCStaticArray *arr, const int *exp, hsize_t n)
{
    CHECK(get_size(arr) == n);
    for (hsize_t i = 0; i < n; i++) {
        int v = -12345;
        CHECK_OK(hmc_static_array_at(arr, &v, i));
        if (v != exp[i]) {
            fprintf(stderr, "    index %llu: got %d, expected %d\n",
                    (unsigned long long)i, v, exp[i]);
        }
        CHECK(v == exp[i]);
    }
}

/* ------------------------------------------------------------------ */
/* init / deinit                                                       */
/* ------------------------------------------------------------------ */

static void test_init_deinit_basic(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 8));
    CHECK(arr != NULL);
    CHECK(get_size(arr) == 0);
    destroy_array(&arr);
}

static void test_init_invalid_arguments(void)
{
    HMCAllocator *a = NULL;
    HMCStaticArray *arr = NULL;

    CHECK_OK(hmc_linear_allocator_init(&a, sizeof(int), 8));

    /* NULL array pointer */
    CHECK_RES(hmc_static_array_init(NULL, &a, sizeof(int), 8),
              HMC_RESULT_INVALID_ARGUMENT);

    /* NULL allocator pointer */
    CHECK_RES(hmc_static_array_init(&arr, NULL, sizeof(int), 8),
              HMC_RESULT_INVALID_ARGUMENT);

    /* elen == 0 is documented as invalid */
    CHECK_RES(hmc_static_array_init(&arr, &a, 0, 8),
              HMC_RESULT_INVALID_ARGUMENT);

    /* None of the failures above should have consumed the allocator. */
    CHECK(a != NULL);

    /* Pointer to a NULL allocator */
    HMCAllocator *null_alloc = NULL;
    CHECK_ERR(hmc_static_array_init(&arr, &null_alloc, sizeof(int), 8));

    CHECK_OK(hmc_linear_allocator_deinit(&a));
}

static void test_init_elen_mismatch_with_quantized_allocator(void)
{
    /* Linear allocator quantizes on elem_len; a different elen must fail. */
    HMCAllocator *a = NULL;
    HMCStaticArray *arr = NULL;

    CHECK_OK(hmc_linear_allocator_init(&a, sizeof(int), 8));
    CHECK_RES(hmc_static_array_init(&arr, &a, sizeof(int) * 2, 8),
              HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK_RES(hmc_static_array_init(&arr, &a, sizeof(char), 8),
              HMC_RESULT_INVALID_ARGUMENT_STATE);
    CHECK_OK(hmc_linear_allocator_deinit(&a));
}

static void test_init_more_elements_than_allocator_holds(void)
{
    /* Far more than a handful of mapped pages can provide. */
    HMCAllocator *a = NULL;
    HMCStaticArray *arr = NULL;

    CHECK_OK(hmc_linear_allocator_init(&a, sizeof(int), 4));
    CHECK_RES(hmc_static_array_init(&arr, &a, sizeof(int), (hsize_t)1 << 28),
              HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK_OK(hmc_linear_allocator_deinit(&a));
}

static void test_deinit_invalid_arguments(void)
{
    CHECK_RES(hmc_static_array_deinit(NULL, NULL), HMC_RESULT_INVALID_ARGUMENT);

    /* keep_allocator marked but no place to hand the allocator back */
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    CHECK_OK(hmc_static_array_keep_allocator(arr));
    CHECK_RES(hmc_static_array_deinit(&arr, NULL), HMC_RESULT_INVALID_ARGUMENT);

    /* The failed deinit must leave the array usable, so retry properly. */
    HMCAllocator *a = NULL;
    CHECK_OK(hmc_static_array_deinit(&arr, &a));
    CHECK(a != NULL);
    CHECK_OK(hmc_linear_allocator_deinit(&a));
}

static void test_init_deinit_many_cycles(void)
{
    /* Leak / double-free detector when run under ASan or valgrind. */
    for (int i = 0; i < 500; i++) {
        HMCStaticArray *arr = NULL;
        CHECK_OK(make_array(&arr, sizeof(int), 16));
        push_range(arr, 0, 16);
        destroy_array(&arr);
    }
}

/* ------------------------------------------------------------------ */
/* push / pop                                                          */
/* ------------------------------------------------------------------ */

static void test_push_appends_in_order(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 8));

    push_range(arr, 10, 15);
    const int exp[] = {10, 11, 12, 13, 14};
    expect_contents(arr, exp, 5);

    destroy_array(&arr);
}

static void test_push_until_full(void)
{
    enum { CAP = 6 };
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), CAP));

    push_range(arr, 0, CAP);
    CHECK(get_size(arr) == CAP);

    int extra = 99;
    CHECK_RES(hmc_static_array_push(arr, &extra), HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK(get_size(arr) == CAP);

    const int exp[CAP] = {0, 1, 2, 3, 4, 5};
    expect_contents(arr, exp, CAP);

    destroy_array(&arr);
}

static void test_push_invalid_arguments(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    int v = 1;

    CHECK_RES(hmc_static_array_push(NULL, &v), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_static_array_push(arr, NULL), HMC_RESULT_INVALID_ARGUMENT);
    CHECK(get_size(arr) == 0);

    destroy_array(&arr);
}

static void test_pop_returns_elements_lifo(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 8));
    push_range(arr, 1, 6); /* 1 2 3 4 5 */

    for (int expected = 5; expected >= 1; expected--) {
        int v = -1;
        CHECK_OK(hmc_static_array_pop(arr, &v));
        CHECK(v == expected);
        CHECK(get_size(arr) == (hsize_t)(expected - 1));
    }

    destroy_array(&arr);
}

static void test_pop_with_null_elem_discards(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    push_range(arr, 0, 3);

    CHECK_OK(hmc_static_array_pop(arr, NULL));
    const int exp[] = {0, 1};
    expect_contents(arr, exp, 2);

    destroy_array(&arr);
}

static void test_pop_on_empty_fails(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));

    int v = 7;
    /* ASSUMPTION: docs list several possible codes; any failure is accepted. */
    CHECK_ERR(hmc_static_array_pop(arr, &v));
    CHECK_ERR(hmc_static_array_pop(arr, NULL));
    CHECK(get_size(arr) == 0);

    /* Array must still be usable afterwards. */
    push_range(arr, 0, 2);
    CHECK(get_size(arr) == 2);

    destroy_array(&arr);
}

static void test_pop_null_array(void)
{
    int v;
    CHECK_RES(hmc_static_array_pop(NULL, &v), HMC_RESULT_INVALID_ARGUMENT);
}

static void test_push_pop_interleaved(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));

    for (int round = 0; round < 100; round++) {
        int a = round, b = round + 1000, out = -1;
        CHECK_OK(hmc_static_array_push(arr, &a));
        CHECK_OK(hmc_static_array_push(arr, &b));
        CHECK_OK(hmc_static_array_pop(arr, &out));
        CHECK(out == b);
        CHECK_OK(hmc_static_array_pop(arr, &out));
        CHECK(out == a);
        CHECK(get_size(arr) == 0);
    }

    destroy_array(&arr);
}

/* ------------------------------------------------------------------ */
/* insert                                                              */
/* ------------------------------------------------------------------ */

static void test_insert_at_front_middle_and_shifts(void)
{
    /*
     * ASSUMPTION: insert is order-preserving (elements at/after 'index'
     * shift towards the end). The doc text for push/insert is swapped in
     * one sentence; this treats insert as "arbitrary position" and push
     * as "append", which matches the function names.
     */
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 10));
    push_range(arr, 1, 4); /* 1 2 3 */

    int v = 100;
    CHECK_OK(hmc_static_array_insert(arr, &v, 0)); /* 100 1 2 3 */
    v = 200;
    CHECK_OK(hmc_static_array_insert(arr, &v, 2)); /* 100 1 200 2 3 */

    const int exp[] = {100, 1, 200, 2, 3};
    expect_contents(arr, exp, 5);

    destroy_array(&arr);
}

static void test_insert_into_empty_at_zero(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));

    int v = 42;
    CHECK_OK(hmc_static_array_insert(arr, &v, 0));
    const int exp[] = {42};
    expect_contents(arr, exp, 1);

    destroy_array(&arr);
}

static void test_insert_into_full_array(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 3));
    push_range(arr, 0, 3);

    int v = 9;
    CHECK_RES(hmc_static_array_insert(arr, &v, 1), HMC_RESULT_RESOURCE_UNAVAILABLE);

    const int exp[] = {0, 1, 2};
    expect_contents(arr, exp, 3);

    destroy_array(&arr);
}

static void test_insert_out_of_bounds(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 8));
    push_range(arr, 0, 3);

    int v = 9;
    CHECK_RES(hmc_static_array_insert(arr, &v, 1000), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_static_array_insert(arr, &v, (hsize_t)-1), HMC_RESULT_INVALID_ACCESS);
    /* Strictly past the end (one gap) must not be accepted either. */
    CHECK_ERR(hmc_static_array_insert(arr, &v, 5));

    const int exp[] = {0, 1, 2};
    expect_contents(arr, exp, 3);

    destroy_array(&arr);
}

static void test_insert_invalid_arguments(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    int v = 1;

    CHECK_RES(hmc_static_array_insert(NULL, &v, 0), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_static_array_insert(arr, NULL, 0), HMC_RESULT_INVALID_ARGUMENT);
    CHECK(get_size(arr) == 0);

    destroy_array(&arr);
}

/* ------------------------------------------------------------------ */
/* remove                                                              */
/* ------------------------------------------------------------------ */

static void test_remove_first_middle_last(void)
{
    /* ASSUMPTION: remove is order-preserving (later elements shift down). */
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 8));
    push_range(arr, 0, 6); /* 0 1 2 3 4 5 */

    int out = -1;
    CHECK_OK(hmc_static_array_remove(arr, &out, 0)); /* -> 1 2 3 4 5 */
    CHECK(out == 0);
    CHECK_OK(hmc_static_array_remove(arr, &out, 2)); /* -> 1 2 4 5 */
    CHECK(out == 3);
    CHECK_OK(hmc_static_array_remove(arr, &out, 3)); /* -> 1 2 4 */
    CHECK(out == 5);

    const int exp[] = {1, 2, 4};
    expect_contents(arr, exp, 3);

    destroy_array(&arr);
}

static void test_remove_with_null_elem(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    push_range(arr, 0, 4);

    CHECK_OK(hmc_static_array_remove(arr, NULL, 1));
    const int exp[] = {0, 2, 3};
    expect_contents(arr, exp, 3);

    destroy_array(&arr);
}

static void test_remove_out_of_bounds(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    push_range(arr, 0, 2);

    int out = 555;
    CHECK_RES(hmc_static_array_remove(arr, &out, 2), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_static_array_remove(arr, &out, 1000), HMC_RESULT_INVALID_ACCESS);
    CHECK(get_size(arr) == 2);

    /* Empty array */
    CHECK_OK(hmc_static_array_clear(arr));
    CHECK_RES(hmc_static_array_remove(arr, &out, 0), HMC_RESULT_INVALID_ACCESS);

    destroy_array(&arr);
}

static void test_remove_null_array(void)
{
    int out;
    CHECK_RES(hmc_static_array_remove(NULL, &out, 0), HMC_RESULT_INVALID_ARGUMENT);
}

static void test_remove_until_empty_then_refill(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 5));
    push_range(arr, 0, 5);

    for (int i = 0; i < 5; i++)
        CHECK_OK(hmc_static_array_remove(arr, NULL, 0));
    CHECK(get_size(arr) == 0);

    push_range(arr, 50, 55);
    const int exp[] = {50, 51, 52, 53, 54};
    expect_contents(arr, exp, 5);

    destroy_array(&arr);
}

/* ------------------------------------------------------------------ */
/* at                                                                  */
/* ------------------------------------------------------------------ */

static void test_at_returns_copy_and_does_not_remove(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    push_range(arr, 7, 10); /* 7 8 9 */

    int v = 0;
    CHECK_OK(hmc_static_array_at(arr, &v, 1));
    CHECK(v == 8);
    CHECK(get_size(arr) == 3);

    /* Mutating the copy must not touch the stored element. */
    v = 12345;
    int again = 0;
    CHECK_OK(hmc_static_array_at(arr, &again, 1));
    CHECK(again == 8);

    destroy_array(&arr);
}

static void test_at_out_of_bounds(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    push_range(arr, 0, 2);

    int v = 31337;
    CHECK_RES(hmc_static_array_at(arr, &v, 2), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_static_array_at(arr, &v, 4), HMC_RESULT_INVALID_ACCESS);
    CHECK_RES(hmc_static_array_at(arr, &v, (hsize_t)-1), HMC_RESULT_INVALID_ACCESS);
    CHECK(v == 31337); /* untouched on failure */

    destroy_array(&arr);
}

static void test_at_on_empty(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));

    int v;
    CHECK_RES(hmc_static_array_at(arr, &v, 0), HMC_RESULT_INVALID_ACCESS);

    destroy_array(&arr);
}

static void test_at_invalid_arguments(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    push_range(arr, 0, 1);

    int v;
    CHECK_RES(hmc_static_array_at(NULL, &v, 0), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_static_array_at(arr, NULL, 0), HMC_RESULT_INVALID_ARGUMENT);

    destroy_array(&arr);
}

/* ------------------------------------------------------------------ */
/* size / clear                                                        */
/* ------------------------------------------------------------------ */

static void test_size_tracks_operations(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 8));
    CHECK(get_size(arr) == 0);

    int v = 1;
    CHECK_OK(hmc_static_array_push(arr, &v));
    CHECK(get_size(arr) == 1);
    CHECK_OK(hmc_static_array_insert(arr, &v, 0));
    CHECK(get_size(arr) == 2);
    CHECK_OK(hmc_static_array_pop(arr, NULL));
    CHECK(get_size(arr) == 1);
    CHECK_OK(hmc_static_array_remove(arr, NULL, 0));
    CHECK(get_size(arr) == 0);

    /* Failed operations must not change size. */
    CHECK_ERR(hmc_static_array_pop(arr, NULL));
    CHECK(get_size(arr) == 0);

    destroy_array(&arr);
}

static void test_size_invalid_arguments(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));

    hsize_t s;
    CHECK_RES(hmc_static_array_size(NULL, &s), HMC_RESULT_INVALID_ARGUMENT);
    CHECK_RES(hmc_static_array_size(arr, NULL), HMC_RESULT_INVALID_ARGUMENT);

    destroy_array(&arr);
}

static void test_clear_empties_and_keeps_capacity(void)
{
    enum { CAP = 5 };
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), CAP));

    push_range(arr, 0, CAP);
    CHECK_OK(hmc_static_array_clear(arr));
    CHECK(get_size(arr) == 0);

    int v;
    CHECK_RES(hmc_static_array_at(arr, &v, 0), HMC_RESULT_INVALID_ACCESS);

    /* Full capacity is available again, and no more than that. */
    push_range(arr, 100, 100 + CAP);
    int extra = 1;
    CHECK_RES(hmc_static_array_push(arr, &extra), HMC_RESULT_RESOURCE_UNAVAILABLE);

    const int exp[CAP] = {100, 101, 102, 103, 104};
    expect_contents(arr, exp, CAP);

    destroy_array(&arr);
}

static void test_clear_on_empty_and_repeated(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));

    CHECK_OK(hmc_static_array_clear(arr));
    CHECK_OK(hmc_static_array_clear(arr));
    CHECK(get_size(arr) == 0);

    destroy_array(&arr);
}

static void test_clear_null_array(void)
{
    CHECK_RES(hmc_static_array_clear(NULL), HMC_RESULT_INVALID_ARGUMENT);
}

/* ------------------------------------------------------------------ */
/* keep_allocator                                                      */
/* ------------------------------------------------------------------ */

static void test_keep_allocator_null_array(void)
{
    CHECK_RES(hmc_static_array_keep_allocator(NULL), HMC_RESULT_INVALID_ARGUMENT);
}

static void test_keep_allocator_returns_ownership(void)
{
    HMCStaticArray *arr = NULL;
    HMCAllocator *a = NULL;

    CHECK_OK(make_array(&arr, sizeof(int), 4));
    push_range(arr, 0, 4);

    CHECK_OK(hmc_static_array_keep_allocator(arr));
    CHECK_OK(hmc_static_array_deinit(&arr, &a));
    CHECK(a != NULL);

    /* The returned allocator is still alive and usable for a new array. */
    HMCStaticArray *arr2 = NULL;
    CHECK_OK(hmc_static_array_init(&arr2, &a, sizeof(int), 4));
    push_range(arr2, 20, 24);
    const int exp[] = {20, 21, 22, 23};
    expect_contents(arr2, exp, 4);
    destroy_array(&arr2); /* arr2 owns the allocator now and releases it */
}

static void test_deinit_without_keep_ignores_out_allocator(void)
{
    /*
     * Without keep_allocator the array destroys the allocator, so we should
     * not get a live allocator back. We only check that this succeeds and
     * leave handing-back semantics to the keep_allocator tests.
     */
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));
    CHECK_OK(hmc_static_array_deinit(&arr, NULL));
}

/* ------------------------------------------------------------------ */
/* Element types                                                       */
/* ------------------------------------------------------------------ */

typedef struct {
    int id;
    double weight;
    char tag[8];
} Rec;

static void test_struct_elements(void)
{
    enum { CAP = 16 };
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(Rec), CAP));

    for (int i = 0; i < CAP; i++) {
        Rec r;
        memset(&r, 0, sizeof r);
        r.id = i;
        r.weight = i * 1.5;
        snprintf(r.tag, sizeof r.tag, "t%d", i);
        CHECK_OK(hmc_static_array_push(arr, &r));
    }

    for (int i = 0; i < CAP; i++) {
        Rec r;
        CHECK_OK(hmc_static_array_at(arr, &r, (hsize_t)i));
        CHECK(r.id == i);
        CHECK(r.weight == i * 1.5);
        char tag[8];
        snprintf(tag, sizeof tag, "t%d", i);
        CHECK(strcmp(r.tag, tag) == 0);
    }

    Rec removed;
    CHECK_OK(hmc_static_array_remove(arr, &removed, 3));
    CHECK(removed.id == 3);
    Rec shifted;
    CHECK_OK(hmc_static_array_at(arr, &shifted, 3));
    CHECK(shifted.id == 4);

    destroy_array(&arr);
}

static void test_single_byte_elements(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(unsigned char), 64));

    for (int i = 0; i < 64; i++) {
        unsigned char c = (unsigned char)(i * 3);
        CHECK_OK(hmc_static_array_push(arr, &c));
    }
    for (int i = 0; i < 64; i++) {
        unsigned char c = 0;
        CHECK_OK(hmc_static_array_at(arr, &c, (hsize_t)i));
        CHECK(c == (unsigned char)(i * 3));
    }
    unsigned char extra = 0;
    CHECK_RES(hmc_static_array_push(arr, &extra), HMC_RESULT_RESOURCE_UNAVAILABLE);

    destroy_array(&arr);
}

static void test_capacity_of_one(void)
{
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 1));

    int a = 1, b = 2, out = 0;
    CHECK_OK(hmc_static_array_push(arr, &a));
    CHECK_RES(hmc_static_array_push(arr, &b), HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK_RES(hmc_static_array_insert(arr, &b, 0), HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK_OK(hmc_static_array_pop(arr, &out));
    CHECK(out == 1);
    CHECK_OK(hmc_static_array_insert(arr, &b, 0));
    CHECK_OK(hmc_static_array_at(arr, &out, 0));
    CHECK(out == 2);

    destroy_array(&arr);
}

static void test_elements_are_copied_not_referenced(void)
{
    /* Array stores copies; changing the source afterwards changes nothing. */
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), 4));

    int v = 10;
    CHECK_OK(hmc_static_array_push(arr, &v));
    v = 99;

    int out = 0;
    CHECK_OK(hmc_static_array_at(arr, &out, 0));
    CHECK(out == 10);

    destroy_array(&arr);
}

/* ------------------------------------------------------------------ */
/* Randomized model-based test                                         */
/* ------------------------------------------------------------------ */

static unsigned long long g_rng = 0x9E3779B97F4A7C15ULL;

static unsigned rnd(unsigned bound)
{
    g_rng = g_rng * 6364136223846793005ULL + 1442695040888963407ULL;
    return (unsigned)((g_rng >> 33) % bound);
}

static void test_randomized_against_reference_model(void)
{
    enum { CAP = 64, STEPS = 20000 };
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), CAP));

    int model[CAP];
    hsize_t msize = 0;
    int next = 1;

    for (int step = 0; step < STEPS; step++) {
        unsigned op = rnd(100);

        if (op < 30) { /* push */
            int v = next++;
            hmcResult_t r = hmc_static_array_push(arr, &v);
            if (msize == CAP) {
                CHECK_RES(r, HMC_RESULT_RESOURCE_UNAVAILABLE);
            } else {
                CHECK_OK(r);
                model[msize++] = v;
            }
        } else if (op < 50) { /* insert at valid existing index */
            int v = next++;
            if (msize == 0) {
                hmcResult_t r = hmc_static_array_insert(arr, &v, 0);
                CHECK_OK(r);
                model[msize++] = v;
            } else {
                hsize_t idx = rnd((unsigned)msize);
                hmcResult_t r = hmc_static_array_insert(arr, &v, idx);
                if (msize == CAP) {
                    CHECK_RES(r, HMC_RESULT_RESOURCE_UNAVAILABLE);
                } else {
                    CHECK_OK(r);
                    memmove(&model[idx + 1], &model[idx],
                            (msize - idx) * sizeof(int));
                    model[idx] = v;
                    msize++;
                }
            }
        } else if (op < 65) { /* pop */
            int out = -1;
            hmcResult_t r = hmc_static_array_pop(arr, &out);
            if (msize == 0) {
                CHECK_ERR(r);
            } else {
                CHECK_OK(r);
                CHECK(out == model[msize - 1]);
                msize--;
            }
        } else if (op < 85) { /* remove at valid index */
            if (msize > 0) {
                hsize_t idx = rnd((unsigned)msize);
                int out = -1;
                CHECK_OK(hmc_static_array_remove(arr, &out, idx));
                CHECK(out == model[idx]);
                memmove(&model[idx], &model[idx + 1],
                        (msize - idx - 1) * sizeof(int));
                msize--;
            }
        } else if (op < 99) { /* at */
            if (msize > 0) {
                hsize_t idx = rnd((unsigned)msize);
                int out = -1;
                CHECK_OK(hmc_static_array_at(arr, &out, idx));
                CHECK(out == model[idx]);
            }
        } else { /* clear (rare) */
            CHECK_OK(hmc_static_array_clear(arr));
            msize = 0;
        }

        if (get_size(arr) != msize) {
            fprintf(stderr, "    size diverged at step %d\n", step);
            CHECK(get_size(arr) == msize);
            break;
        }
    }

    expect_contents(arr, model, msize);
    destroy_array(&arr);
}

/* ------------------------------------------------------------------ */
/* Large array                                                         */
/* ------------------------------------------------------------------ */

static void test_large_array_fill_and_verify(void)
{
    enum { CAP = 100000 };
    HMCStaticArray *arr = NULL;
    CHECK_OK(make_array(&arr, sizeof(int), CAP));

    for (int i = 0; i < CAP; i++) {
        int v = i ^ 0x5A5A;
        CHECK_OK(hmc_static_array_push(arr, &v));
    }
    CHECK(get_size(arr) == CAP);

    for (int i = 0; i < CAP; i += 997) {
        int v = 0;
        CHECK_OK(hmc_static_array_at(arr, &v, (hsize_t)i));
        CHECK(v == (i ^ 0x5A5A));
    }

    int extra = 0;
    CHECK_RES(hmc_static_array_push(arr, &extra), HMC_RESULT_RESOURCE_UNAVAILABLE);

    destroy_array(&arr);
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    /* init / deinit */
    RUN(test_init_deinit_basic);
    RUN(test_init_invalid_arguments);
    RUN(test_init_elen_mismatch_with_quantized_allocator);
    RUN(test_init_more_elements_than_allocator_holds);
    RUN(test_deinit_invalid_arguments);
    RUN(test_init_deinit_many_cycles);

    /* push / pop */
    RUN(test_push_appends_in_order);
    RUN(test_push_until_full);
    RUN(test_push_invalid_arguments);
    RUN(test_pop_returns_elements_lifo);
    RUN(test_pop_with_null_elem_discards);
    RUN(test_pop_on_empty_fails);
    RUN(test_pop_null_array);
    RUN(test_push_pop_interleaved);

    /* insert */
    RUN(test_insert_at_front_middle_and_shifts);
    RUN(test_insert_into_empty_at_zero);
    RUN(test_insert_into_full_array);
    RUN(test_insert_out_of_bounds);
    RUN(test_insert_invalid_arguments);

    /* remove */
    RUN(test_remove_first_middle_last);
    RUN(test_remove_with_null_elem);
    RUN(test_remove_out_of_bounds);
    RUN(test_remove_null_array);
    RUN(test_remove_until_empty_then_refill);

    /* at */
    RUN(test_at_returns_copy_and_does_not_remove);
    RUN(test_at_out_of_bounds);
    RUN(test_at_on_empty);
    RUN(test_at_invalid_arguments);

    /* size / clear */
    RUN(test_size_tracks_operations);
    RUN(test_size_invalid_arguments);
    RUN(test_clear_empties_and_keeps_capacity);
    RUN(test_clear_on_empty_and_repeated);
    RUN(test_clear_null_array);

    /* keep_allocator */
    RUN(test_keep_allocator_null_array);
    RUN(test_keep_allocator_returns_ownership);
    RUN(test_deinit_without_keep_ignores_out_allocator);

    /* element types / edge cases */
    RUN(test_struct_elements);
    RUN(test_single_byte_elements);
    RUN(test_capacity_of_one);
    RUN(test_elements_are_copied_not_referenced);

    /* randomized + large */
    RUN(test_randomized_against_reference_model);
    RUN(test_large_array_fill_and_verify);

    printf("\n%d checks, %d failed\n", g_checks, g_failed);
    return g_failed ? 1 : 0;
}
