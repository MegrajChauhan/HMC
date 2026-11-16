#ifndef _HMC_LF_STACK_
#define _HMC_LF_STACK_

#include <hmc_base_types.h>
#include <hmc_allocator.h>
#include <hmc_results.h>

typedef struct HMCLFStack HMCLFStack;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_lfstack_init(HMCLFStack **stack, HMCAllocator **allocator, hsize_t elen, hsize_t ecap);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL_SPECIFY(1) 
hmcResult_t hmc_lfstack_deinit(HMCLFStack **stack, HMCAllocator **allocator);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_lfstack_share(HMCLFStack *stack, HMCLFStack **dest);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_lfstack_unshare(HMCLFStack **stack);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_lfstack_keep_allocator(HMCLFStack *stack);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_lfstack_push(HMCLFStack *stack, void *data);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_lfstack_pop(HMCLFStack *stack, void *buf);

#endif
