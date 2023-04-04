/*---------------------------------------------------------------------------------
    overly simple ARM7 code for dumping firmware from flash chip
---------------------------------------------------------------------------------*/
#include <nds.h>

#include <nds/bios.h>
#include "dump.h"

#include "../../common/fifoChannels.h"

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------
	readUserSettings();

    irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();
    fifoInit();
	installSystemFIFO();
    irqSet(IRQ_VBLANK, VblankHandler);
    irqEnable(IRQ_VBLANK);

    fifoSendValue32(FIFO_RETURN, 1); // notify ARM9 that things ready

    // Keep the ARM7 out of main RAM
    while (1)
    {
        swiWaitForVBlank();

        if(fifoCheckValue32(FIFO_CONTROL)) {
            u32 dumpOption = fifoGetValue32(FIFO_CONTROL);
            u32 ret = 0;
            u32 mailAddr = fifoGetValue32(FIFO_BUFFER_ADDR);
            if(dumpOption == DSBF_DUMP_BIOS7) {
                arm7dump((u8 *)mailAddr);
                ret = 16384;
            } else if (dumpOption == DSBF_EXIT)
                // just exit, not like anything is alloc'd here
                return 0;
            fifoSendValue32(FIFO_RETURN, ret);
        }
    }
}
