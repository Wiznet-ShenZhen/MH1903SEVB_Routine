#include "mhscpu.h"
#include "mhscpu_it.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles UART0.
  * @param  None
  * @retval None
  */

void UART0_IRQHandler(void)
{
	volatile uint32_t iir;

	UART_TypeDef* UARTx = UART0;

	iir = UART_GetITIdentity(UARTx);

	switch (iir & 0x0f)
	{

	case UART_IT_ID_RX_RECVD:
	{
		uint8_t receiveData = UART_ReceiveData(UARTx);

		if ((USART_RX_STA & 0x8000) == 0)
		{
			if (USART_RX_STA & 0x4000)
			{
				if (receiveData != 0x0A)
					USART_RX_STA = 0;
				else
					USART_RX_STA |= 0x8000;
			}
			else
			{
				if (receiveData == 0x0D)
					USART_RX_STA |= 0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0x3FFF] = receiveData;

					USART_RX_STA++;

					if (USART_RX_STA > (sizeof(USART_RX_BUF) - 1))
						USART_RX_STA = 0;
				}
			}
		}
	}

	break;

	case UART_IT_ID_MODEM_STATUS:
	{
		volatile uint32_t msr = UARTx->MSR;
	}

	break;

	case UART_IT_ID_LINE_STATUS:
	{
		volatile uint32_t lsr = UARTx->LSR;
	}

	break;

	case UART_IT_ID_BUSY_DETECT:
	{
		volatile uint32_t usr = UARTx->USR;
	}

	break;

	case UART_IT_ID_CHAR_TIMEOUT:
	{
		volatile uint32_t rbr = UART_ReceiveData(UARTx);
	}

	break;

	default:

		break;

	}
}

/**
  * @brief  This function handles TIM0.
  * @param  None
  * @retval None
  */

extern uint32_t ms;

void TIM0_0_IRQHandler(void)
{
	ms++;

	if (ms >= 1000)
	{
		ms = 0;

	}

	TIM_ClearITPendingBit(TIMM0, TIM_0);

	NVIC_ClearPendingIRQ(TIM0_0_IRQn);
}
