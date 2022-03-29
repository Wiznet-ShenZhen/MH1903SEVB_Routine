#include <stdio.h>
#include <string.h>
#include "mhscpu.h"
#include "flash.h"

#define FLASH_SECTOR_SIZE		0x1000

/**
  * @brief  Flash Erase Sector.
  * @param  cmdParam:      pointer to a QSPI_CommandTypeDef structure that contains the configuration information.
  * @param  sectorAddress: The sector address to be erased
  * @retval FLASH Status:  The returned value can be: QSPI_STATUS_ERROR, QSPI_STATUS_OK
  */
  //#define ROM_QSPI_EraseSector    (*((uint8_t (*)(QSPI_CommandTypeDef *cmdParam, uint32_t sectorAddress))(*(uint32_t *)0x0024)))

	/**
	  * @brief  Flash Program Interface.
	  * @param  cmdParam:      pointer to a QSPI_CommandTypeDef structure that contains the configuration information.
	  * @param  DMA_Channelx:  DMA_Channel_0
	  * @param  addr:          specifies the address to be programmed.
	  * @param  size:          specifies the size to be programmed.
	  * @param  buffer:        pointer to the data to be programmed, need word aligned
	  * @retval FLASH Status:  The returned value can be: QSPI_STATUS_ERROR, QSPI_STATUS_OK
	  */
	  //#define ROM_QSPI_ProgramPage    (*((uint8_t (*)(QSPI_CommandTypeDef *cmdParam, DMA_TypeDef *DMA_Channelx, uint32_t addr, uint32_t size, uint8_t *buffer))(*(uint32_t *)0x0028)))   

	  //uint8_t __attribute__((section("RAM_CODE"), __used__)) FLASH_EraseSector(uint32_t sectorAddress)
	  //{
	  //	uint8_t ret;
	  //
	  //	__disable_irq();
	  //	__disable_fault_irq();
	  //
	  //	ret = ROM_QSPI_EraseSector(NULL, sectorAddress);
	  //
	  //	__enable_fault_irq();
	  //	__enable_irq();
	  //
	  //	return ret;
	  //}

	  //uint8_t __attribute__((section("RAM_CODE"), __used__)) FLASH_ProgramPage(uint32_t addr, uint32_t size, uint8_t* buffer)
	  //{
	  //	uint8_t ret;
	  //	QSPI_CommandTypeDef cmdType;
	  //
	  //	cmdType.Instruction = QUAD_INPUT_PAGE_PROG_CMD;
	  //	cmdType.BusMode = QSPI_BUSMODE_114;
	  //	cmdType.CmdFormat = QSPI_CMDFORMAT_CMD8_ADDR24_PDAT;
	  //
	  //	__disable_irq();
	  //	__disable_fault_irq();
	  //
	  //	ret = ROM_QSPI_ProgramPage(&cmdType, DMA_Channel_0, addr, size, buffer);
	  //
	  //	__enable_fault_irq();
	  //	__enable_irq();
	  //
	  //	return ret;
	  //}

void DataPrintf(void* buf, uint32_t bufsize)
{
	uint32_t i = 0;
	uint8_t* pBuf = (uint8_t*)buf;

	if (0 != bufsize)
	{
		for (i = 0; i < bufsize; i++)
		{
			if (0 != i && 0 == i % 16)
			{
				printf(" \n");
			}

			printf("%02X ", pBuf[i]);
		}
	}
	printf("\n");
}

int DataCheck(void* src, void* dst, uint32_t size)
{
	uint8_t* pSrc = (uint8_t*)src, * pDst = (uint8_t*)dst;

	if (memcmp(pDst, pSrc, size))
	{
		/*printf("=======================================\r\n");

		printf("Source:\r\n");

		DataPrintf(pSrc, size);

		printf("Destination:\r\n");

		DataPrintf(pDst, size);

		printf("=======================================\r\n");*/

		return -1;
	}

	return 0;
}

int EraseCheck(uint32_t addr, uint32_t pagNum)
{
	uint32_t i = 0;
	CACHE_InitTypeDef CACHE_AddrStruct;
	uint8_t erase_Buf[X25Q_PAGE_SIZE] = { 0 };

	memset(erase_Buf, 0xFF, sizeof(erase_Buf));

	for (i = 0; i < pagNum; i++)
	{
		CACHE_AddrStruct.Address = addr + i * X25Q_PAGE_SIZE;
		CACHE_AddrStruct.size = X25Q_PAGE_SIZE;

		CACHE_Clean(CACHE, &CACHE_AddrStruct);

		if (-1 == DataCheck(erase_Buf, (uint8_t*)(addr + i * X25Q_PAGE_SIZE), sizeof(erase_Buf)))
		{
			printf("Erase Check failed!\n");
			while (1);
		}
	}

	return 0;
}

uint32_t FLASH_PagesMask(uint32_t Size)
{
	uint32_t pageNumber;

	if ((Size % X25Q_PAGE_SIZE) != 0)
		pageNumber = (Size / X25Q_PAGE_SIZE) + 1;
	else
		pageNumber = Size / X25Q_PAGE_SIZE;

	return pageNumber;
}

uint32_t FLASH_SectorMask(uint32_t Size)
{
	uint32_t sectorNumber;

	if ((Size % FLASH_SECTOR_SIZE) != 0)
		sectorNumber = (Size / FLASH_SECTOR_SIZE) + 1;
	else
		sectorNumber = Size / FLASH_SECTOR_SIZE;

	return sectorNumber;
}

uint8_t FlashReceiveBuf[X25Q_PAGE_SIZE] __attribute__((at(0x20001000)));

void Flash_Write_Page(uint32_t address, uint8_t* buffer)
{
	memset(FlashReceiveBuf, 0xFF, X25Q_PAGE_SIZE);
	memcpy(FlashReceiveBuf, buffer, X25Q_PAGE_SIZE);

	SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);
	SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);

	FLASH_ProgramPage(address, X25Q_PAGE_SIZE, FlashReceiveBuf);

	CACHE_CleanAll(CACHE);
	if (-1 == DataCheck(FlashReceiveBuf, (uint8_t*)address, X25Q_PAGE_SIZE))
		printf("Addr %#x data[Cache] check error!\r\n", address);
}

void Flash_Write_Page_From_Pipe(uint32_t address, TEMP_PARAM_LOOP_BUFFER* buffer)
{
	memset(FlashReceiveBuf, 0xFF, X25Q_PAGE_SIZE);
	for (uint16_t i = 0; i < X25Q_PAGE_SIZE; i++)
		FlashReceiveBuf[i] = buffer->Read(buffer);

	SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);
	SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);

	FLASH_ProgramPage(address, X25Q_PAGE_SIZE, FlashReceiveBuf);

	CACHE_CleanAll(CACHE);
	if (-1 == DataCheck(FlashReceiveBuf, (uint8_t*)address, X25Q_PAGE_SIZE))
		printf("Addr %#x data[Cache] check error!\r\n", address);
}

#define FLASH_PROCESS_WATCH_DOG_RELOAD		

void Flash_Erase(uint32_t address, uint32_t size)
{
	uint32_t sectorNumber;

	sectorNumber = FLASH_SectorMask(size);

	SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);
	SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);

	for (uint32_t i = 0; i < sectorNumber; i++)
	{
		FLASH_EraseSector(address + i * FLASH_SECTOR_SIZE);

		EraseCheck(address + i * FLASH_SECTOR_SIZE, 16);

		FLASH_PROCESS_WATCH_DOG_RELOAD;
	}
}
