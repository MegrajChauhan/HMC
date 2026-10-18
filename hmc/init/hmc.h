#ifndef _HMC_
#define _HMC_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_config.h>
#include <hmc_results.h>

typedef struct HMCContext HMCContext;
typedef struct HMCConfig HMCConfig;

struct HMCConfig {
  double alloc_g_flush_max;
  hsize_t alloc_g_alloc_lim;
};

#define hmc_lib_init(...) hmc_lib_init_((HMCConfig){.alloc_g_flush_max = 0.75, \
                                                     .alloc_g_alloc_lim = 4*1024, \
                                                      ##__VA_ARGS__})

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_lib_init_(HMCConfig conf);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_lib_deinit();

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_lib_set_alloc_g_flush_max(double flush_max);

#endif
