/*---------------------------------------------------------------------------------
    overly simple ARM7 code for dumping firmware from flash chip
---------------------------------------------------------------------------------*/
#include <nds.h>

#include <nds/bios.h>
//#include <nds/arm7/touch.h>
//#include <nds/arm7/clock.h>
#include "dump.h"

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------
    // Reset the clock if needed
    rtcReset();

    irqInit();
    irqSet(IRQ_VBLANK, VblankHandler);
    irqEnable(IRQ_VBLANK);

    // Keep the ARM7 out of main RAM
    while (1)
    {
        swiWaitForVBlank();

        if (IPC->mailBusy == 0xF1)
        {
            extern u32 DumpFirmware(u8 *buf, u32 max_size);
            IPC->mailSize = DumpFirmware((u8 *)IPC->mailAddr, IPC->mailSize);
            IPC->mailBusy = 0;
        }
        else if (IPC->mailBusy == 0xF2)
        {
            arm7dump((u8 *)IPC->mailAddr);
            IPC->mailSize = 16384;
            IPC->mailBusy = 0;
        }
    }
}

/*
        if (IPC->mailBusy == 0xF1)
        {
            if (IPC->mailData == 0x00)
            {
                extern u32 DumpFirmware(u8 *buf, u32 max_size);
                IPC->mailSize = DumpFirmware((u8 *)IPC->mailAddr, IPC->mailSize);
                IPC->mailBusy = 0;
            }
            if (IPC->mailData == 0xFF)
            {
                arm7dump((u8 *)IPC->mailAddr, IPC->mailSize);
                IPC->mailSize = 16384;
                IPC->mailBusy = 0;
            }
*/
