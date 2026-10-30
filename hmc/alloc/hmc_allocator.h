#ifndef _HMC_ALLOCATOR_
#define _HMC_ALLOCATOR_

#include <hmc_base_types.h>
#include <hmc_results.h>

typedef struct HMCAllocatorVT HMCAllocatorVT;
typedef struct HMCAllocator HMCAllocator;

HMC_ATTR_EXTERNAL hmcResult_t hmc_allocator_destroy(HMCAllocator **allocator) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_alloc(HMCAllocator *allocator, void **mem,
                                        hsize_t len) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_free(HMCAllocator *allocator, void **mem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_realloc(HMCAllocator *allocator, void **mem,
                                          hsize_t len) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_ialloc(HMCAllocator *allocator, void **mem,
                                         hsize_t len, u8_t val) HMC_ATTR_NO_NULL;

#endif
