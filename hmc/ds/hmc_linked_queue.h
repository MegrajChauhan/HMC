#ifndef _HMC_LINKED_QUEUE_
#define _HMC_LINKED_QUEUE_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_allocator.h>
#include <hmc_results.h>

typedef struct HMCLinkedQueue HMCLinkedQueue;

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_linked_queue_init(HMCLinkedQueue **queue, HMCAllocator **allocator,  hsize_t elen);

HMC_ATTR_NO_NULL_SPECIFY(1) HMC_ATTR_EXTERNAL 
hmcResult_t hmc_linked_queue_deinit(HMCLinkedQueue **queue, HMCAllocator **allocator);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_linked_queue_keep_allocator(HMCLinkedQueue *queue);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_linked_queue_enqueue(HMCLinkedQueue *queue, void *elem);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_linked_queue_dequeue(HMCLinkedQueue *queue, void *elem);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_linked_queue_top(HMCLinkedQueue *queue, void *elem);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_linked_queue_bottom(HMCLinkedQueue *queue, void *elem);

HMC_ATTR_NO_NULL HMC_ATTR_EXTERNAL 
hmcResult_t hmc_linked_queue_size(HMCLinkedQueue *queue, hsize_t *size);

#endif
