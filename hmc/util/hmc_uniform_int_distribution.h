#ifndef _HMC_UNIFORM_INT_DISTRIBUTION_
#define _HMC_UNIFORM_INT_DISTRIBUTION_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_rand_generator_vt.h>
#include <hmc_results.h>

typedef struct HMCUniformIntDistribution HMCUniformIntDistribution;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_int_distribution_init(HMCUniformIntDistribution **uid);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_int_distribution_deinit(HMCUniformIntDistribution **uid);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_int_distribution_bind(HMCUniformIntDistribution *uid, void **generator, const HMCRGVT *vt);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_int_distribution_unbind(HMCUniformIntDistribution *uid, void **generator);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_int_distribution_set_range(HMCUniformIntDistribution *uid, u64_t max, u64_t min);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_gen(HMCUniformIntDistribution *uid, u64_t* res);

#endif
