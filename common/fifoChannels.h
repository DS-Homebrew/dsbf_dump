#pragma once

#define FIFO_BUFFER_ADDR	FIFO_USER_01
#define FIFO_BUFFER_SIZE	FIFO_USER_02
#define FIFO_CONTROL		FIFO_USER_03
#define FIFO_RETURN			FIFO_USER_04

enum DSBF_FIFO_CONTROL_VALUES {
	DSBF_EXIT,
	DSBF_DUMP_BIOS7
};
