#ifndef _HMC_BASE_TYPES_
#define _HMC_BASE_TYPES_

#include <hmc_config.h>

#ifdef __cplusplus
#include <atomic>
#endif

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;
typedef unsigned _HMC_TYPE_LONG_
    u64_t; // currently we have no 32-bit only support
           // and i don't think we will have one

typedef signed char i8_t;
typedef short i16_t;
typedef int i32_t;
typedef _HMC_TYPE_LONG_ i64_t;

typedef u64_t hsize_t;
typedef i64_t hepoch_t;

typedef u32_t hwidechar_t;
typedef u32_t hcodepoint_t;

#ifndef __cplusplus
typedef _Atomic hsize_t atm_hsize_t;
typedef _Atomic u8_t atm_u8_t;
typedef _Atomic u16_t atm_u16_t;
typedef _Atomic u32_t atm_u32_t;
typedef _Atomic u64_t atm_u64_t;

typedef _Atomic i8_t atm_i8_t;
typedef _Atomic i16_t atm_i16_t;
typedef _Atomic i32_t atm_i32_t;
typedef _Atomic i64_t atm_i64_t;
#else
typedef std::atomic<hsize_t> atm_hsize_t;
typedef std::atomic<u8_t> atm_u8_t;
typedef std::atomic<u16_t> atm_u16_t;
typedef std::atomic<u32_t> atm_u32_t;
typedef std::atomic<u64_t> atm_u64_t;

typedef std::atomic<i8_t> atm_i8_t;
typedef std::atomic<i16_t> atm_i16_t;
typedef std::atomic<i32_t> atm_i32_t;
typedef std::atomic<i64_t> atm_i64_t;
#endif

typedef char *string_t;
typedef const char *cstring_t;
typedef u8_t* byteptr_t;

#endif
