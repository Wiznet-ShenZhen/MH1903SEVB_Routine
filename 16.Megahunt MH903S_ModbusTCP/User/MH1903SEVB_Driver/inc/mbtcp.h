#ifndef _MBTCP_H_
#define _MBTCP_H_

#include <stdbool.h>
#include "config.h"
#include "Type.h"

extern uint8 mbTCPtid1;
extern uint8 mbTCPtid2;
	
extern volatile uint8 *pucRTUBufferCur;
extern volatile uint16 usRTUBufferPos;
extern volatile uint8 *pucTCPBufferCur;
extern volatile uint16 usTCPBufferPos;


bool MBudp2rtuFrame(void);
bool MBtcp2rtuFrame(void);

void mbTCPtoEVB(void);
void mbEVBtoTCP(void);

#define MB_SER_PDU_SIZE_MIN     3 
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF      1       /*!< Offset of Modbus-PDU in Ser-PDU. */

#define MB_RTU_ADDR_SIZE				1
#endif


