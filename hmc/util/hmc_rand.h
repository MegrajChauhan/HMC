#ifndef _HMC_RAND_
#define _HMC_RAND_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_rand_generator_vt.h>
#include <hmc_results.h>

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_random_byte(u8_t *dest);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_random_word(u16_t *dest);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_random_dword(u32_t *dest);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_random_qword(u64_t *dest);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_random_double(double *dest);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_random_float(float *dest);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL const HMCRGVT* hmc_random_get_VT();
HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL void* hmc_random_get_instance();

#endif
