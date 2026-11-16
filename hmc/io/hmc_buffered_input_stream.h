#ifndef _HMC_BUFFERED_INPUT_STREAM_
#define _HMC_BUFFERED_INPUT_STREAM_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_input_stream.h>
#include <hmc_results.h>

typedef struct HMCBufferedInputStream HMCBufferedInputStream;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_init(HMCBufferedInputStream **bis);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_deinit(HMCBufferedInputStream **bis);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_bind(HMCBufferedInputStream *bis, void **source, const HMCISVT* vt);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_unbind(HMCBufferedInputStream *bis, void **source);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_read_bytes(HMCBufferedInputStream *bis, u8_t *buf, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_read_i16(HMCBufferedInputStream *bis, i16_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_read_i32(HMCBufferedInputStream *bis, i32_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_read_i64(HMCBufferedInputStream *bis, i64_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_read_u16(HMCBufferedInputStream *bis, u16_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_read_u32(HMCBufferedInputStream *bis, u32_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_read_u64(HMCBufferedInputStream *bis, u64_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_read_double(HMCBufferedInputStream *bis, double *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_input_stream_read_float(HMCBufferedInputStream *bis, float *buf, hsize_t *N);

#endif
