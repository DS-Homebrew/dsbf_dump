/*
	NDS BIOS/firmware dumper

	Copyright (C) 2023 DS-Homebrew

	SPDX-License-Identifier: MIT
*/

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <nds.h>
#include <fat.h>

#include "fifoChannels.h"
#include "tonccpy.h"

/*
	Firmware type enum.
	Found at 0x1D of firmware header.
	Here, we use it to truncate the firmware to the correct size.
	A switch case for this can be found in dump_firmware().

	NDSL_2 is an undocumented firmware version. It is unknown what it does different, 
	but it's there, it exists.
*/
enum device_type {
	DEVICE_TYPE_NDSL = 0x20,
	DEVICE_TYPE_NDSL_2 = 0x35,
	DEVICE_TYPE_IQUE = 0x43,
	DEVICE_TYPE_NDSI = 0x57,
	DEVICE_TYPE_IQUEL = 0x63,
	DEVICE_TYPE_NDSP = 0xFF
};

// dump DS BIOS9
// no point in adding return value as BIOS is always 4KiB
void dump_arm9(u8* buffer, u32 size) {
	printf("Dumping BIOS9\n\n");
	tonccpy((void*)buffer, (void*)0xFFFF0000, size);
}

// dump DS BIOS7
// no point in adding return value as BIOS is always 16KiB
void dump_arm7(u8* buffer, u32 size) {
	printf("Call ARM7 to dump BIOS7\n\n");
	fifoSendValue32(FIFO_BUFFER_ADDR, (u32)buffer);
	fifoSendValue32(FIFO_BUFFER_SIZE, size);
	fifoSendValue32(FIFO_CONTROL, DSBF_DUMP_BIOS7);
	fifoWaitValue32(FIFO_RETURN);
	fifoGetValue32(FIFO_RETURN);
}

// dump DS firmware
// return value: size of firmware
u32 dump_firmware(u8* buffer, u32 size) {
	u32 ret = 0;
	printf("Call ARM7 to dump FW\n\n");
	fifoSendValue32(FIFO_BUFFER_ADDR, (u32)buffer);
	fifoSendValue32(FIFO_BUFFER_SIZE, size);
	DC_InvalidateRange((void*)buffer, size);
	fifoSendValue32(FIFO_CONTROL, DSBF_DUMP_FW);
	fifoWaitValue32(FIFO_RETURN);
	fifoGetValue32(FIFO_RETURN);
	switch(buffer[0x1D]) {
		case DEVICE_TYPE_NDSI:
		case DEVICE_TYPE_NDSL:
		case DEVICE_TYPE_NDSL_2:
		case DEVICE_TYPE_NDSP:
		case DEVICE_TYPE_IQUE:
		case DEVICE_TYPE_IQUEL:
			break;
		default:
			printf("WARNING: this device has an\nunknown firmware version\n\n");
			break;
	}
/*
	switch(buffer[0x1D]) {
		case DEVICE_TYPE_NDSI:
			ret = 0x20000;
			break;
		case DEVICE_TYPE_NDSL:
		case DEVICE_TYPE_NDSP:
			ret = 0x40000;
			break;
		case DEVICE_TYPE_IQUE:
		case DEVICE_TYPE_IQUEL:
		default:
			ret = 0x80000;
			break;
	}
*/
	ret = 0x80000;
	return ret;
}

bool write_file(char* path, u8* buffer, u32 size) {
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
}

#define BUFFER_SIZE 1048576

int dump_all(void) {
	u8* buffer = (u8*)memalign(32, BUFFER_SIZE);
	char filename[29] = "/FWXXXXXXXXXXXX"; // uncreative but does the job
	int rc = 0;

	// wait for ARM7 ready
	fifoWaitValue32(FIFO_RETURN);
	fifoGetValue32(FIFO_RETURN);

	u32 ret = dump_firmware(buffer, BUFFER_SIZE);

	printf("MAC: ");
	for(int i=0; i<6; i++) {
		snprintf(filename + 3 + (i*2), 3, "%02X", buffer[0x36+i]);
		printf("%02X", buffer[0x36+i]);
		if (i < 5) printf(":");
	}
	printf("\n\n");

	// use MAC address as folder name
	mkdir(filename, 0777);
	if(access(filename, F_OK) != 0) {
		rc = -4;
		goto end;
	}

	memcpy(filename+15, "/firmware.bin\0", 14);
	if(!write_file(filename, buffer, ret)) {
		rc = -1;
		goto end;
	}
	printf("\n\n");

	toncset(buffer, 0, BUFFER_SIZE);
	dump_arm7(buffer, 0x4000);
	memcpy(filename+16, "biosnds7.rom", 12);
	if(!write_file(filename, buffer, 0x4000)) {
		rc = -2;
		goto end;
	}
	printf("\n\n");

	toncset(buffer, 0, BUFFER_SIZE);
	dump_arm9(buffer, 0x1000);
	memcpy(filename+16, "biosnds9.rom", 12);
	if(!write_file(filename, buffer, 0x1000)) {
		rc = -3;
		goto end;
	}
	printf("\n\n");

	memset(filename+15, 0, sizeof(filename)-15);
	printf("Done! Files saved to\n%s\n\n", filename);

end:
	free(buffer);
	return rc;
}

int main(int argc, char **argv)
{
	int ret = 0;
	consoleDemoInit();

	printf(" NDS B+F dumper 0.2\n");
	printf("=------------------=\n");

	if(!fatInitDefault()) {
		printf("FAT init fail!\n");
		goto end;
	} 

	if(isDSiMode()) {
		printf("This app only runs in DS-mode.\n");
		goto end;
	}
	ret = dump_all();
	if(ret != 0) printf("Something went wrong... error %d\n", ret);

end:
	printf("Press START to exit.\n");
	while(true) {
		swiWaitForVBlank();
		scanKeys();
		if(keysDown() & KEY_START) break;
	}
	return 0;
}
