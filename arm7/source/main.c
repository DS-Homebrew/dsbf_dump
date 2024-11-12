/*
	default ARM7 core

	Copyright (C) 2005 - 2010
	Michael Noland (joat)
	Jason Rogers (dovoto)
	Dave Murphy (WinterMute)

	SPDX-License-Identifier: Zlib
	SPDX-FileNotice: Modified to allow ARM9 to read the ARM7 BIOS and firmware.
*/

#include <calico.h>
#include <nds.h>

#include "DSBF_Threads.h"

void readBios(u8* dest, u32 src, u32 size);

static Thread DSBF_Thread7;
alignas(8) static u8 DSBF_Thread7Stack[1024];

static inline DSBF_ThreadMsgType DSBF_ThreadGetType(u32 msg)
{
	return (DSBF_ThreadMsgType)(msg & 0xff);
}

static int DSBF_ThreadMain(void* arg) {
	// Set up PXI mailbox, used to receive PXI command words
	Mailbox DSBF_PxiControlMailbox;
	u32 DSBF_PxiControlMailboxData[8];
	mailboxPrepare(&DSBF_PxiControlMailbox, DSBF_PxiControlMailboxData, sizeof(DSBF_PxiControlMailboxData)/sizeof(u32));
	pxiSetMailbox(DSBF_PXI_CONTROL, &DSBF_PxiControlMailbox);

	// Main PXI message loop
	for (;;) {
		// Receive a message
		u32 msg = mailboxRecv(&DSBF_PxiControlMailbox);
		DSBF_ThreadMsgType type = DSBF_ThreadGetType(msg);
		// default to EINVAL in case of invalid commands
		u32 rc = 1;
		switch (type) {
			// DSBF_DUMP_JEDEC: Read the firmware chip's JEDEC identifier
			case DSBF_DUMP_JEDEC: {
				spiLock();
				nvramReadJedec(&rc);
				spiUnlock();

				break;
			}

			// DSBF_DUMP_BIOS7: Read the ARM7 BIOS
			case DSBF_DUMP_BIOS7: {
				u8* bufferAddr = (u8*)mailboxRecv(&DSBF_PxiControlMailbox);
				u32 bufferSize = mailboxRecv(&DSBF_PxiControlMailbox);
				readBios(bufferAddr, 0, bufferSize);
				rc = 0;
				break;
			}
			default:
				break;
		}

		// Send a reply back to the ARM9
		pxiReply(DSBF_PXI_CONTROL, rc);
	}

	return 0;
}

int main() {
	// Read settings from NVRAM
	envReadNvramSettings();

	// Set up extended keypad server (X/Y/hinge)
	keypadStartExtServer();

	// Configure and enable VBlank interrupt
	lcdSetIrqMask(DISPSTAT_IE_ALL, DISPSTAT_IE_VBLANK);
	irqEnable(IRQ_VBLANK);

	// Set up RTC
	rtcInit();
	rtcSyncTime();

	// Initialize power management
	pmInit();

	// Set up block device peripherals
	blkInit();

	// Set up server thread
	threadPrepare(&DSBF_Thread7, DSBF_ThreadMain, NULL, &DSBF_Thread7Stack[sizeof(DSBF_Thread7Stack)], MAIN_THREAD_PRIO);
	threadStart(&DSBF_Thread7);

	// Keep the ARM7 mostly idle
	while (pmMainLoop()) {
		threadWaitForVBlank();
	}

	return 0;
}
