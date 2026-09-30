#ifndef _HMC_ALLOCATOR_
#define _HMC_ALLOCATOR_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_results.h>

typedef struct HMCAllocatorVT HMCAllocatorVT;
typedef struct HMCAllocator HMCAllocator;
typedef enum hmcAllocator_t hmcAllocator_t;
typedef hmcResult_t (*hmcalloc_t)(void *, void **, hsize_t);
typedef hmcResult_t (*hmcfree_t)(void *, void **);
typedef hmcResult_t (*hmcrealloc_t)(void *, void **, hsize_t);
typedef hmcResult_t (*hmcialloc_t)(void *, void **, hsize_t, u8_t);
struct HMCAllocatorVT {
  hmcalloc_t hmc_mem_alloc;
  hmcfree_t hmc_mem_free;
  hmcrealloc_t hmc_mem_realloc;
  hmcialloc_t hmc_mem_ialloc;
};

enum hmcAllocator_t { HMC_ALLOCATOR_LINEAR, HMC_ALLOCATOR_INVALID };

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_allocator_destroy(HMCAllocator **allocator) HMC_ATTR_NO_NULL;

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_alloc(HMCAllocator *allocator, void **mem,
                                        hsize_t len) HMC_ATTR_NO_NULL;

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_free(HMCAllocator *allocator, void **mem)
                                                 HMC_ATTR_NO_NULL;

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_realloc(HMCAllocator *allocator, void **mem,
                                          hsize_t len) HMC_ATTR_NO_NULL;

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_ialloc(HMCAllocator *allocator, void **mem,
                                         hsize_t len, u8_t val) HMC_ATTR_NO_NULL;

#endif
