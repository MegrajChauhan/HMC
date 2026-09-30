#ifndef _HMC_BASE_DEFS_
#define _HMC_BASE_DEFS_

#include <ctype.h>
#include <hmc_config.h>

#ifndef _hmc_surelyT
#define _hmc_surelyT(x)                                                        \
  __builtin_expect(                                                            \
      !!(x),                                                                   \
      1) // tell the compiler that the expression x is most likely to be true
#define _hmc_surelyF(x)                                                        \
  __builtin_expect(                                                            \
      !!(x),                                                                   \
      0) // tell the compiler that the expression x is most likely to be false
#endif

#if defined(_HMC_OPTIMIZE_)
#define HMC_ATTR_ALWAYS_INLINE static inline
#else
#define HMC_ATTR_ALWAYS_INLINE static inline __attribute__((always_inline))
#endif

#define HMC_ATTR_NO_DISCARD __attribute__((nodiscard))
#define HMC_ATTR_NO_THROW __attribute__((no_throw))
#define HMC_ATTR_NO_RETURN __attribute__((no_return))
#define HMC_ATTR_NO_NULL __attribute__((nonnull))
#define HMC_ATTR_NO_NULL_SPECIFY(...) __attribute__((nonnull(__VA_ARGS__)))
#define HMC_ATTR_ALIAS(name) __attribute__((alias(#name)))
#define HMC_ATTR_CONSTRUCTOR __attribute__((constructor))

#ifdef _USE_LINUX_
#define _HMC_ATTR_EXPORT_ __attribute__((visibility("default")))
#endif

#define HMC_ATTR_INTERNAL                                                      \
  static // for a variable or a function that is localized to a module only
#define HMC_ATTR_LOCAL static // any static variable inside a function
#define HMC_ATTR_EXTERNAL extern
#define HMC_ATTR_THREAD_LOCAL _Thread_local

#define _hmc_stringify(x) #x
#define _hmc_concat(x, y) x##y
#define _hmc_toBool(x) !!(x)

#define _hmc_signExtend8(val)                                                  \
  do {                                                                         \
    if ((val >> 7) == 1)                                                       \
      val |= 0xFFFFFFFFFFFFFF00;                                               \
  } while (0)
#define _hmc_signExtend16(val)                                                 \
  do {                                                                         \
    if ((val >> 15) == 1)                                                      \
      val |= 0xFFFFFFFFFFFF0000;                                               \
  } while (0)
#define _hmc_signExtend32(val)                                                 \
  do {                                                                         \
    if ((val >> 31) == 1)                                                      \
      val |= 0xFFFFFFFFFF000000;                                               \
  } while (0)

#define _hmc_isUpperCase(ch) ((ch) >= 'A' && (ch) <= 'Z')
#define _hmc_isLowerCase(ch) ((ch) >= 'a' && (ch) <= 'z')
#define _hmc_isAlpha(ch) (_hmc_isLowerCase(ch) || _hmc_isUpperCase(ch))
#define _hmc_isNum(ch) (((ch) >= '0' && (ch) <= '9'))
#define _hmc_isAlphaNum(ch) (_hmc_isAlpha(ch) || _hmc_isNum(ch))
#define _hmc_isSpace(ch) isspace(ch)

#define _hmc_alignTo(num, alignment)                                           \
  ((((num) + (alignment) - 1) / (alignment)) * (alignment))

#endif
