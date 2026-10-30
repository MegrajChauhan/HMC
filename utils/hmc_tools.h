#ifndef _HMC_TOOLS_
#define _HMC_TOOLS_

#include <hmc_base_defn.h>
#include <stdlib.h>
#include <stdio.h>

#define _hmc_assert(cond, msg) \
  do {\
    if (!(cond)) {\
      fprintf(stderr, "[Assertion Failed]:%s:%d: The condition (%s) failed (%s)\n", __FILE__, __LINE__, _hmc_stringify((cond)), (msg));\
      exit(-1);\
    } \
  } while(0)
#define _hmc_check(expr, val) \
  do {\
    __auto_type _hmc_check_value_res = (expr);\
    if ((_hmc_check_value_res) != (val)) \
      _hmc_assert(((expr) == (val)), "values do not match");\
  } while (0)
#define _hmc_die(msg, EXIT_CODE, ...) \
  do {\
    fprintf(stderr, msg"\n", ##__VA_ARGS__);\
    exit(EXIT_CODE);\
  } while(0)
#define _hmc_unreachable() _hmc_die("Unreachable code:%s:%d",100, __FILE__, __LINE__)

#ifndef _HMC_BUILD_RELEASE_
#define _hmc_assert_dbg(cond, msg) _hmc_assert(cond, msg)
#define _hmc_check_dbg(expr, val) _hmc_check(expr, val)
#else
#define _hmc_assert_dbg(cond, msg)
#define _hmc_check_dbg(expr, val)
#endif

#endif
