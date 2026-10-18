#ifndef _HMC_POOL_ALLOCATOR_
#define _HMC_POOL_ALLOCATOR_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_results.h>

typedef struct HMCPoolAllocator HMCPoolAllocator;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_pool_allocator_init(HMCAllocator **allocator) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_pool_allocator_deinit(HMCAllocator **allocator) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_pool_allocator_share(HMCAllocator **src, HMCAllocator **dst) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_pool_allocator_unshare(HMCAllocator **allocator) HMC_ATTR_NO_NULL;

#endif
