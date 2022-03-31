#include "mhscpu.h"
#include "delay.h"

tick g_current_tick = 0;

void Delay_Init(void)
{
	SYSCTRL_ClocksTypeDef clocks;

	SYSCTRL_GetClocksFreq(&clocks);
	SysTick_Config(clocks.CPU_Frequency / 1000000);   ///< 1us
}

void SysTick_Handler(void)
{
	g_current_tick++;
}

tick get_tick(void)
{
	return g_current_tick;
}

tick get_diff_tick(tick cur_tick, tick prior_tick)
{
	if (cur_tick < prior_tick)
	{
		return (cur_tick + (~prior_tick));
	}
	else
	{
		return (cur_tick - prior_tick);
	}
}

void Delay_us(tick usec)
{
	tick old_tick;

	old_tick = get_tick();
	while (get_diff_tick(get_tick(), old_tick) < usec)
	{
	}
}

void Delay_ms(tick msec)
{
	tick old_tick;

	old_tick = get_tick();
	while (get_diff_tick(get_tick(), old_tick) < (msec * 1000))
	{
	}
}
