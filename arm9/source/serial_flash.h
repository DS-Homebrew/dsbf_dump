/*
	NDS BIOS/firmware dumper

	Copyright (C) 2023 DS-Homebrew

	SPDX-License-Identifier: MIT
*/

#pragma once

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
