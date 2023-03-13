#include <nds.h>

/*
 * memcmp2
 */
volatile int memcmp2(u8 *a, u8 *b, u32 len)
{
	u32 i;
	for (i=0; i<len; i++)
	{
		if (a[i] != b[i]) return 1;
	}
	return 0;
}

/*
 * DumpFirmware
 */
u32 DumpFirmware(u8 *firmware_buffer, u32 max_size)
{
	u32 a;
	u32 size = 0;
	u32 checkpoint = 128*1024;

	// release from deep power-down
	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_NVRAM | SPI_CONTINUOUS;
	REG_SPIDATA = 0xAB; SerialWaitBusy();
	REG_SPICNT = 0;
	swiDelay(1);

	// read...
	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_NVRAM | SPI_CONTINUOUS;
	REG_SPIDATA = 0x0B; SerialWaitBusy();		// fast read
	REG_SPIDATA = 0; SerialWaitBusy();	// >>16
	REG_SPIDATA = 0; SerialWaitBusy();	// >>8
	REG_SPIDATA = 0; SerialWaitBusy();	// >>0
	REG_SPIDATA = 0; SerialWaitBusy();	// dummy byte for fast read
	
	for (a=0; ; a++)
	{
		if (a == checkpoint + 256)
		{
			if (memcmp2(firmware_buffer, firmware_buffer+checkpoint, 256) == 0) { size = checkpoint; break; }
			checkpoint <<= 1;
		}
		REG_SPIDATA = 0; SerialWaitBusy();
		if (a >= max_size) { size = max_size*2; break; }
		firmware_buffer[a] = REG_SPIDATA;
	}
	REG_SPICNT = 0;
	swiDelay(1);

	return size;
}
