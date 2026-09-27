#ifndef _HMC_GLOBAL_ALLOCATOR_
#define _HMC_GLOBAL_ALLOCATOR_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_results.h>

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_g_alloc(void **mem, hsize_t nbytes) HMC_ATTR_NO_NULL;
HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_g_realloc(void **mem, hsize_t nbytes) HMC_ATTR_NO_NULL;
HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_g_ialloc(void **mem, hsize_t nbytes,
                                           u8_t val) HMC_ATTR_NO_NULL;
HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_g_free(void **mem) HMC_ATTR_NO_NULL;

#endif
