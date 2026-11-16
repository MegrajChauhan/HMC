#ifndef _HMC_MT19937_
#define _HMC_MT19937_

/*
 * Inspired by the official implementation provided by Matsumoto and Nishimura
 * */

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_rand_generator_vt.h>
#include <hmc_results.h>

typedef struct HMCmt19937 HMCmt19937;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_mt19937_init(HMCmt19937 **mt);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_mt19937_deinit(HMCmt19937 **mt);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_mt19937_seed(HMCmt19937 *mt, u32_t seed);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_mt19937_gen_int(HMCmt19937 *mt, u32_t *res);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_mt19937_gen_float(HMCmt19937 *mt, float *res);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL const HMCRGVT* hmc_mt19937_get_VT();

#endif
