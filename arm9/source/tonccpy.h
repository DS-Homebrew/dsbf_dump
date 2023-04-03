/*
	Copyright 2005-2009 J Vijn

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
	the Software, and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
	IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//# Stuff you may not have yet.

#ifndef TONCCPY_H
#define TONCCPY_H


#ifdef __cplusplus
extern "C" {
#endif

#include <nds/ndstypes.h>

typedef unsigned int uint;
#define BIT_MASK(len)       ( (1<<(len))-1 )
static inline u32 quad8(u16 x)   {   x |= x<<8; return x | x<<16;    }


//# Declarations and inlines.

void tonccpy(void *dst, const void *src, uint size);

void __toncset(void *dst, u32 fill, uint size);
static inline void toncset(void *dst, u8 src, uint size);
static inline void toncset16(void *dst, u16 src, uint size);
static inline void toncset32(void *dst, u32 src, uint size);


//! VRAM-safe memset, byte version. Size in bytes.
static inline void toncset(void *dst, u8 src, uint size)
{   __toncset(dst, quad8(src), size);               }

//! VRAM-safe memset, halfword version. Size in hwords.
static inline void toncset16(void *dst, u16 src, uint size)
{   __toncset(dst, src|src<<16, size*2);            }

//! VRAM-safe memset, word version. Size in words.
static inline void toncset32(void *dst, u32 src, uint size)
{   __toncset(dst, src, size*4);                    }

#ifdef __cplusplus
}
#endif
#endif
