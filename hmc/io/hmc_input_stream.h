#ifndef _HMC_INPUT_STREAM_
#define _HMC_INPUT_STREAM_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_results.h>

typedef struct HMCInputStream HMCInputStream;
typedef struct HMCISVT HMCISVT;
typedef hmcResult_t (*hreadIS_t)(void *, void *, hsize_t*);
typedef hmcResult_t (*hprobeIS_t)(void *, hsize_t *);

struct HMCISVT {
  hreadIS_t IS_read;   // read from the Input Source
  hprobeIS_t IS_probe; // probe the Input Source for any data (Optional)
};

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_init(HMCInputStream **is);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_deinit(HMCInputStream **is);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_bind(HMCInputStream *is, void **source, const HMCISVT *vt);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_unbind(HMCInputStream *is, void **source);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_read_bytes(HMCInputStream *is, u8_t *buf, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_read_i16(HMCInputStream *is, i16_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_read_i32(HMCInputStream *is, i32_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_read_i64(HMCInputStream *is, i64_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_read_u16(HMCInputStream *is, u16_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_read_u32(HMCInputStream *is, u32_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_read_u64(HMCInputStream *is, u64_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_read_double(HMCInputStream *is, double *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_input_stream_read_float(HMCInputStream *is, float *buf, hsize_t *N);

#endif
