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
    fifoInit();
    irqSet(IRQ_VBLANK, VblankHandler);
    irqEnable(IRQ_VBLANK);
	installSystemFIFO();

    fifoSendValue32(FIFO_CONTROL, 1); // notify ARM9 that things ready

    // Keep the ARM7 out of main RAM
    while (1)
    {
        swiWaitForVBlank();

        if(fifoCheckValue32(FIFO_CONTROL)) {
            u32 dumpOption = fifoGetValue32(FIFO_CONTROL);
            u32 ret = 0;
            u32 mailAddr = fifoGetValue32(FIFO_BUFFER_ADDR);
            u32 mailSize = fifoGetValue32(FIFO_BUFFER_SIZE);
            extern u32 DumpFirmware(u8 *buf, u32 max_size);
            if(dumpOption == 0xF1)
                ret = DumpFirmware((u8 *)mailAddr, mailSize);
            else if(dumpOption == 0xF2) {
                arm7dump((u8 *)mailAddr);
                ret = 16384;
            }
            fifoSendValue32(FIFO_RETURN, ret);
        }
    }
}
