/*
    dumps DS arm7, arm9 bios and firmware
*/

#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gba_nds_fat.h"

#define MAX_SIZE    (1*1024*1024 + 256)  // 1048832 != 262144 file out ??

FAT_FILE *f;

u32 DumpFirmware(u8 *firmware_buffer, u32 max_size)
{
    IPC->mailAddr = (u32)firmware_buffer;
    IPC->mailSize = max_size;
//    IPC->mailData = 0x00;
    IPC->mailBusy = 0xF1;
    while (IPC->mailBusy);
    return IPC->mailSize;
}

u32 DumpBios (u8 *firmware_buffer, u32 max_size)
{
    IPC->mailAddr = (u32)firmware_buffer;
    IPC->mailSize = max_size;
//    IPC->mailData = 0xFF;
//    IPC->mailBusy = 0xF1;
    IPC->mailBusy = 0xF2;
    while (IPC->mailBusy);
    return IPC->mailSize;
}

int SaveToFile(char *filename, u8 *firmware_buffer, u32 size)
{
//    if (!FAT_InitFiles()) {iprintf("FAT init failed\n"); return -1;}
    f = FAT_fopen(filename, "wb");
    if (f == '\0') iprintf("File open failed");
    FAT_fwrite(firmware_buffer, 1, size, f);
    FAT_fclose(f);
    return 0;
}

int dumper()
{
    u8 *firmware_buffer = (u8 *)malloc(MAX_SIZE) + 0x400000;    // uncached
    u32 size = DumpFirmware(firmware_buffer, MAX_SIZE);

    iprintf("-Dumping firmware-\n");
    iprintf("Size  : %u\n", size);
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
    iprintf("Size  : %u\n", size);
    iprintf("Saving: BIOSNDS7.ROM...\n\n", filename);
    if (SaveToFile("BIOSNDS7.ROM", firmware_buffer, size) < 0)
    {
        iprintf("Error!\n");
        return -1;
    }
//Arm 9 bios directly. The range to read is 0xFFFF0000 - 0xFFFF0FFF. (4KB).
    iprintf("--dumping A9 bios--\n");
    size = 4096; // arm9 is 4k
    iprintf("Size  : %u\n", size);
    iprintf("Saving: BIOSNDS9.ROM...\n\n", filename);
    if (SaveToFile("BIOSNDS9.ROM", (u8*)0xFFFF0000, size) < 0)
    {
        iprintf("Error!\n");
        return -1;
    }

    iprintf("Freeing files\n");
    return 0;
}

/*
 * main
 */
int main()
{
    irqInit();
    irqSet(IRQ_VBLANK, 0);
    //irqEnable(IRQ_VBLANK);

    POWER_CR = POWER_ALL_2D ;

    videoSetMode(MODE_FB1);    //not using the main screen - make it black
    videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);    //sub bg 0 will be used to print text
    vramSetBankC(VRAM_C_SUB_BG);

    SUB_BG0_CR = BG_MAP_BASE(31);

    BG_PALETTE_SUB[255] = RGB15(0,0,31);
    consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);

    iprintf(" NDS B+F dumper 0.1\n");
    iprintf("=------------------=\n");

    if (!FAT_InitFiles()) {iprintf("\nFAT init failed!\n"); return -1;}
    else dumper();

    if (!FAT_FreeFiles())  {iprintf("\nFAT free failed!\n"); return -1;}
    iprintf("Dumps completed.\n");


    while (1) swiWaitForVBlank();

    return 0;
}
