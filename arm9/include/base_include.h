#ifndef BASE_INCLUDE__
#define BASE_INCLUDE__

#ifdef __GBA__

// GBA defines and all
#include <gba.h>
#include "useful_qualifiers.h"
#define SCANLINES 0xE4
#define ROM 0x8000000

#define REG_WAITCNT *(vu16*)(REG_BASE + 0x204) // Wait state Control
#define SRAM_READING_VALID_WAITCYCLES 3
#define SRAM_ACCESS_CYCLES 9
#define NON_SRAM_MASK 0xFFFC
#define BASE_WAITCNT_VAL 0x4314
#define CLOCK_SPEED 16777216

#endif

#ifdef __NDS__

// NDS defines and all
#include <nds.h>
#include "useful_qualifiers.h"
#define SCANLINES 0x107
#define ROM GBAROM

#define REG_WAITCNT REG_EXMEMCNT // Wait state Control
#define SRAM_READING_VALID_WAITCYCLES 3
#define SRAM_ACCESS_CYCLES (2 * 18)
#define NON_SRAM_MASK 0xFFFC
#define BASE_WAITCNT_VAL 0x0014
#define ARM9_CLOCK_SPEED 67108864
#define ARM7_CLOCK_SPEED 33554432
#define ARM7_GBA_CLOCK_SPEED 16777216
#define CLOCK_SPEED ARM9_CLOCK_SPEED

#endif

#define VBLANK_SCANLINES SCREEN_HEIGHT

#endif
