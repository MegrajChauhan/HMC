#ifndef _HMC_STATIC_ARRAY_
#define _HMC_STATIC_ARRAY_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_results.h>

typedef struct HMCStaticArray HMCStaticArray;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_init(HMCStaticArray **array, HMCAllocator **allocator, hsize_t elen, hsize_t nelem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_deinit(HMCStaticArray **array, HMCAllocator **allocator) HMC_ATTR_NO_NULL_SPECIFY(1);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_keep_allocator(HMCStaticArray *array) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_insert(HMCStaticArray *array, void *elem, hsize_t index) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_push(HMCStaticArray *array, void *elem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_pop(HMCStaticArray *array, void *elem) HMC_ATTR_NO_NULL_SPECIFY(1);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_remove(HMCStaticArray *array, void *elem, hsize_t index) HMC_ATTR_NO_NULL_SPECIFY(1);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_at(HMCStaticArray *array, void *elem, hsize_t index) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_size(HMCStaticArray *array, hsize_t *size) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_static_array_clear(HMCStaticArray *array) HMC_ATTR_NO_NULL;

#endif
