#ifndef _HMC_OUTPUT_STREAM_
#define _HMC_OUTPUT_STREAM_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_results.h>

typedef struct HMCOutputStream HMCOutputStream;
typedef struct HMCOSVT HMCOSVT;
typedef hmcResult_t (*hwriteOS_t)(void *, void *, hsize_t*);

struct HMCOSVT {
  hwriteOS_t OS_write; // write from the Output destination
};

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_init(HMCOutputStream **os);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_deinit(HMCOutputStream **os);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_bind(HMCOutputStream *os, void **dest, const HMCOSVT* vt);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_unbind(HMCOutputStream *os, void **dest);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_write_bytes(HMCOutputStream *os, u8_t *buf, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_write_i16(HMCOutputStream *os, i16_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_write_i32(HMCOutputStream *os, i32_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_write_i64(HMCOutputStream *os, i64_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_write_u16(HMCOutputStream *os, u16_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_write_u32(HMCOutputStream *os, u32_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_write_u64(HMCOutputStream *os, u64_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_write_double(HMCOutputStream *os, double *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_output_stream_write_float(HMCOutputStream *os, float *buf, hsize_t *N);

#endif
