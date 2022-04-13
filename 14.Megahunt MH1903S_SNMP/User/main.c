#include <stdio.h>
#include <stdint.h>
#include "mhscpu.h"
#include "sysc.h"
#include "delay.h"
#include "usart.h"
#include "w5500.h"
#include "socket.h"
#include "snmp.h"

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

	setKPALVTR(SOCK_SNMP, 0x02);
}

int32_t wiznetLedStatus = 0;

void getWIZnetLed(void* ptr, uint8_t* len)
{
	if (wiznetLedStatus == 0)
		*len = sprintf((char*)ptr, "LED OFF");
	else
		*len = sprintf((char*)ptr, "LED ON");
}

void setWIZnetLed(int32_t val)
{
	wiznetLedStatus = val;

	if (wiznetLedStatus == 0)
		printf("Set LED OFF\r\n");
	else
		printf("Set LED ON\r\n");
}

dataEntryType snmpData[] =
{
	// System MIB

	// SysDescr Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 1, 0},
	SNMPDTYPE_OCTET_STRING, 30, {"WIZnet Embedded SNMP Agent"},
	NULL, NULL},

	// SysObjectID Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 2, 0},
	SNMPDTYPE_OBJ_ID, 8, {"\x2b\x06\x01\x02\x01\x01\x02\x00"},
	NULL, NULL},

	// SysContact Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 4, 0},
	SNMPDTYPE_OCTET_STRING, 30, {"support@wiznet.co.kr"},
	NULL, NULL},

	// SysName Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 5, 0},
	SNMPDTYPE_OCTET_STRING, 30, {"http://www.wiznet.co.kr"},
	NULL, NULL},

	// Location Entry
	{8, {0x2b, 6, 1, 2, 1, 1, 6, 0},
	SNMPDTYPE_OCTET_STRING, 30, {"4F Humax Village"},
	NULL, NULL},

	// SysServices
	{8, {0x2b, 6, 1, 2, 1, 1, 7, 0},
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, NULL},

	// WIZnet LED 
	{8, {0x2b, 6, 1, 4, 1, 0, 1, 0},
	SNMPDTYPE_OCTET_STRING, 30, {""},
	getWIZnetLed, NULL},

	{8, {0x2b, 6, 1, 4, 1, 0, 2, 0},
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, setWIZnetLed}
};

const int32_t maxData = (sizeof(snmpData) / sizeof(dataEntryType));

void SNMP_Init(void)
{
	WDEBUG("Start UserSnmpDemo");

	SnmpXInit();

	dataEntryType enterprise_oid = { 8, {0x2b, 6, 1, 4, 1, 0, 0x10, 0}, SNMPDTYPE_OBJ_ID, 8, {"\x2b\x06\x01\x04\x01\x00\x10\x00"},	NULL, NULL };

	dataEntryType trap_oid1 = { 8, {0x2b, 6, 1, 4, 1, 0, 11, 0}, SNMPDTYPE_OCTET_STRING, 30, {""}, NULL, NULL };
	dataEntryType trap_oid2 = { 8, {0x2b, 6, 1, 4, 1, 0, 12, 0}, SNMPDTYPE_INTEGER, 4, {""}, NULL, NULL };

	strcpy((char*)trap_oid1.u.octetstring, "Alert!!!");
	trap_oid2.u.intval = 123456;

	//SnmpXTrapSend("222.98.173.250", "127.0.0.0", "public", enterprise_oid, 1, 0, 0);
	//SnmpXTrapSend("222.98.173.250", "127.0.0.0", "public", enterprise_oid, 6, 0, 2, &trap_oid1, &trap_oid2);
	SnmpXTrapSend((int8_t*)"192.168.1.110", (int8_t*)"192.168.1.111", (int8_t*)"public", enterprise_oid, 1, 0, 0);
	SnmpXTrapSend((int8_t*)"192.168.1.111", (int8_t*)"127.0.0.0", (int8_t*)"public", enterprise_oid, 6, 0, 2, &trap_oid1, &trap_oid2);
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
	printf("COMW-EVB SNMP v1\r\n");

	SNMP_Init();

	SnmpXDaemon();
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
