#ifndef __DELAY_H
#define	__DELAY_H

#ifdef __cplusplus
extern "C"
{
#endif 

#include "stdint.h"

	typedef uint32_t tick;

	void Delay_Init(void);

	void Delay_us(tick usec);
	void Delay_ms(tick msec);

#ifdef __cplusplus
}
#endif

#endif   ///< __DELAY_H
