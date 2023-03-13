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

        if(fifoCheckValue32(FIFO_USER_02)) {
            u32 ret, mailAddr, mailSize = 0;
            extern u32 DumpFirmware(u8 *buf, u32 max_size);
            switch(fifoGetValue32(FIFO_USER_02)) {
                case 0xF1:
                    mailAddr = fifoGetValue32(FIFO_USER_01);
                    mailSize = fifoGetValue32(FIFO_USER_01);
                    ret = DumpFirmware((u8 *)mailAddr, mailSize);
                    break;
                case 0xF2:
                    arm7dump((u8 *)fifoGetValue32(FIFO_USER_01));
                    ret = 16384;
                    break;
                default:
                    break;
            }
            if (ret)
                fifoSendValue32(FIFO_USER_03, ret);
        }
    }
}
