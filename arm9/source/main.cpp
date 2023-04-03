/*
    dumps DS arm7, arm9 bios and firmware
*/

#include <nds.h>
#include <nds/arm9/console.h>
#include <fat.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../common/fifoChannels.h"

#define MAX_SIZE    (1*1024*1024 + 256)  // 1048832 != 262144 file out ??

u32 DumpFirmware(u8 *firmware_buffer, u32 max_size)
{
    fifoSendValue32(FIFO_BUFFER_ADDR, (u32)firmware_buffer);
    fifoSendValue32(FIFO_BUFFER_SIZE, max_size);
    fifoSendValue32(FIFO_CONTROL, DSBF_DUMP_FW);
    fifoWaitValue32(FIFO_RETURN);
    return fifoGetValue32(FIFO_RETURN);
}

u32 DumpBios (u8 *firmware_buffer, u32 max_size)
{
    fifoSendValue32(FIFO_BUFFER_ADDR, (u32)firmware_buffer);
    fifoSendValue32(FIFO_BUFFER_SIZE, max_size);
    fifoSendValue32(FIFO_CONTROL, DSBF_DUMP_BIOS7);
    fifoWaitValue32(FIFO_RETURN);
    return fifoGetValue32(FIFO_RETURN);
}

int SaveToFile(char *filename, u8 *firmware_buffer, u32 size)
{
    FILE *f = fopen(filename, "wb");
    if (!f) {
        iprintf("File open failed");
        fclose(f);
        return -1;
    }
    fwrite(firmware_buffer, 1, size, f);
    fclose(f);
    return 0;
}

int dumper()
{
    // wait for ARM7 ready
    fifoWaitValue32(FIFO_RETURN);
    fifoGetValue32(FIFO_RETURN);

    u8 *firmware_buffer = (u8 *)malloc(MAX_SIZE) + 0x400000;    // uncached
    u32 size = DumpFirmware(firmware_buffer, MAX_SIZE);

    iprintf("-Dumping firmware-\n");
    iprintf("Size  : %lu\n", size);
    if (size > MAX_SIZE)
    {
        iprintf("Firmware bigger than expected.\n");
        return -1;
    }

    iprintf("MAC   : ");
    for (int i=0; i<6; i++)
    {
        printf("%02X", firmware_buffer[0x36+i]);
        if (i < 5) printf(":");
    }
    iprintf("\n");
    char filename[13+1];
    strcpy(filename, "FW001122.BIN");
    for (int i=0; i<6; i++)
    {
        filename[2+i] = "0123456789ABCDEF"[firmware_buffer[0x39 + i/2] >> (4*(i&1^1)) & 15];
    }
    iprintf("Saving: %s...\n\n", filename);
    if (SaveToFile(filename, firmware_buffer, size) < 0)
    {
        iprintf("Error!\n");
        return -1;
    }
    iprintf("--dumping A7 bios--\n");
    size = DumpBios(firmware_buffer, MAX_SIZE);
    iprintf("Size  : %lu\n", size);
    iprintf("Saving: BIOSNDS7.ROM...\n\n");
    if (SaveToFile("BIOSNDS7.ROM", firmware_buffer, size) < 0)
    {
        iprintf("Error!\n");
        return -1;
    }
//Arm 9 bios directly. The range to read is 0xFFFF0000 - 0xFFFF0FFF. (4KB).
    iprintf("--dumping A9 bios--\n");
    size = 4096; // arm9 is 4k
    iprintf("Size  : %lu\n", size);
    iprintf("Saving: BIOSNDS9.ROM...\n\n");
    if (SaveToFile("BIOSNDS9.ROM", (u8*)0xFFFF0000, size) < 0)
    {
        iprintf("Error!\n");
        return -1;
    }

    iprintf("Freeing files\n");
    return 0;
}

int exit(void) {
    iprintf("Press START to exit.\n");
    while(true) {
        swiWaitForVBlank();
        scanKeys();
        if(keysDown() & KEY_START) break;
    }
    fifoSendValue32(FIFO_CONTROL, DSBF_EXIT);
    return 0;
}

/*
 * main
 */
int main(void)
{
    consoleDemoInit();

    iprintf(" NDS B+F dumper 0.2\n");
    iprintf("=------------------=\n");

    if (!fatInitDefault()) {
        iprintf("\nFAT init failed!\n");
        return exit();
    }
    if(isDSiMode()) {
        iprintf("\nThis app only works in DS mode.\n");
        return exit();
    }
    dumper();

    iprintf("Dumps completed.\n");

    return exit();
}
