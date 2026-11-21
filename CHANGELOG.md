# Changelog

All notable changes to HMC are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0] - 2026-11-19

First release.

### Added

- Memory
  - Global allocator
  - `HMCLinearAllocator`: linear allocator
  - `HMCPoolAllocator`: pool allocator
  - `HMCStackAllocator`: stack allocator
  - `HMCMemoryMap`: memory mapping
- Containers
  - `HMCDynamicArray`: dynamic array
  - `HMCStaticArray`: static array
  - `HMCStack`: static linear stack
  - `HMCQueue`: static linear queue
  - `HMCLinkedQueue`: dynamic linked list queue
  - `HMCLFStack`: lock-free linear stack
- I/O
  - `HMCBuffer`: IO buffer
  - `HMCFile`: non-buffered file IO
  - `HMCFileReader`, `HMCFileWriter`: buffered file IO
  - `HMCInputStream`, `HMCOutputStream`: generic non-buffered IO streams
  - `HMCBufferedInputStream`, `HMCBufferedOutputStream`: generic buffered IO streams
- Text
  - UTF-8 support
- Concurrency
  - `HMCThread`: multi-threading
  - `HMCConditionVariable`, `HMCMutexLock`, `HMCSema`: synchronization primitives
- Time
  - `HMCClock`: various clocks, with the supporting structures `HMCPreciseTime` and `HMCTime`
- Random numbers
  - `HMCmt19937`: Mersenne Twister pseudo-random number generator
  - System-provided random number generation
  - `HMCUniformIntDistribution`, `HMCUniformRealDistribution`: uniform integer and real number distributions
