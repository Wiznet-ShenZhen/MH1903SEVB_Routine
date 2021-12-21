#ifndef __MD5_H
#define __MD5_H

///-------------------------------------------------------------------------------------------------
/// <summary>	
///		MD5 context. 
/// 		
/// 		state	state (ABCD)
/// 		count	number of bits, modulo 2^64 (lsb first)
/// 		buffer	input buffer
/// 		
/// </summary>
///
/// <remarks>	Tony Wang, 15:48 19/3/21. </remarks>
///-------------------------------------------------------------------------------------------------

#include <stdint.h>

typedef struct {
	uint32_t state[4];
	uint32_t count[2];
	uint8_t  buffer[64];
} md5_ctx;

void md5_init(md5_ctx* context);

void md5_update(md5_ctx* context, uint8_t* buffer, uint32_t length);

void md5_final(uint8_t result[16], md5_ctx* context);

#endif	
