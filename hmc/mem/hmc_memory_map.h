#ifndef _HMC_MEMORY_MAP_
#define _HMC_MEMORY_MAP_

#include <hmc_base_types.h>
#include <hmc_global_allocator.h>
#include <hmc_results.h>

typedef struct HMCMemoryMap HMCMemoryMap;
typedef struct HMCMapProps HMCMapProps;

struct HMCMapProps {
  u64_t readable: 1;
  u64_t writable: 1;
  u64_t executable: 1; /*Currently won't be implemented*/
  u64_t private: 1; // '1' = private else share [in terms of process ownership]
  u64_t resb: 60;
};

#define hmc_memory_map_get_mapped_memory(map, len, ...) hmc_memory_map_get_mapped_memory_((map), (len), (HMCMapProps){.readable=1, .writable=1, .executable=0, .private=1, ##__VA_ARGS__})

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_init(HMCMemoryMap **map) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_deinit(HMCMemoryMap **map) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_obtain_ptr(HMCMemoryMap *map, void **mem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_release_ptr(HMCMemoryMap *map, void **mem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_get_mapped_memory_(HMCMemoryMap *map, hsize_t len, HMCMapProps props) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_release_mapped_memory(HMCMemoryMap *map) HMC_ATTR_NO_NULL;

#endif
