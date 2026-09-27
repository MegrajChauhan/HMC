#ifndef _HMC_BASE_PLATFORM_
#define _HMC_BASE_PLATFORM_

#include <hmc_base_types.h>
#include <hmc_config.h>

#ifdef _HMC_HOST_PLATFORM_LINUX_
#include <dlfcn.h>
#include <fcntl.h>
#include <liburing.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef pthread_t _hmc_thread_t;
typedef pthread_mutex_t _hmc_mutex_t;
typedef pthread_cond_t _hmc_cond_t;
typedef int _hmc_fd_t;
typedef void *_hmc_pgd_t;

#else
// others....
#endif

#endif
