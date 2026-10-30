#ifndef _HMC_STACK_
#define _HMC_STACK_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_allocator.h>
#include <hmc_results.h>

typedef struct HMCStack HMCStack;

HMC_ATTR_EXTERNAL hmcResult_t hmc_stack_init(HMCStack **st, HMCAllocator **allocator,  hsize_t elen, hsize_t nelem);

HMC_ATTR_EXTERNAL hmcResult_t hmc_stack_deinit(HMCStack **st, HMCAllocator **allocator);

HMC_ATTR_EXTERNAL hmcResult_t hmc_stack_keep_allocator(HMCStack *st);

HMC_ATTR_EXTERNAL hmcResult_t hmc_stack_push(HMCStack *st, void *elem);

HMC_ATTR_EXTERNAL hmcResult_t hmc_stack_pop(HMCStack *st, void *elem);

HMC_ATTR_EXTERNAL hmcResult_t hmc_stack_top(HMCStack *st, void *elem);

HMC_ATTR_EXTERNAL hmcResult_t hmc_stack_size(HMCStack *st, hsize_t *size);

HMC_ATTR_EXTERNAL hmcResult_t hmc_stack_clear(HMCStack *st);


#endif
