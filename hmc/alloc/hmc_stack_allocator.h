#ifndef _HMC_STACK_ALLOCATOR_
#define _HMC_STACK_ALLOCATOR_

#include <hmc_allocator.h>
#include <hmc_base_types.h>
#include <hmc_results.h>

typedef struct HMCStackAllocator HMCStackAllocator;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_stack_allocator_init(
    HMCAllocator **allocator, hsize_t len, hsize_t alignment) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_stack_allocator_expand(
    HMCAllocator *allocator, hsize_t new_len, hsize_t* possible) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t
hmc_stack_allocator_deinit(HMCAllocator **allocator) HMC_ATTR_NO_NULL;

#endif
