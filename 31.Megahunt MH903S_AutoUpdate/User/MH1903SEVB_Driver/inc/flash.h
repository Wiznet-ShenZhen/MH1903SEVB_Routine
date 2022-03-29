#ifndef __FLASH_H
#define __FLASH_H

#include "mhscpu.h"
#include "pipe.h"

void Flash_Write_Page(uint32_t address, uint8_t* buffer);

void Flash_Write_Page_From_Pipe(uint32_t address, TEMP_PARAM_LOOP_BUFFER* buffer);

void Flash_Erase(uint32_t address, uint32_t size);

#endif	
