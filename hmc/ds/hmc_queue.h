#ifndef _HMC_QUEUE_
#define _HMC_QUEUE_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_allocator.h>
#include <hmc_results.h>

typedef struct HMCQueue HMCQueue;

HMC_ATTR_EXTERNAL hmcResult_t hmc_queue_init(HMCQueue **queue, HMCAllocator **allocator,  hsize_t elen, hsize_t nelem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_queue_deinit(HMCQueue **queue, HMCAllocator **allocator) HMC_ATTR_NO_NULL_SPECIFY(1);

HMC_ATTR_EXTERNAL hmcResult_t hmc_queue_keep_allocator(HMCQueue *queue) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_queue_enqueue(HMCQueue *queue, void *elem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_queue_dequeue(HMCQueue *queue, void *elem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_queue_top(HMCQueue *queue, void *elem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_queue_bottom(HMCQueue *queue, void *elem) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_queue_size(HMCQueue *queue, hsize_t *size) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL hmcResult_t hmc_queue_clear(HMCQueue *queue) HMC_ATTR_NO_NULL;

#endif
