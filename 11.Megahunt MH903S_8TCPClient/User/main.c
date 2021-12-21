#include <stdio.h>
#include <stdint.h>
#include "mhscpu.h"
#include "sysc.h"
#include "delay.h"
#include "timer.h"
#include "usart.h"
#include "w5500.h"
#include "socket.h"
#include "tcp.h"
uint8_t MAC[6] = { 0x00,0x08,0xdc,0x11,0x12,0x13 };
uint8_t LocalIp[4] = { 192,168,1,150 };
uint8_t Subnet[4] = { 255,255,255,0 };
uint8_t Gateway[4] = { 192,168,1,1 };
uint8_t DNS[4] = { 114,114,114,114 };

///-------------------------------------------------------------------------------------------------
/// <summary>Netwwork initialize.</summary>
///
/// <remarks>Tony Wang, 2021/6/24.</remarks>
///-------------------------------------------------------------------------------------------------

void Netwwork_Init(void)
{
	uint8_t MAC[6] = { 0x00,0x08,0xdc,0x11,0x12,0x13 };
	uint8_t LocalIp[4] = { 192,168,1,150 };
	uint8_t Subnet[4] = { 255,255,255,0 };
	uint8_t Gateway[4] = { 192,168,1,1 };

	uint8_t TxSize[MAX_SOCK_NUM] = { 2,2,2,2,2,2,2,2 };
	uint8_t RxSize[MAX_SOCK_NUM] = { 2,2,2,2,2,2,2,2 };

	uint8_t temp[4] = { 0 };

	setSHAR(MAC);
	setSUBR(Subnet);
	setGAR(Gateway);
	setSIPR(LocalIp);

	sysinit(TxSize, RxSize);

	setRTR(2000);
	setRCR(3);

	getSIPR(temp);
	printf("COMW-EVB IP : %d.%d.%d.%d\r\n", temp[0], temp[1], temp[2], temp[3]);

	getSUBR(temp);
	printf("COMW-EVB SN : %d.%d.%d.%d\r\n", temp[0], temp[1], temp[2], temp[3]);

	getGAR(temp);
	printf("COMW-EVB GW : %d.%d.%d.%d\r\n", temp[0], temp[1], temp[2], temp[3]);

}

int main(void)
{
	SystemClock_Config_HSE();

	Delay_Init();

	//Timer_Init();

	USART_Init();

	printf("USART_Init\r\n");

	W5500_Init();

	printf("W5500_Init\r\n");

	W5500_Reset();

	printf("W5500_Reset\r\n");

	Netwwork_Init();

	printf("Netwwork_Init\r\n");
	printf("Start 8TCP Test!\r\n"); 	

	while (1)
	{
		do_8tcpc();
	}
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	 /* Infinite loop */
	while (1)
	{
	}
}
#endif
