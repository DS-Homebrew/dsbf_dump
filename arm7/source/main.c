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
	initClockIRQ();
	fifoInit();
	installSystemFIFO();
	irqSet(IRQ_VBLANK, VblankHandler);
	irqEnable(IRQ_VBLANK);

	fifoSendValue32(FIFO_RETURN, 1); // notify ARM9 that things ready

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
