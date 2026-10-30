#ifndef _HMC_CHRONO_
#define _HMC_CHRONO_

#include <hmc_base_defn.h>
#include <hmc_base_types.h>
#include <hmc_results.h>

typedef struct HMCClock HMCClock;
typedef struct HMCTime HMCTime;
typedef struct HMCPreciseTime HMCPreciseTime;

typedef enum hclock_t {
  HMC_CLOCK_SYSTEM_WIDE, 
  HMC_CLOCK_MONOTONIC,  
  HMC_CLOCK_BOOTTIME,   
  HMC_CLOCK_PROCESS,    
  HMC_CLOCK_THREAD,     
} hclock_t;

struct HMCPreciseTime {
  hsize_t t_seconds;
  hsize_t t_nanoseconds;
};

struct HMCTime {
  u8_t sec;
  u8_t min;
  u8_t hour;
  u8_t week_day;
  u8_t month_day;
  u8_t month;
  u16_t year; 
};

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_clock_init(HMCClock **clk, hclock_t type);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_clock_deinit(HMCClock **clk);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_clock_time(HMCClock *clk);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_clock_get_elapsed_since_start(HMCClock *clk, HMCPreciseTime *tp);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_clock_get_elapsed_since_last_check(HMCClock *clk, HMCPreciseTime *tp);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_clock_sleep(HMCClock *clk, HMCPreciseTime *td, HMCPreciseTime *rd);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_clock_nanosleep(HMCClock *clk, HMCPreciseTime *td, HMCPreciseTime *rd);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_clock_sleep_abs(HMCClock *clk, HMCPreciseTime *td, HMCPreciseTime *rd);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_clock_nanosleep_abs(HMCClock *clk, HMCPreciseTime *td, HMCPreciseTime *rd);

HMC_ATTR_EXTERNAL 
hsize_t hmc_time_buf_length_for_string();

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_time_get_epoch(hepoch_t *res);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_time_get_time(HMCTime* res);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_time_from_epoch(HMCTime *res, hepoch_t epoch);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_time_to_epoch(HMCTime *t, hepoch_t *ep);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_time_to_string_from_epoch(hepoch_t epoch, char **buf);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_NULL 
hmcResult_t hmc_time_to_string_from_time(HMCTime *tm, char **buf);

#endif
