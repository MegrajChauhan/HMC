#ifndef _HMC_BUFFERED_OUTPUT_STREAM_
#define _HMC_BUFFERED_OUTPUT_STREAM_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_output_stream.h>
#include <hmc_results.h>

typedef struct HMCBufferedOutputStream HMCBufferedOutputStream;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_init(HMCBufferedOutputStream **bos);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_deinit(HMCBufferedOutputStream **bos);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_bind(HMCBufferedOutputStream *bos, void **dest, const HMCOSVT* vt);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_unbind(HMCBufferedOutputStream *bos, void **dest);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_write_bytes(HMCBufferedOutputStream *bos, u8_t *buf, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_write_i16(HMCBufferedOutputStream *bos, i16_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_write_i32(HMCBufferedOutputStream *bos, i32_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_write_i64(HMCBufferedOutputStream *bos, i64_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_write_u16(HMCBufferedOutputStream *bos, u16_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_write_u32(HMCBufferedOutputStream *bos, u32_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_write_u64(HMCBufferedOutputStream *bos, u64_t *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_write_double(HMCBufferedOutputStream *bos, double *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_write_float(HMCBufferedOutputStream *bos, float *buf, hsize_t *N);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffered_output_stream_flush(HMCBufferedOutputStream *bos, hsize_t *len);

#endif
