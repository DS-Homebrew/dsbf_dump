/*
	NDS BIOS/firmware dumper

	Copyright (C) 2023 DS-Homebrew

	SPDX-License-Identifier: MIT
*/

#include <nds.h>

#include "fifoChannels.h"

void VblankHandler(void) {
}

void readBios(u8* dest, u32 src, u32 size);

int main(void) {
	readUserSettings();
	ledBlink(0);

	irqInit();
	irqSet(IRQ_VBLANK, VblankHandler);
	fifoInit();
	installSystemFIFO();
    initClockIRQTimer(3);
    irqEnable(IRQ_VBLANK);

	u32 isRegularDS = REG_SNDEXTCNT == 0 ? 1 : 0; // If sound frequency setting is found, then the console is not a DS Phat/Lite
	fifoSendValue32(FIFO_RETURN, isRegularDS); // notify ARM9 that things ready

	// Keep the ARM7 out of main RAM
	while(1) {
		swiWaitForVBlank();

		if(fifoCheckValue32(FIFO_CONTROL)) {
			u32 option = fifoGetValue32(FIFO_CONTROL);
			u32 ret = 0;
			u32 mailAddr = fifoGetValue32(FIFO_BUFFER_ADDR);
			u32 mailSize = fifoGetValue32(FIFO_BUFFER_SIZE);
			switch(option) {
				case DSBF_EXIT:
					return 0;
				case DSBF_DUMP_JEDEC:
					readFirmwareJEDEC((void *)mailAddr, mailSize);
					ret = mailSize;
					break;
				case DSBF_DUMP_FW:
					readFirmware(0, (void *)mailAddr, mailSize);
					ret = mailSize;
					break;
				case DSBF_DUMP_BIOS7:
					readBios((void*)mailAddr, 0, mailSize);
					ret = mailSize;
					break;
				default:
					break;
			}
			fifoSendValue32(FIFO_RETURN, ret);
		}
	}
}
