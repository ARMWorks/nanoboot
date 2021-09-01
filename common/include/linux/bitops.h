/* SPDX-License-Identifier: GPL-2.0 */

#pragma once

#if defined(__aarch64__)
#define BITS_PER_LONG 64
#define BITS_PER_LONG_LONG 64
#else
#define BITS_PER_LONG 32
#define BITS_PER_LONG_LONG 64
#endif

#define BIT(nr)			(1UL << (nr))
#define BIT_ULL(nr)		(1ULL << (nr))
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)	(1ULL << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)	((nr) / BITS_PER_LONG_LONG)
#define BITS_PER_BYTE		8
