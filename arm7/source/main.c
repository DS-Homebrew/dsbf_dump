/*---------------------------------------------------------------------------------
    overly simple ARM7 code for dumping firmware from flash chip
---------------------------------------------------------------------------------*/
#include <nds.h>

#include <nds/bios.h>
//#include <nds/arm7/touch.h>
//#include <nds/arm7/clock.h>
#include "dump.h"

#include "../../common/fifoChannels.h"

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

        if(fifoCheckValue32(FIFO_CONTROL)) {
            u32 ret = 0;
            u32 mailAddr = fifoGetValue32(FIFO_BUFFER_ADDR);
            u32 mailSize = fifoGetValue32(FIFO_BUFFER_SIZE);
            extern u32 DumpFirmware(u8 *buf, u32 max_size);
            switch(fifoGetValue32(FIFO_CONTROL)) {
                case 0xF1:
                    ret = DumpFirmware((u8 *)mailAddr, mailSize);
                    break;
                case 0xF2:
                    arm7dump((u8 *)mailAddr);
                    ret = 16384;
                    break;
                default:
                    break;
            }
            if (ret > 0)
                fifoSendValue32(FIFO_RETURN, ret);
        }
    }
}
