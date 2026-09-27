#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <hmc_allocator.h>
#include <hmc_linear_allocator.h>
#include <hmc.h>

/* ============================== harness ============================== */

static int g_tests_run = 0;
static int g_tests_failed = 0;
static int g_current_test_failed = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("      FAIL (%s:%d): %s\n", __FILE__, __LINE__, #cond);   \
            g_current_test_failed = 1;                                       \
        }                                                                    \
    } while (0)

#define CHECK_RESULT(actual, expected)                                       \
    CHECK((actual) == (expected))

#define RUN_TEST(fn)                                                         \
    do {                                                                     \
        g_current_test_failed = 0;                                          \
        g_tests_run++;                                                      \
        printf("[ RUN  ] %s\n", #fn);                                       \
        fn();                                                               \
        if (g_current_test_failed) {                                        \
            g_tests_failed++;                                               \
            printf("[ FAIL ] %s\n", #fn);                                   \
        } else {                                                            \
            printf("[  OK  ] %s\n", #fn);                                   \
        }                                                                   \
    } while (0)

/* ============================ shared config ============================ */

#define DEFAULT_ELEM_LEN ((hsize_t)16)
#define DEFAULT_NELEM    ((hsize_t)8)

static HMCAllocator *make_allocator(hsize_t elem_len, hsize_t nelem) {
    HMCAllocator *allocator = NULL;
    hmcResult_t res = hmc_linear_allocator_init(&allocator, elem_len, nelem);
    CHECK_RESULT(res, HMC_RESULT_SUCCESS);
    CHECK(allocator != NULL);
    return allocator;
}

static void destroy_allocator(HMCAllocator *allocator) {
    hmcResult_t res = hmc_linear_allocator_deinit(&allocator);
    CHECK_RESULT(res, HMC_RESULT_SUCCESS);
}

/* ============================ init / deinit ============================ */

static void test_init_valid(void) {
    HMCAllocator *allocator = NULL;
    hmcResult_t res =
        hmc_linear_allocator_init(&allocator, DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    CHECK_RESULT(res, HMC_RESULT_SUCCESS);
    CHECK(allocator != NULL);

    hmc_linear_allocator_deinit(&allocator);
}

static void test_init_null_allocator_ptr(void) {
    hmcResult_t res =
        hmc_linear_allocator_init(NULL, DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);
}

static void test_init_zero_elem_len(void) {
    HMCAllocator *allocator = NULL;
    hmcResult_t res = hmc_linear_allocator_init(&allocator, 0, DEFAULT_NELEM);

    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);
    CHECK(allocator == NULL);
}

static void test_init_zero_nelem(void) {
    HMCAllocator *allocator = NULL;
    hmcResult_t res =
        hmc_linear_allocator_init(&allocator, DEFAULT_ELEM_LEN, 0);

    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);
    CHECK(allocator == NULL);
}

static void test_init_minimum_valid_sizes(void) {
    HMCAllocator *allocator = make_allocator((hsize_t)1, (hsize_t)1);

    void *mem = NULL;
    hmcResult_t res = hmc_alloc(allocator, &mem, (hsize_t)1);
    CHECK_RESULT(res, HMC_RESULT_SUCCESS);
    CHECK(mem != NULL);

    destroy_allocator(allocator);
}

static void test_init_multiple_independent_instances(void) {
    HMCAllocator *a = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);
    HMCAllocator *b = make_allocator((hsize_t)32, (hsize_t)4);

    CHECK(a != b);

    void *mem_a = NULL;
    void *mem_b = NULL;
    CHECK_RESULT(hmc_alloc(a, &mem_a, (hsize_t)1), HMC_RESULT_SUCCESS);
    CHECK_RESULT(hmc_alloc(b, &mem_b, (hsize_t)1), HMC_RESULT_SUCCESS);
    CHECK(mem_a != NULL);
    CHECK(mem_b != NULL);
    CHECK(mem_a != mem_b);

    void *tmp = NULL;
    CHECK_RESULT(hmc_alloc(b, &tmp, (hsize_t)1), HMC_RESULT_SUCCESS);
    CHECK_RESULT(hmc_alloc(b, &tmp, (hsize_t)1), HMC_RESULT_SUCCESS);
    CHECK_RESULT(hmc_alloc(b, &tmp, (hsize_t)1), HMC_RESULT_SUCCESS);
    CHECK_RESULT(hmc_alloc(b, &tmp, (hsize_t)1), HMC_RESULT_RESOURCE_UNAVAILABLE);

    CHECK_RESULT(hmc_alloc(a, &tmp, (hsize_t)1), HMC_RESULT_SUCCESS);

    destroy_allocator(a);
    destroy_allocator(b);
}

static void test_deinit_null(void) {
    hmcResult_t res = hmc_linear_allocator_deinit(NULL);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);
}

static void test_deinit_null_pointee(void) {
    HMCAllocator *allocator = NULL;
    hmcResult_t res = hmc_linear_allocator_deinit(&allocator);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);
}

static void test_deinit_nulls_pointer(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    hmcResult_t res = hmc_linear_allocator_deinit(&allocator);
    CHECK_RESULT(res, HMC_RESULT_SUCCESS);
    CHECK(allocator == NULL);
}

/* ================================ alloc ================================= */

static void test_alloc_within_capacity(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    void *mem = NULL;
    hmcResult_t res = hmc_alloc(allocator, &mem, (hsize_t)1);

    CHECK_RESULT(res, HMC_RESULT_SUCCESS);
    CHECK(mem != NULL);

    destroy_allocator(allocator);
}

static void test_alloc_returns_writable_memory(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    void *mem = NULL;
    CHECK_RESULT(hmc_alloc(allocator, &mem, (hsize_t)1), HMC_RESULT_SUCCESS);
    CHECK(mem != NULL);

    if (mem != NULL) {
        memset(mem, 0xAB, (size_t)DEFAULT_ELEM_LEN);
        unsigned char *bytes = (unsigned char *)mem;
        int all_match = 1;
        for (hsize_t i = 0; i < DEFAULT_ELEM_LEN; i++) {
            if (bytes[i] != 0xAB) {
                all_match = 0;
                break;
            }
        }
        CHECK(all_match);
    }

    destroy_allocator(allocator);
}

static void test_alloc_until_exhausted(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    for (hsize_t i = 0; i < DEFAULT_NELEM; i++) {
        void *mem = NULL;
        hmcResult_t res = hmc_alloc(allocator, &mem, (hsize_t)1);
        CHECK_RESULT(res, HMC_RESULT_SUCCESS);
        CHECK(mem != NULL);
    }

    destroy_allocator(allocator);
}

static void test_alloc_beyond_capacity(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    for (hsize_t i = 0; i < DEFAULT_NELEM; i++) {
        void *mem = NULL;
        CHECK_RESULT(hmc_alloc(allocator, &mem, (hsize_t)1), HMC_RESULT_SUCCESS);
    }

    void *overflow = NULL;
    hmcResult_t res = hmc_alloc(allocator, &overflow, (hsize_t)1);
    CHECK_RESULT(res, HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK(overflow == NULL);

    destroy_allocator(allocator);
}

static void test_alloc_single_call_exceeding_capacity(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    void *mem = NULL;
    hmcResult_t res =
        hmc_alloc(allocator, &mem, (hsize_t)(DEFAULT_NELEM + 1));

    CHECK_RESULT(res, HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK(mem == NULL);

    destroy_allocator(allocator);
}

static void test_alloc_exact_capacity_in_one_call(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    void *mem = NULL;
    hmcResult_t res = hmc_alloc(allocator, &mem, DEFAULT_NELEM);
    CHECK_RESULT(res, HMC_RESULT_SUCCESS);
    CHECK(mem != NULL);

    void *overflow = NULL;
    CHECK_RESULT(hmc_alloc(allocator, &overflow, (hsize_t)1),
                 HMC_RESULT_RESOURCE_UNAVAILABLE);

    destroy_allocator(allocator);
}

static void test_alloc_null_allocator(void) {
    void *mem = NULL;
    hmcResult_t res = hmc_alloc(NULL, &mem, (hsize_t)1);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);
}

static void test_alloc_null_mem_ptr(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    hmcResult_t res = hmc_alloc(allocator, NULL, (hsize_t)1);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);

    destroy_allocator(allocator);
}

static void test_alloc_zero_len(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    void *mem = NULL;
    hmcResult_t res = hmc_alloc(allocator, &mem, (hsize_t)0);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);

    destroy_allocator(allocator);
}

static void test_alloc_is_sequential(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    void *first = NULL;
    void *second = NULL;
    CHECK_RESULT(hmc_alloc(allocator, &first, (hsize_t)1), HMC_RESULT_SUCCESS);
    CHECK_RESULT(hmc_alloc(allocator, &second, (hsize_t)1), HMC_RESULT_SUCCESS);

    ptrdiff_t stride = (unsigned char *)second - (unsigned char *)first;
    CHECK(stride == (ptrdiff_t)DEFAULT_ELEM_LEN);

    destroy_allocator(allocator);
}

/* =============================== ialloc ================================= */

static void test_ialloc_sets_all_bytes(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    void *mem = NULL;
    hmcResult_t res = hmc_ialloc(allocator, &mem, (hsize_t)1, (u8_t)0x7E);

    CHECK_RESULT(res, HMC_RESULT_SUCCESS);
    CHECK(mem != NULL);

    if (mem != NULL) {
        unsigned char *bytes = (unsigned char *)mem;
        int all_match = 1;
        for (hsize_t i = 0; i < DEFAULT_ELEM_LEN; i++) {
            if (bytes[i] != 0x7E) {
                all_match = 0;
                break;
            }
        }
        CHECK(all_match);
    }

    destroy_allocator(allocator);
}

static void test_ialloc_boundary_values(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);
    const u8_t values[] = {0x00, 0xFF, 0x01, 0xAA};

    for (size_t v = 0; v < sizeof(values) / sizeof(values[0]); v++) {
        void *mem = NULL;
        hmcResult_t res = hmc_ialloc(allocator, &mem, (hsize_t)1, values[v]);
        CHECK_RESULT(res, HMC_RESULT_SUCCESS);
        CHECK(mem != NULL);

        if (mem != NULL) {
            unsigned char *bytes = (unsigned char *)mem;
            for (hsize_t i = 0; i < DEFAULT_ELEM_LEN; i++) {
                CHECK(bytes[i] == (unsigned char)values[v]);
            }
        }
    }

    destroy_allocator(allocator);
}

static void test_ialloc_until_exhausted(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    for (hsize_t i = 0; i < DEFAULT_NELEM; i++) {
        void *mem = NULL;
        hmcResult_t res = hmc_ialloc(allocator, &mem, (hsize_t)1, (u8_t)i);
        CHECK_RESULT(res, HMC_RESULT_SUCCESS);
        CHECK(mem != NULL);
    }

    void *overflow = NULL;
    CHECK_RESULT(hmc_ialloc(allocator, &overflow, (hsize_t)1, (u8_t)0),
                 HMC_RESULT_RESOURCE_UNAVAILABLE);

    destroy_allocator(allocator);
}

static void test_ialloc_null_allocator(void) {
    void *mem = NULL;
    hmcResult_t res = hmc_ialloc(NULL, &mem, (hsize_t)1, (u8_t)0);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);
}

static void test_ialloc_null_mem_ptr(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    hmcResult_t res = hmc_ialloc(allocator, NULL, (hsize_t)1, (u8_t)0);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);

    destroy_allocator(allocator);
}

static void test_alloc_and_ialloc_share_capacity(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    for (hsize_t i = 0; i < DEFAULT_NELEM; i++) {
        void *mem = NULL;
        hmcResult_t res = (i % 2 == 0)
                               ? hmc_alloc(allocator, &mem, (hsize_t)1)
                               : hmc_ialloc(allocator, &mem, (hsize_t)1, (u8_t)0);
        CHECK_RESULT(res, HMC_RESULT_SUCCESS);
        CHECK(mem != NULL);
    }

    void *overflow = NULL;
    CHECK_RESULT(hmc_alloc(allocator, &overflow, (hsize_t)1),
                 HMC_RESULT_RESOURCE_UNAVAILABLE);
    CHECK_RESULT(hmc_ialloc(allocator, &overflow, (hsize_t)1, (u8_t)0),
                 HMC_RESULT_RESOURCE_UNAVAILABLE);

    destroy_allocator(allocator);
}

/* ============================ free / realloc ============================ */

static void test_free_documented_result(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    void *mem = NULL;
    CHECK_RESULT(hmc_alloc(allocator, &mem, (hsize_t)1), HMC_RESULT_SUCCESS);
    memset(mem, 0x5A, (size_t)DEFAULT_ELEM_LEN);

    hmcResult_t res = hmc_free(allocator, &mem);
    CHECK(res == HMC_RESULT_UNAVAILABLE || res == HMC_RESULT_OPERATION_UNSUPPORTED);

    unsigned char *bytes = (unsigned char *)mem;
    int untouched = 1;
    for (hsize_t i = 0; i < DEFAULT_ELEM_LEN; i++) {
        if (bytes[i] != 0x5A) {
            untouched = 0;
            break;
        }
    }
    CHECK(untouched);

    void *next = NULL;
    CHECK_RESULT(hmc_alloc(allocator, &next, (hsize_t)1), HMC_RESULT_SUCCESS);

    destroy_allocator(allocator);
}

static void test_free_null_allocator(void) {
    void *mem = (void *)0x1; /* non-NULL dummy */
    hmcResult_t res = hmc_free(NULL, &mem);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);
}

static void test_free_null_mem(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    hmcResult_t res = hmc_free(allocator, NULL);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);

    destroy_allocator(allocator);
}

static void test_realloc_documented_result(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    void *mem = NULL;
    CHECK_RESULT(hmc_alloc(allocator, &mem, (hsize_t)1), HMC_RESULT_SUCCESS);

    hmcResult_t res = hmc_realloc(allocator, &mem, (hsize_t)2);
    CHECK(res == HMC_RESULT_UNAVAILABLE || res == HMC_RESULT_OPERATION_UNSUPPORTED);

    destroy_allocator(allocator);
}

static void test_realloc_null_allocator(void) {
    void *mem = (void *)0x1;
    hmcResult_t res = hmc_realloc(NULL, &mem, (hsize_t)1);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);
}

static void test_realloc_null_mem(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);

    hmcResult_t res = hmc_realloc(allocator, NULL, (hsize_t)1);
    CHECK_RESULT(res, HMC_RESULT_INVALID_ARGUMENT);

    destroy_allocator(allocator);
}

/* ============================ full lifecycle ============================ */

static void test_full_lifecycle(void) {
    HMCAllocator *allocator = make_allocator((hsize_t)64, (hsize_t)4);

    void *elems[4] = {0};
    for (int i = 0; i < 4; i++) {
        hmcResult_t res = hmc_ialloc(allocator, &elems[i], (hsize_t)1, (u8_t)(i + 1));
        CHECK_RESULT(res, HMC_RESULT_SUCCESS);
        CHECK(elems[i] != NULL);
    }

    for (int i = 0; i < 4; i++) {
        unsigned char *bytes = (unsigned char *)elems[i];
        CHECK(bytes[0] == (unsigned char)(i + 1));
    }

    void *overflow = NULL;
    CHECK_RESULT(hmc_alloc(allocator, &overflow, (hsize_t)1),
                 HMC_RESULT_RESOURCE_UNAVAILABLE);

    hmcResult_t deinit_res = hmc_linear_allocator_deinit(&allocator);
    CHECK_RESULT(deinit_res, HMC_RESULT_SUCCESS);
    CHECK(allocator == NULL);
}

static void test_reinit_after_deinit(void) {
    HMCAllocator *allocator = make_allocator(DEFAULT_ELEM_LEN, DEFAULT_NELEM);
    destroy_allocator(allocator);

    allocator = NULL;
    hmcResult_t res =
        hmc_linear_allocator_init(&allocator, DEFAULT_ELEM_LEN, DEFAULT_NELEM);
    CHECK_RESULT(res, HMC_RESULT_SUCCESS);
    CHECK(allocator != NULL);

    void *mem = NULL;
    CHECK_RESULT(hmc_alloc(allocator, &mem, (hsize_t)1), HMC_RESULT_SUCCESS);

    destroy_allocator(allocator);
}

/* ================================= main ================================= */

int main(void) {
    hmc_lib_init(_HMC_CONFIG_DEFAULT_);
    RUN_TEST(test_init_valid);
    RUN_TEST(test_init_null_allocator_ptr);
    RUN_TEST(test_init_zero_elem_len);
    RUN_TEST(test_init_zero_nelem);
    RUN_TEST(test_init_minimum_valid_sizes);
    RUN_TEST(test_init_multiple_independent_instances);

    RUN_TEST(test_deinit_null);
    RUN_TEST(test_deinit_null_pointee);
    RUN_TEST(test_deinit_nulls_pointer);

    RUN_TEST(test_alloc_within_capacity);
    RUN_TEST(test_alloc_returns_writable_memory);
    RUN_TEST(test_alloc_until_exhausted);
    RUN_TEST(test_alloc_beyond_capacity);
    RUN_TEST(test_alloc_single_call_exceeding_capacity);
    RUN_TEST(test_alloc_exact_capacity_in_one_call);
    RUN_TEST(test_alloc_null_allocator);
    RUN_TEST(test_alloc_null_mem_ptr);
    RUN_TEST(test_alloc_zero_len);
    RUN_TEST(test_alloc_is_sequential);

    RUN_TEST(test_ialloc_sets_all_bytes);
    RUN_TEST(test_ialloc_boundary_values);
    RUN_TEST(test_ialloc_until_exhausted);
    RUN_TEST(test_ialloc_null_allocator);
    RUN_TEST(test_ialloc_null_mem_ptr);
    RUN_TEST(test_alloc_and_ialloc_share_capacity);

    RUN_TEST(test_free_documented_result);
    RUN_TEST(test_free_null_allocator);
    RUN_TEST(test_free_null_mem);
    RUN_TEST(test_realloc_documented_result);
    RUN_TEST(test_realloc_null_allocator);
    RUN_TEST(test_realloc_null_mem);

    RUN_TEST(test_full_lifecycle);
    RUN_TEST(test_reinit_after_deinit);

    printf("\n%d/%d tests passed\n", g_tests_run - g_tests_failed, g_tests_run);
    hmc_lib_deinit();
    return g_tests_failed == 0 ? 0 : 1;
}
