#ifndef _HMC_BUFFER_
#define _HMC_BUFFER_

#include <hmc_base_types.h>
#include <hmc_file_defs.h>
#include <hmc_results.h>

typedef struct HMCBuffer HMCBuffer;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffer_init(HMCBuffer **buf);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL_SPECIFY(1) hmcResult_t hmc_buffer_deinit(HMCBuffer **buf, void **buffer);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL_SPECIFY(1) hmcResult_t hmc_buffer_prepare(HMCBuffer *buf, void **buffer, hsize_t len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL_SPECIFY(1) hmcResult_t hmc_buffer_release(HMCBuffer *buf, void **buffer);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffer_read(HMCBuffer *buf, void *dest, hsize_t* len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffer_write(HMCBuffer *buf, void *src, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffer_seekp(HMCBuffer *buf, hsize_t off, hpos_t whence);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_buffer_tellp(HMCBuffer *buf, hsize_t *off);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL_SPECIFY(1) hmcResult_t hmc_buffer_resize(HMCBuffer *buf, void **buffer, hsize_t nlen);

#endif
