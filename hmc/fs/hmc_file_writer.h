#ifndef _HMC_FILE_WRITER_
#define _HMC_FILE_WRITER_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_buffered_output_stream.h>
#include <hmc_file.h>
#include <hmc_results.h>

typedef struct HMCFileWriter HMCFileWriter;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_writer_init(HMCFileWriter **f);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_writer_deinit(HMCFileWriter **f);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_writer_open(HMCFileWriter *f, const char *path, u8_t append);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_writer_close(HMCFileWriter *f);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_writer_write(HMCFileWriter *f, void *buf, hsize_t* len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_writer_tellp(HMCFileWriter *f, hsize_t *off);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_writer_seekp(HMCFileWriter *f, hsize_t off, hpos_t whence);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_writer_flush(HMCFileWriter *f, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_writer_get_output_stream(HMCFileWriter **f, HMCBufferedOutputStream **bos);

#endif
