#include <stdio.h>
#include <string.h>
#include "pipe.h"

static uint16_t Buffer_Count(TEMP_PARAM_LOOP_BUFFER* Buffer)
{
	if (Buffer->Full == 1)
		return Buffer->Size;
	else
		return((Buffer->Size + Buffer->WriteOffset - Buffer->ReadOffset) % Buffer->Size);
}

static void Buffer_Clear(TEMP_PARAM_LOOP_BUFFER* Buffer)
{
	Buffer->Full = 0;

	Buffer->ReadOffset = 0;

	Buffer->WriteOffset = 0;

	memset(Buffer->Buffer, 0x00, Buffer->Size);
}

static uint8_t Buffer_Read(TEMP_PARAM_LOOP_BUFFER* Buffer)
{
	if (Buffer->Count(Buffer))
	{
		uint8_t readData;

		if (Buffer->Full == 1)
			Buffer->Full = 0;

		readData = Buffer->Buffer[Buffer->ReadOffset++];

		if (Buffer->ReadOffset == Buffer->Size)
			Buffer->ReadOffset = 0;

		return readData;
	}
	else
		return 0x00;
}

static void Buffer_Write(TEMP_PARAM_LOOP_BUFFER* Buffer, uint8_t data)
{
	if (Buffer->Full != 1)
	{
		Buffer->Buffer[Buffer->WriteOffset++] = data;

		if (Buffer->WriteOffset == Buffer->Size)
			Buffer->WriteOffset = 0;

		if (Buffer->WriteOffset == Buffer->ReadOffset)
			Buffer->Full = 1;
	}
}

static void Buffer_Init(TEMP_PARAM_LOOP_BUFFER* Buffer, uint8_t* buffer, uint16_t size)
{
	Buffer->Buffer = buffer;
	Buffer->Size = size;

	Buffer->Count = Buffer_Count;
	Buffer->Clear = Buffer_Clear;
	Buffer->Read = Buffer_Read;
	Buffer->Write = Buffer_Write;

	Buffer->Clear(Buffer);
}

TEMP_PARAM_LOOP_BUFFER Pipe_OTA;

uint8_t Pipe_OTA_Buffer[0x800];

void Pipe_Init(void)
{
	Buffer_Init(
		&Pipe_OTA,
		Pipe_OTA_Buffer,
		sizeof(Pipe_OTA_Buffer)
	);
}
