/*
	NDS BIOS/firmware dumper

	Copyright (C) 2023 DS-Homebrew

	SPDX-License-Identifier: MIT
*/

#pragma once

/*
	Firmware header. Found at 0x0-0x29.
*/
typedef struct {
	u16 part3RomAddr;
	u16 part4RomAddr;
	u16 part34crc16;
	u16 part12crc16;
	u32 firmwareIdentifier; // should be "MAC" on retail, "XB00" on no$gba
	u16 part1RomAddr;
	u16 part1RomDestination;
	u16 part2RomAddr;
	u16 part2RomDestination;
	u16 shiftAmount;
	u16 part5RomAddr;
	u8  buildMinute;
	u8  buildHour;
	u8  buildDay;
	u8  buildMonth;
	u8  buildYear;
	u8  deviceType; // see device_type enum
	u16 reserved2;
    u16 userSettingsOffset;
	u16 unk[2];
	u16 part5crc16;
	u16 reserved3;
} PACKED firmware_header_t;

#define FIRMWARE_IQUE_EXT_USERSETTINGS BIT(6)
#define FIRMWARE_DSI_EXT_USERSETTINGS FIRMWARE_IQUE_EXT_USERSETTINGS // same bit enabled

/*
	Firmware type enum.
	Found at 0x1D of firmware header.
	Here, we use it to truncate the firmware to the correct size.
	A switch case for this can be found in printAdditionalFWInfo().
*/
enum device_type {
	DEVICE_TYPE_NDSP_PROTO = 0,
	DEVICE_TYPE_NDSL_KIOSK = 1,
	DEVICE_TYPE_NDSL = 0x20,  // also a certain prototype
	DEVICE_TYPE_NDSL_KOR = 0x35,
	DEVICE_TYPE_IQUE = 0x43,
	DEVICE_TYPE_NDSI = 0x57,
	DEVICE_TYPE_IQUEL = 0x63,
	DEVICE_TYPE_NDSP = 0xFF
};

/*
	Some flash chips do not respond with the chip size on byte 3 of the JEDEC command.
	These need a quirk; known ones that do are listed here.
*/
// 512KiB chips
static const u8 flash_chip_quirk_512[][3] = {
	{0x62, 0x11} // Sanyo LE25FW403A
};

// 256KiB chips
static const u8 flash_chip_quirk_256[][3] = {
	{0x62, 0x16} // Sanyo LE25FW203T
};
