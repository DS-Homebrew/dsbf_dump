/*
	NDS BIOS/firmware dumper

	Copyright (C) 2023 DS-Homebrew

	SPDX-License-Identifier: MIT
*/

#pragma once

#define DSBF_PXI_CONTROL PxiChannel_User0

typedef enum DSBF_ThreadMsgType {
	DSBF_DUMP_JEDEC = 0,
	DSBF_DUMP_BIOS7 = 1
} DSBF_ThreadMsgType;
