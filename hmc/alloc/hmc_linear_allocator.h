#ifndef _HMC_LINEAR_ALLOCATOR_
#define _HMC_LINEAR_ALLOCATOR_

#include <hmc_allocator.h>
#include <hmc_base_types.h>
#include <hmc_global_allocator.h>
#include <hmc_results.h>

typedef struct HMCLinearAllocator HMCLinearAllocator;

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_linear_allocator_init(
    HMCAllocator **allocator, hsize_t elem_len, hsize_t nelem) HMC_ATTR_NO_NULL;

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t
hmc_linear_allocator_deinit(HMCAllocator **allocator) HMC_ATTR_NO_NULL;

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_linear_allocator_expand(
    HMCAllocator *allocator, hsize_t new_len, hsize_t* possible) HMC_ATTR_NO_NULL_SPECIFY(1);
#endif
