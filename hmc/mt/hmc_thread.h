#ifndef _HMC_THREADS_
#define _HMC_THREADS_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_lock.h>
#include <hmc_results.h>

typedef struct HMCThread HMCThread;
typedef void* (*hmcthreadexec_t)(void *);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_thread_init(HMCThread **thread);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_thread_deinit(HMCThread **thread);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_thread_launch(HMCThread *thread, hmcthreadexec_t func, void *arg);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_thread_launch_detached(HMCThread *thread, hmcthreadexec_t func, void *arg);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_thread_join(HMCThread *thread);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD hmcResult_t hmc_thread_get_result(HMCThread *thread, void **res);

#endif
