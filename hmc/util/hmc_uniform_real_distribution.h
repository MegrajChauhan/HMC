#ifndef _HMC_UNIFORM_REAL_DISTRIBUTION_
#define _HMC_UNIFORM_REAL_DISTRIBUTION_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_rand_generator_vt.h>
#include <hmc_results.h>

typedef struct HMCUniformRealDistribution HMCUniformRealDistribution;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_real_distribution_init(HMCUniformRealDistribution **uid);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_real_distribution_deinit(HMCUniformRealDistribution **uid);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_real_distribution_bind(HMCUniformRealDistribution *uid, void **generator, const HMCRGVT *vt);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_real_distribution_unbind(HMCUniformRealDistribution *uid, void **generator);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_real_distribution_set_range(HMCUniformRealDistribution *uid, double max, double min);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_uniform_genf(HMCUniformRealDistribution *uid, double* res);

#endif
