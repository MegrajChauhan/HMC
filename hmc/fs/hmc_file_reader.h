#ifndef _HMC_FILE_READER_
#define _HMC_FILE_READER_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_buffered_input_stream.h>
#include <hmc_file.h>
#include <hmc_results.h>

typedef struct HMCFileReader HMCFileReader;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_reader_init(HMCFileReader **f);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_reader_deinit(HMCFileReader **f);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_reader_open(HMCFileReader *f, const char *path);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_reader_close(HMCFileReader *f);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_reader_read(HMCFileReader *f, void *buf, hsize_t* len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_reader_tellp(HMCFileReader *f, hsize_t *off);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_reader_seekp(HMCFileReader *f, hsize_t off, hpos_t whence);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_reader_get_input_stream(HMCFileReader **f, HMCBufferedInputStream **bis);

#endif
