/* SPDX-License-Identifier: GPL-2.0 */

#pragma once

#include <stdbool.h>


#define MIN(a, b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a < _b ? _a : _b; })

#define MAX(a, b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a > _b ? _a : _b; })

#define DIV_ROUND_UP(n, d)   (((n) + (d) - 1) / (d))

#define ROUND_UP(a, b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      ((_a - 1) | (_b - 1)) + 1; })

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type, member) );})
