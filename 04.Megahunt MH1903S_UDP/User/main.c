#include <stdio.h>
#include <stdint.h>
#include "mhscpu.h"
#include "sysc.h"
#include "delay.h"
#include "usart.h"
#include "w5500.h"
#include "socket.h"

#define UDP_SOCKET		0

uint8_t remoteIP[4] = { 0 };
uint16_t remotePort;

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

	setKPALVTR(UDP_SOCKET, 0x02);
}

///-------------------------------------------------------------------------------------------------
/// <summary>TCP Client Process Function.</summary>
///
/// <remarks>Tony Wang, 2021/6/25.</remarks>
///-------------------------------------------------------------------------------------------------

void UDP()
{
	uint8_t buffer[2048];

	uint16_t localPort = 5000;

	uint16_t len = 0;

	switch (getSn_SR(UDP_SOCKET))
	{

	case SOCK_UDP:

		Delay_ms(100);

		if (getSn_IR(UDP_SOCKET) & Sn_IR_RECV)
		{
			setSn_IR(UDP_SOCKET, Sn_IR_RECV);

			printf("Received\r\n");
		}

		len = getSn_RX_RSR(UDP_SOCKET);

		if (len > 0)
		{
			memset(buffer, 0, len + 1);

			recvfrom(UDP_SOCKET, buffer, len, remoteIP, &remotePort);

			printf("Receive : %s\r\n", buffer);

			sendto(UDP_SOCKET, buffer, len, remoteIP, remotePort);
		}

		if (USART_RX_STA & 0x8000)
		{
			len = USART_RX_STA & 0x3FFF;

			sendto(UDP_SOCKET, USART_RX_BUF, len, remoteIP, remotePort);

			USART_RX_STA = 0;

			memset(USART_RX_BUF, 0, len + 1);
		}

		break;

	case SOCK_CLOSED:

		socket(UDP_SOCKET, Sn_MR_UDP, localPort, Sn_MR_CLOSE);

		break;

	}
}

int main(void)
{
	SystemClock_Config_HSE();

	Delay_Init();

	USART_Init();

	printf("USART_Init\r\n");

	W5500_Init();

	printf("W5500_Init\r\n");

	W5500_Reset();

	printf("W5500_Reset\r\n");

	Netwwork_Init();

	printf("Netwwork_Init\r\n");
	printf("COMW-EVB UDP\r\n");

	while (1)
	{
		UDP();
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
