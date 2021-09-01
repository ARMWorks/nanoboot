/* SPDX-License-Identifier: GPL-2.0 */

#pragma once

#include <stdint.h>

typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;

typedef __u8 u8;
typedef __u16 u16;
typedef __u32 u32;
typedef __u64 u64;

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
#if defined(__GNUC__)
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
#endif
typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;

/*
 * A dma_addr_t can hold any valid DMA address, i.e., any address returned
 * by the DMA API.
 *
 * If the DMA API only uses 32-bit addresses, dma_addr_t need only be 32
 * bits wide.  Bus addresses, e.g., PCI BARs, may be wider than 32 bits,
 * but drivers do memory-mapped I/O to ioremapped kernel virtual addresses,
 * so they don't care about the size of the actual bus addresses.
 */
#ifdef __aarch64__
typedef u64 dma_addr_t;
#else
typedef u32 dma_addr_t;
#endif