#ifndef _HMC_MEMORY_MAP_
#define _HMC_MEMORY_MAP_

#include <hmc_base_types.h>
#include <hmc_global_allocator.h>
#include <hmc_results.h>

typedef struct HMCMemoryMap HMCMemoryMap;

#define hmc_memory_map_get_mapped_memory(map, len, ...) hmc_memory_map_get_mapped_memory_((map), (len), (HMCMapProps){.readable=1, .writable=1, .executable=0, .private=1, ##__VA_ARGS__})

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_init(HMCMemoryMap **map) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_deinit(HMCMemoryMap **map) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_obtain_ptr(HMCMemoryMap *map, void **mem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_release_ptr(HMCMemoryMap *map, void **mem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_get_mapped_memory_(HMCMemoryMap *map, hsize_t len, HMCMapProps props) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_memory_map_release_mapped_memory(HMCMemoryMap *map) HMC_ATTR_NO_NULL;

#endif
