#ifndef _HMC_FILE_
#define _HMC_FILE_

#include <hmc_base_types.h>
#include <hmc_results.h>

#define hmc_file_open(f, path, ...) hmc_file_open_((f), (path), (HMCFileProperties){0,##__VA_ARGS__})

typedef struct HMCFile HMCFile;
typedef struct HMCFileProperties HMCFileProperties;

typedef enum hpos_t {
  HMC_POS_CUR,
  HMC_POS_BEG,
  HMC_POS_END,
} hpos_t;

struct HMCFileProperties {
  u64_t read: 1;
  u64_t write: 1;
  u64_t append: 1;
  u64_t resb: 61;
};

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_init(HMCFile **f);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_deinit(HMCFile **f);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_open_(HMCFile *f, const char *path, HMCFileProperties props);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_close(HMCFile *f);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_read(HMCFile *f, void *buf, hsize_t* len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_write(HMCFile *f, void *buf, hsize_t* len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_tellp(HMCFile *f, hsize_t *off);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL hmcResult_t hmc_file_seekp(HMCFile *f, hsize_t off, hpos_t whence);

#endif
