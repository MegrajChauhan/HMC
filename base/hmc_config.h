#ifndef _HMC_CONFIG_
#define _HMC_CONFIG_

#define _HMC_HOST_ARCH_LITTLE_ENDIAN_ 0x00
#define _HMC_HOST_ARCH_BIG_ENDIAN_ 0x01

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _HMC_HOST_ARCH_BYTE_ORDER_ _HMC_HOST_ARCH_LITTLE_ENDIAN_
#else
#define _HMC_HOST_ARCH_BYTE_ORDER_ _HMC_HOST_ARCH_BIG_ENDIAN_
#endif

#define _HMC_HOST_ARCH_ENDIANNESS_ _HMC_HOST_ARCH_BYTE_ORDER_

#if defined(__amd64) || defined(__amd64__)
#define _HMC_HOST_CPU_AMD_
#define _HMC_HOST_ARCH_x86_
#endif

#if defined(__x86_64)
#define _HMC_HOST_ARCH_x86_
#endif

#if defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#define _HMC_HOST_PLATFORM_LINUX_
#define _USE_LINUX_
#ifndef __USE_MISC // for STDLIB
#define __USE_MISC
#endif
#endif

#if __SIZEOF_VOID__ == 4
// This is a 32-bit system
#error HMC Currently has no idea about 32-bit systems
#endif

#if __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
#define _HMC_TYPE_LONG_ long
#else
#define _HMC_TYPE_LONG_ long long
#endif

#endif
