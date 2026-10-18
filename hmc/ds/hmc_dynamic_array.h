#ifndef _HMC_DYNAMIC_ARRAY_
#define _HMC_DYNAMIC_ARRAY_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_allocator.h>
#include <hmc_results.h>

typedef struct HMCDynamicArray HMCDynamicArray;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_init(HMCDynamicArray **array, HMCAllocator **allocator, hsize_t elen, hsize_t nelem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_deinit(HMCDynamicArray **array, HMCAllocator **allocator) HMC_ATTR_NO_NULL_SPECIFY(1);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_keep_allocator(HMCDynamicArray *array) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_resize(HMCDynamicArray *array, hsize_t new_nelen) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_insert(HMCDynamicArray *array, void *elem, hsize_t index) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_push(HMCDynamicArray *array, void *elem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_pop(HMCDynamicArray *array, void *elem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_remove(HMCDynamicArray *array, void *elem, hsize_t index) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_at(HMCDynamicArray *array, void *elem, hsize_t index) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_size(HMCDynamicArray *array, hsize_t *size) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_dynamic_array_clear(HMCDynamicArray *array) HMC_ATTR_NO_NULL;

#endif
