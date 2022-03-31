#include "mhscpu.h"
#include "timer.h"

uint32_t ms = 0;

void Timer_Init(void)
{
	TIM_InitTypeDef TIM_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

	SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TIMM0, ENABLE);
	SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_TIMM0, ENABLE);

	//Timer
	TIM_InitStruct.TIM_Period = SYSCTRL->PCLK_1MS_VAL;

	TIM_InitStruct.TIMx = TIM_0;
	TIM_Init(TIMM0, &TIM_InitStruct);
	TIM_ITConfig(TIMM0, TIM_InitStruct.TIMx, ENABLE);

	//NVIC
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);

	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = TIM0_0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;

	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIMM0, (TIM_NumTypeDef)TIM_0, ENABLE);
}
