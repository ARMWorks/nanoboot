#ifndef _ARCH_HARDWARE_H_
#define _ARCH_HARDWARE_H_

#include <asm/sizes.h>

#ifndef __ASSEMBLY__
#define UData(Data) ((unsigned long) (Data))

#define __REG(x) (*(volatile unsigned long *)(x))
#define __REGl(x) (*(volatile unsigned long *)(x))
#define __REGw(x) (*(volatile unsigned short *)(x))
#define __REGb(x) (*(volatile unsigned char *)(x))
#define __REG2(x,y) (*(volatile unsigned long *)((x) + (y)))
#else
#define UData(Data) (Data)

#define __REG(x) (x)
#define __REGl(x) (x)
#define __REGw(x) (x)
#define __REGb(x) (x)
#define __REG2(x,y) ((x) + (y))
#endif

#define Fld(Size, Shft) (((Size) << 16) + (Shft))

#define FSize(Field) ((Field) >> 16)
#define FShft(Field) ((Field) & 0x0000FFFF)
#define FMsk(Field) (((UData (1) << FSize (Field)) - 1) << FShft (Field))
#define FAlnMsk(Field) ((UData (1) << FSize (Field)) - 1)
#define F1stBit(Field) (UData (1) << FShft (Field))

#define FClrBit(Data, Bit) (Data = (Data & ~(Bit)))
#define FClrFld(Data, Field) (Data = (Data & ~FMsk(Field)))

#define FInsrt(Value, Field) \
                 (UData (Value) << FShft (Field))

#define FExtr(Data, Field) \
                 ((UData (Data) >> FShft (Field)) & FAlnMsk (Field))

#endif /* _ARCH_HARDWARE_H_ */
