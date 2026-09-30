#ifndef _HMC_LOCK_
#define _HMC_LOCK_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_results.h>

typedef struct HMCMutexLock HMCMutexLock;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_mutex_init(HMCMutexLock* lock) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_mutex_deinit(HMCMutexLock *lock) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_mutex_lock(HMCMutexLock *lock) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_mutex_unlock(HMCMutexLock *lock) HMC_ATTR_NO_NULL;

#endif
