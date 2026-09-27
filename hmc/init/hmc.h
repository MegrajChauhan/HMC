#ifndef _HMC_
#define _HMC_

#include <hmc_base_defn.h>
#include <hmc_base_platform.h>
#include <hmc_base_types.h>
#include <hmc_config.h>
#include <hmc_global_allocator.h>
#include <hmc_results.h>

#define _HMC_CONFIG_DEFAULT_ 0

typedef struct HMCContext HMCContext;

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_lib_init(u64_t conf);

HMC_ATTR_NO_DISCARD HMC_ATTR_EXTERNAL hmcResult_t hmc_lib_deinit();

#endif
