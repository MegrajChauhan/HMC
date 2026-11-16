#ifndef _HMC_RAND_GENERATOR_VT_
#define _HMC_RAND_GENERATOR_VT_

#include <hmc_base_types.h>

typedef struct HMCRGVT HMCRGVT;

typedef hmcResult_t (*hgendeinit_t)(void**);
typedef hmcResult_t (*hgenint32_t)(void*, u32_t*);
typedef hmcResult_t (*hgenint64_t)(void*, u64_t*);

struct HMCRGVT {
  hgenint32_t gen_32;
  hgenint64_t gen_64;
  hgendeinit_t gen_deinit;
};

#endif
