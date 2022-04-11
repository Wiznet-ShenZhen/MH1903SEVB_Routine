#ifndef __PIPE_H
#define __PIPE_H

#include <stdint.h>

typedef struct _TEMP_PARAM_LOOP_BUFFER
{
	//Buffer full flag
	uint8_t Full;

	//Buffer read offset
	uint16_t ReadOffset;

	//Buffer write offset
	uint16_t WriteOffset;

	//Buffer pointer
	uint8_t* Buffer;

	//Buffer size
	uint16_t Size;

	//Get buffer left data count
	uint16_t(*Count)(struct _TEMP_PARAM_LOOP_BUFFER*);

	//Clear all data in buffer
	void(*Clear)(struct _TEMP_PARAM_LOOP_BUFFER*);

	//Read one byte from buffer
	uint8_t(*Read)(struct _TEMP_PARAM_LOOP_BUFFER*);

	//Write one byte into buffer
	void(*Write)(struct _TEMP_PARAM_LOOP_BUFFER*, uint8_t);

}TEMP_PARAM_LOOP_BUFFER;

extern TEMP_PARAM_LOOP_BUFFER Pipe_Usart;

void Pipe_Init(void);

#endif
