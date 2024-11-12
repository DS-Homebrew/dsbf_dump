/*
	NDS BIOS/firmware dumper

	Copyright (C) 2016 Eric Biggers (crc32_gzip())
	Copyright (C) 2023 DS-Homebrew

	SPDX-License-Identifier: MIT
*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <calico/arm/cache.h>
#include <calico/nds/env.h>
#include <nds.h>
#include <fat.h>

#include "DSBF_Threads.h"
#include "serial_flash.h"

PrintConsole topScreen;
PrintConsole bottomScreen;

static bool isNitroUnit = false;

static u32 crc32_gzip(const u8 *p, size_t len)
{
	u32 crc = ~0;
	const u32 divisor = 0xEDB88320;

	for (size_t i = 0; i < len * 8; i++) {
		u32 multiple;

		int bit = (p[i / 8] >> (i % 8)) & 1;
		crc ^= bit;
		if (crc & 1)
			multiple = divisor;
		else
			multiple = 0;
		crc >>= 1;
		crc ^= multiple;
	}

	return ~crc;
}

void printBiosCRC32(u8* buffer, u32 size) {
	// size == 0x4000: BIOS7
	// size == 0x1000: BIOS9
	// else == ???
	if (size != 0x1000 && size != 0x4000) return;

	u32 checksum = crc32_gzip(buffer, size);

	consoleSelect(&bottomScreen);

	if(size == 0x4000)
		printf("BIOS7 CRC32: %08lX\n\n", checksum);
	else if(size == 0x1000)
		printf("BIOS9 CRC32: %08lX\n\n", checksum);

	consoleSelect(&topScreen);
}

#define BUFFER_SIZE 1048576

// get flash chip info
int get_fw_info(u8* buffer, u32 size) {
	u32 jedecIdU32 = 0;
	if(size < 3)
		return -ENOMEM;

	printf("Call ARM7 to read flashchip info\n\n");
	jedecIdU32 = pxiSendAndReceive(DSBF_PXI_CONTROL, DSBF_DUMP_JEDEC);
	buffer[0] = (u8)((jedecIdU32 >> 16) & 0xFF);
	buffer[1] = (u8)((jedecIdU32 >> 8)  & 0xFF);
	buffer[2] = (u8)((jedecIdU32)       & 0xFF);
	return 0;
}

// dump DS BIOS9
// no point in adding return value as BIOS is always 4KiB
int dump_arm9(u8* buffer, u32 size) {
	printf("Dumping BIOS9\n");

	if(buffer == NULL || BUFFER_SIZE < size)
		return -ENOMEM;

	memcpy((void*)buffer, (void*)0xFFFF0000, size);
	printBiosCRC32(buffer, size);
	return 0;
}

// dump DS BIOS7
// no point in adding return value as BIOS is always 16KiB
int dump_arm7(u8* buffer, u32 size) {
	int rc = 0;

	printf("Call ARM7 to dump BIOS7\n");

	if(buffer == NULL || BUFFER_SIZE < size)
		return -ENOMEM;

	u32 pxiParams[2] = {
		(u32)buffer,
		size,
	};
	armDCacheFlush((void*)buffer, BUFFER_SIZE);
	rc = pxiSendWithDataAndReceive(DSBF_PXI_CONTROL, DSBF_DUMP_BIOS7, pxiParams, sizeof(pxiParams)/sizeof(u32));
	if(rc != 0)
		return rc;

	printBiosCRC32(buffer, size);

	return 0;
}

// dump DS firmware
int dump_firmware(u8* buffer, u32 size) {
	printf("Call ARM7 to dump FW\n");

	if(buffer == NULL || BUFFER_SIZE < size)
		return -ENOMEM;

	pmReadNvram(buffer, 0, size);
	return 0;
}

bool save_dump(char* path, u8* buffer, u32 size) {
	if (!isNitroUnit) {
		printf("Saving %s\n", path+16);
		FILE* f = fopen(path, "wb");
		if(!f) {
			printf("Failed to open %s\n", path);
			return false;
		}
		fseek(f, 0, SEEK_SET);
		fwrite(buffer, size, 1, f);
		fflush(f);
		fclose(f);
		return true;
	} else {
		// If we're on a nitro unit, write the values into memory so they can be
		// fetched from the host machine, through is-nitro-debugger or a custom
		// tool.
		//
		// We fill the area around the buffer with very recognizable values, so
		// they can be very easily searched for.
		u8* data = malloc(size + 512);
		memset(data, 0xDE, 256);
		memset(data+256+size, 0xAD, 256);
		memcpy(data+256, buffer, size);
		printf("buffer at: %p\n", data);
		printf("If manually dumping press START\n");
		printf("to continue, otherwise please\n");
		printf("hold.\n");
		while (true) {
			swiWaitForVBlank();
			scanKeys();
			// User has manually requested we continue.
			if (keysDown() & KEY_START) break;
			// Tools can set one of the start bytes to 0xFF in order to automatically
			// continue. They don't necissarily have to find the start of the buffer,
			// just any byte of the buffer is fine.
			bool foundToolSigil = false;
			for (u16 idx = 0; idx < 256; ++idx) {
				if (*(data + idx) == 0xFF) {
					foundToolSigil = true;
					break;
				}
			}
			if (foundToolSigil) break;
		}
		// Set data back to 0, just to be extra sure it wouldn't be confused for
		// any future dumps.
		memset(data, 0x00, size + 512);
		free(data);
		return true;
	}
}

void printAdditionalFWInfo(u8* buffer) {
	firmware_header_t* fwHeader = (firmware_header_t*)malloc(sizeof(firmware_header_t));
	memcpy(fwHeader, buffer, sizeof(firmware_header_t));

	consoleSelect(&bottomScreen);

	printf("FW build date: 20%02X-%02X-%02X %02X:%02X\n\n", fwHeader->buildYear, fwHeader->buildMonth, fwHeader->buildDay, fwHeader->buildHour, fwHeader->buildMinute);

	printf("Device type: 0x%02X", fwHeader->deviceType);
	switch(fwHeader->deviceType) {
		case DEVICE_TYPE_NDSP_PROTO:
			printf(", DS Phat\n(Prototype)");
			break;
		case DEVICE_TYPE_NDSL_KIOSK:
			printf(", DS Lite\n(Kiosk)");
			break;
		case DEVICE_TYPE_NDSL:
			printf(", DS Lite\n(normal)");
			break;
		case DEVICE_TYPE_NDSL_KOR:
			printf(", DS Lite (KOR)");
			break;
		case DEVICE_TYPE_IQUE:
			printf(", iQueDS");
			break;
		case DEVICE_TYPE_NDSI:
			printf(", DSi-mode");
			break;
		case DEVICE_TYPE_IQUEL:
			printf(", iQueDS Lite");
			break;
		case DEVICE_TYPE_NDSP:
			printf(", DS Phat");
			break;
		default:
			printf("\nWARNING: this device has an\nunknown firmware version");
			break;
	}
	printf("\n\n");

	consoleSelect(&topScreen);
	free(fwHeader);
}

// the firmware size is the third byte in the JEDEC read.
// the size is log2 of the chip size.
// or it should be, but there does exist some quirky chips.
u32 get_fw_size(u8* jedecId) {
	// DSi / 3DS are 128KB
	if(g_envExtraInfo->nvram_console_type == EnvConsoleType_DSi) return 1 << 0x11;

	// Check 512KB chip quirks
	for(int i = 0; i < sizeof(flash_chip_quirk_512); i++) {
		if (memcmp(jedecId, flash_chip_quirk_512[i], 2) == 0) return 1 << 0x13;
	}

	// Check 256KB chip quirks
	for(int i = 0; i < sizeof(flash_chip_quirk_256); i++) {
		if (memcmp(jedecId, flash_chip_quirk_256[i], 2) == 0) return 1 << 0x12;
	}
	return 1 << jedecId[2];
}

void dump_all(void) {
	u8* buffer = (u8*)memalign(32, BUFFER_SIZE);
	u8 jedecId[3] = {};
	char filename[29] = "/FWXXXXXXXXXXXX"; // uncreative but does the job
	int rc = 0;

	get_fw_info(jedecId, sizeof(jedecId));
	// the firmware size is the third byte in the JEDEC read.
	// the size is log2 of the chip size.
	u32 firmware_size = get_fw_size(jedecId);
	consoleSelect(&bottomScreen);
	printf("JEDEC values: 0x%02X, 0x%02X, 0x%02X\n\n", jedecId[0], jedecId[1], jedecId[2]);
	printf("Firmware size = %ld KB\n\n", firmware_size / 1024);
	consoleSelect(&topScreen);

	memset(buffer, 0, BUFFER_SIZE);
	rc = dump_firmware(buffer, firmware_size);
	if(rc != 0)
	{
		printf("Failed. Error %d\n", rc);
		goto end;
	}

	consoleSelect(&bottomScreen);
	printf("MAC: ");
	for(int i=0; i<6; i++) {
		snprintf(filename + 3 + (i*2), 3, "%02X", buffer[0x36+i]);
		printf("%02X", buffer[0x36+i]);
		if (i < 5) printf(":");
	}
	printf("\n\n");
	consoleSelect(&topScreen);

	printAdditionalFWInfo(buffer);

	// use MAC address as folder name
	if (!isNitroUnit) {
		mkdir(filename, 0777);
		if(access(filename, F_OK) != 0) {
			printf("Failed to create folder for dump.\n");
			goto end;
		}
	}

	memcpy(filename+15, "/firmware.bin\0", 14);
	if(!save_dump(filename, buffer, firmware_size)) {
		printf("Failed to save firmware to file.\n");
		goto end;
	}
	printf("\n\n");

	memset(buffer, 0, BUFFER_SIZE);
	rc = dump_arm7(buffer, 0x4000);
	if(rc != 0)
	{
		printf("Failed. Error %d\n", rc);
		goto end;
	}
	memcpy(filename+16, "biosnds7.rom", 12);
	if(!save_dump(filename, buffer, 0x4000)) {
		printf("Failed to save BIOS7 to file.\n");
		goto end;
	}
	printf("\n\n");

	memset(buffer, 0, BUFFER_SIZE);
	rc = dump_arm9(buffer, 0x1000);
	if(rc != 0)
	{
		printf("Failed. Error %d\n", rc);
		goto end;
	}
	memcpy(filename+16, "biosnds9.rom", 12);
	if(!save_dump(filename, buffer, 0x1000)) {
		printf("Failed to save BIOS9 to file.\n");
		goto end;
	}
	printf("\n\n");

	memset(filename+15, 0, sizeof(filename)-15);
	printf("Done! Files saved to\n%s\n\n", filename);

end:
	free(buffer);
}

int main(int argc, char **argv)
{
	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	consoleInit(&topScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

	consoleSelect(&topScreen);
	printf(" NDS B+F dumper 1.4.0\n");
	printf("=------------------=\n");

	if(!fatInitDefault()) {
		printf("FAT init fail!\n\n");
		printf("If this is an IS-NITRO\n");
		printf("development unit this is\n");
		printf("expected.\n\n");
		printf("Please press the A button to\n");
		printf("continue as a dev-kit.\n");
		printf("Otherwise press start to exit.\n\n");

		while(true) {
			threadWaitForVBlank();
			scanKeys();
			if(keysDown() & KEY_START) return 0;
			if(keysDown() & KEY_A) {
				isNitroUnit = true;
				break;
			}
		}
	}

	if(systemIsTwlMode()) {
		printf("This app only runs in DS-mode.\n");
		goto end;
	}
	dump_all();

end:
	printf("Press START to exit.\n");
	while(pmMainLoop()) {
		threadWaitForVBlank();
		scanKeys();
		if(keysDown() & KEY_START) break;
	}
	return 0;
}
