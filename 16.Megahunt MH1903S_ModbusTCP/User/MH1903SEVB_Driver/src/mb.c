#include <stdio.h>
#include "mbtcp.h"
#include "mbascii.h"
#include "usart.h"
#include "mhscpu_gpio.h"
#include "w5500.h"
#include "socket.h"
#include "mb.h"



volatile uint8* pucASCIIBufferCur;
volatile uint16 usASCIIBufferPos;
volatile uint8 *pucTCPBufferCur;
volatile uint16 usTCPBufferPos;

#define  LED_0_1_PORT  GPIOC 
#define  LED_2_3_PORT  GPIOE

#define  LED0_PIN  GPIO_Pin_8
#define  LED1_PIN  GPIO_Pin_9

#define  LED2_PIN  GPIO_Pin_1
#define  LED3_PIN  GPIO_Pin_2


void LED_Configuration(void)
{
	SYSCTRL_APBPeriphClockCmd( SYSCTRL_APBPeriph_GPIO, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
	GPIO_InitStruct.GPIO_Pin = LED0_PIN | LED1_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD_PU;
	GPIO_Init(LED_0_1_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
	GPIO_InitStruct.GPIO_Pin = LED2_PIN | LED3_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD_PU;
	GPIO_Init(LED_2_3_PORT, &GPIO_InitStruct);


	
}


void mbTCPtoEVB(void)
{
	
	if(MBtcp2evbFrame() != FALSE)  //帧接收完成
	{
		if((uint8)pucASCIIBufferCur[3]==0x00)
		{
			if((uint8)pucASCIIBufferCur[4]==0xff)
			{
				GPIO_ResetBits(LED_0_1_PORT, LED0_PIN);
				printf("LED 0 ON\r\n");
			}
			else if((uint8)pucASCIIBufferCur[4]==0x00)
			{
			  GPIO_SetBits(LED_0_1_PORT, LED0_PIN);
				printf("LED 0 OFF\r\n");
			}
		}
		else if(pucASCIIBufferCur[3]==0x01)
		{
			if((uint8)pucASCIIBufferCur[4]==0xff)
			{
				GPIO_ResetBits(LED_0_1_PORT, LED1_PIN);
				printf("LED 1 ON\r\n");
			}
			else if((uint8)pucASCIIBufferCur[4]==0x00)
			{
			  GPIO_SetBits(LED_0_1_PORT, LED1_PIN);
				printf("LED 1 OFF\r\n");
			}
		}
		else if(pucASCIIBufferCur[3]==0x02)
		{
			if((uint8)pucASCIIBufferCur[4]==0xff)
			{
				GPIO_ResetBits(LED_2_3_PORT, LED2_PIN);
				printf("LED 2 ON\r\n");
			}
			else if((uint8)pucASCIIBufferCur[4]==0x00)
			{
			  GPIO_SetBits(LED_2_3_PORT, LED2_PIN);
				printf("LED 2 OFF\r\n");
			}
		}
		else if(pucASCIIBufferCur[3]==0x03)
		{
			if((uint8)pucASCIIBufferCur[4]==0xff)
			{
				GPIO_ResetBits(LED_2_3_PORT, LED3_PIN);
				printf("LED 3 ON\r\n");
			}
			else if((uint8)pucASCIIBufferCur[4]==0x00)
			{
			  GPIO_SetBits(LED_2_3_PORT, LED3_PIN);
				printf("LED 3 OFF\r\n");
			}
		}
		while(usASCIIBufferPos)
  	{      
			printf("%0x ",(uint8)*pucASCIIBufferCur);
		  pucASCIIBufferCur++;
		  usASCIIBufferPos--;
	  }
	}
}

void mbEVBtoTCP(void)
{
	if(MBevb2tcpFrame() != FALSE)
	{
		switch(getSn_SR(0))
		{	 
		case SOCK_ESTABLISHED:
    case SOCK_CLOSE_WAIT:
			send(0, (uint8*)pucTCPBufferCur, usTCPBufferPos);
			break;
		default:
      break;
	  }
	}
}

