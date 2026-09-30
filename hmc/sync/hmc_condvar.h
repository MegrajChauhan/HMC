#ifndef _HMC_CONDVAR_
#define _HMC_CONDVAR_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_lock.h>
#include <hmc_results.h>

typedef struct HMCConditionVariable HMCConditionVariable;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_condvar_init(HMCConditionVariable* var) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_condvar_deinit(HMCConditionVariable *var) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_condvar_signal(HMCConditionVariable *var) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_condvar_wait(HMCConditionVariable *var, HMCMutexLock *lock) HMC_ATTR_NO_NULL;

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_condvar_broadcast(HMCConditionVariable *var) HMC_ATTR_NO_NULL;

#endif
