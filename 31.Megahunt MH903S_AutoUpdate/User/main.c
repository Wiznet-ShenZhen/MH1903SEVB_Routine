#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "mhscpu.h"
#include "sysc.h"
#include "delay.h"
#include "usart.h"
#include "w5500.h"
#include "socket.h"
#include "flash.h"
#include "pipe.h"

/*
	Warning:

	When modify macro definition <BOOT_MODE> or <APP_MODE>, remember to modify
	start address of firmware area and size of firmware in keil!

	Only one macro definition can be active in one time!

	Area Name					Start					Size
	------------------------------------------------------------
	Bootloader					0x01001000U				0x8000
	Application					0x01009000U				0x1C000

*/

#define BOOT_MODE

//#define APP_MODE

#define TCP_CLIENT_SOCKET	0

typedef enum _OTA_STATE
{
	Normal,
	Inactive,
	InactivePending,
	Active,
	Updating,
	Done
}OTA_INACTIVE;

OTA_INACTIVE otaState = Inactive;

#define	FW_VER_HIGH			1

#define FW_VER_LOW			0

#define OTA_DETECT_VERSION_FUNCTION

#ifdef OTA_DETECT_VERSION_FUNCTION

#define	OTA_DETECT_MSG		"POST /data.php HTTP/1.1\r\n"\
							"Host: ibinhub.com\r\n"\
							"Accept: image/gif,image/x-xbitmap,image/jpeg,image/pjpeg,*/*\r\n"\
							"User-Agent: Mozilla/4.0 (compatible;MSTE 5.5;Windows 98)\r\n"\
							"Content-Length: 48\r\n"\
							"Content-Type: application/x-www-form-urlencoded\r\n"\
							"Cache-Control: no-Cache\r\n"\
							"Connection: close\r\n"\
							"\r\n"\
							"key=%s&format=json" 

#define	OTA_DETECT_KEY		"d1bf23b409a18deb0f00d12295e03a08"

///-------------------------------------------------------------------------------------------------
/// <summary>Check version.</summary>
///
/// <remarks>Tony Wang, 2022/3/29.</remarks>
///
/// <param name="buffer">[in,out] If non-null, the buffer.</param>
///
/// <returns>An uint8_t, 0 for not update, 1 for update.</returns>
///-------------------------------------------------------------------------------------------------

uint8_t Check_Version(uint8_t* buffer)
{
	char* p, * pVersonHigh, * pVersonLow;

	uint8_t version[2];

	p = (void*)0;
	p = strstr((char*)buffer, "\r\n\r\n");
	p = strstr(p, "ver");

	//Something go wrong!
	if (p == NULL)
		return 0;

	p = strstr(p, ":");

	pVersonHigh = (void*)0;
	pVersonHigh = strstr(p, "\"") + 1;

	pVersonLow = (void*)0;
	pVersonLow = strstr(pVersonHigh, ".") + 1;

	memset(version, 0x00, sizeof(version));

	p = strstr(pVersonHigh, ".");
	p = '\0';

	version[1] = atoi((const char*)pVersonHigh);

	p = strstr(pVersonLow, "\"");
	p = '\0';

	version[0] = atoi((const char*)pVersonLow);

	printf("Check_Version <%d.%d>\r\n", version[1], version[0]);

#ifdef BOOT_MODE

	return 1;

#endif

#ifdef APP_MODE

	if (version[1] < FW_VER_HIGH)
		return 0;

	if (version[1] > FW_VER_HIGH)
		return 1;

	if (version[0] <= FW_VER_LOW)
		return 0;

	if (version[0] > FW_VER_LOW)
		return 1;

	return 0;

#endif
}

uint8_t binaryName[64];

void Get_Binary_Name(uint8_t* buffer)
{
	char* p, * pUri;

	uint16_t nameLength = 0;

	p = (void*)0;
	p = strstr((char*)buffer, "url");

	//Something go wrong!
	if (p == NULL)
		return;

	pUri = (void*)0;
	pUri = strstr(p, "upload") + strlen("upload") + 1;

	nameLength = strstr(pUri, "\"") - pUri;

	memset(binaryName, 0x00, sizeof(binaryName));
	memcpy(binaryName, pUri, nameLength);

	binaryName[nameLength] = '\0';

	printf("Get_Binary_Name <%s>\r\n", (char*)binaryName);
}

void OTA_Detect_Version(void)
{
	uint8_t buffer[2048];

	memset(buffer, 0x00, sizeof(buffer));

	//Prepare Http post message.
	sprintf((char*)buffer, OTA_DETECT_MSG, OTA_DETECT_KEY);

	//Send post request.
	send(TCP_CLIENT_SOCKET, buffer, strlen((char*)buffer));

	//Wait for data...
	while (getSn_RX_RSR(TCP_CLIENT_SOCKET) <= 0);

	memset(buffer, 0x00, sizeof(buffer));

	//Receive response.
	recv(TCP_CLIENT_SOCKET, buffer, getSn_RX_RSR(TCP_CLIENT_SOCKET));

	//Process response.

	/*

	Note:

	Response may looks like:

	json:

		{
			"ver":"x.x.x.x",
			"url":"http://www.ibinhub.com/upload/xxxxxxxxxx.xxx",
			"len":"xxx",
			"hash":"xxx",
			"hashvalue":"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
			"info":"xxxxxxxx"
		}

	*/

	Get_Binary_Name(buffer);

	//Wait for disconnect.
	if (Check_Version(buffer))
		otaState = InactivePending;
	else
	{
		printf("App is the latest version.");

		otaState = Normal;
	}
}

#endif 

#define OTA_REQUEST_FIRMWARE_FUNCTION

#ifdef OTA_REQUEST_FIRMWARE_FUNCTION

#define	OTA_UPDATE_MSG		"GET /upload/%s HTTP/1.1\r\n"\
							"Host: ibinhub.com\r\n"\
							"Connection: keep-alive\r\n"\
							"Upgrade-Insecure-Requests: 1\r\n"\
							"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.181 Safari/537.36\r\n"\
							"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"\
							"Accept-Encoding: gzip, deflate\r\n\r\n"

uint32_t binaryLength = 0;

void Get_Binary_Length(uint8_t* buffer)
{
	char* p, * pLength;

	uint8_t tempBuffer[16];

	p = (void*)0;
	p = strstr((char*)buffer, "Content-Length");

	//Something go wrong!
	if (p == NULL)
		return;

	pLength = (void*)0;
	pLength = p + strlen("Content-Length") + 2;

	p = strstr(pLength, "\r\n");

	memset(tempBuffer, 0x00, sizeof(tempBuffer));
	memcpy(tempBuffer, pLength, p - pLength);

	tempBuffer[p - pLength] = '\0';

	binaryLength = atoi((const char*)tempBuffer);

	printf("Get_Binary_Length <%ld>\r\n", binaryLength);
}

void Detect_Firmware_In_First_Packet(uint8_t* buffer, uint32_t length)
{
	char* p;

	uint32_t tempLength = 0;

	p = (void*)0;
	p = strstr((char*)buffer, "\r\n\r\n") + 3;

	//Some data is in first packet.
	if (p - (char*)buffer + 1 < length)
	{
		p += 1;

		tempLength = length - (p - (char*)buffer);

		Pipe_OTA.Clear(&Pipe_OTA);

		for (uint32_t i = 0; i < tempLength; i++)
			Pipe_OTA.Write(&Pipe_OTA, (uint8_t)p[i]);
	}
}

void OTA_Request_Firmware(void)
{
	uint8_t buffer[2048];

	uint32_t length = 0;

	memset(buffer, 0x00, sizeof(buffer));

	//Prepare Http get message.
	sprintf((char*)buffer, OTA_UPDATE_MSG, (char*)binaryName);

	//Send get request.
	send(TCP_CLIENT_SOCKET, buffer, strlen((char*)buffer));

	//Wait for data...
	while (getSn_RX_RSR(TCP_CLIENT_SOCKET) <= 0);

	memset(buffer, 0x00, sizeof(buffer));

	//Receive response.
	length = recv(TCP_CLIENT_SOCKET, buffer, getSn_RX_RSR(TCP_CLIENT_SOCKET));

	//Process response.

	/*

	Note:

	Response may looks like:

	HTTP/1.1 200 OK
	Server: nginx
	Date: Tue, 29 Mar 2022 11:08:59 GMT
	Content-Type: application/octet-stream
	Content-Length: 16432
	Last-Modified: Sat, 26 Mar 2022 12:22:00 GMT
	Connection: keep-alive
	ETag: "623f0568-4030"
	X-Frame-Options: SAMEORIGIN
	Accept-Ranges: bytes

	----------------------------------------------------
	data
	----------------------------------------------------

	*/

	Get_Binary_Length(buffer);

	Detect_Firmware_In_First_Packet(buffer, length);

	otaState = Updating;
}

#endif 

#define OTA_UPDATE_FIRMWARE_FUNCTION

#ifdef OTA_UPDATE_FIRMWARE_FUNCTION

/*
	Area Name					Start					Size
	------------------------------------------------------------
	Bootloader					0x01001000U				0x8000
	Application					0x01009000U				0x1C000
	ApplicationSizeBackup		0x01025000U				0x100
	ApplicationBackup			0x01025100U				0x1C000
*/

#define BootloaderAddress 				0x01001000U

#define ApplicationAddress 				(BootloaderAddress + 0x8000) 

#define ApplicationSizeBackupAddress	(ApplicationAddress + 0x1C000) 

#define ApplicationBackupAddress		(ApplicationSizeBackupAddress+0x100)	

void Jump_To_Area(uint32_t destination)
{
	typedef int (*jumpArea)(void);

	volatile int* ptr = (int*)destination;

	jumpArea area;

	if (*ptr != 0xffffffff)
	{
		area = (jumpArea)(*(__IO uint32_t*)(destination + 4));

		__disable_irq();

		__set_MSP(*(__IO uint32_t*) destination);

		area();
	}
}

void Init_Updating(void)
{
#ifdef BOOT_MODE

	//Erase necessary flash
	Flash_Erase(ApplicationAddress, binaryLength);

#endif 

#ifdef APP_MODE

	uint8_t TempBuffer[X25Q_PAGE_SIZE];

	//Erase necessary flash
	Flash_Erase(ApplicationSizeBackupAddress, binaryLength + 0x100);

	//Write firmware size
	memset(TempBuffer, 0xFF, sizeof(TempBuffer));

	TempBuffer[0] = (uint8_t)((binaryLength & 0xff000000) >> 24);
	TempBuffer[1] = (uint8_t)((binaryLength & 0xff0000) >> 16);
	TempBuffer[2] = (uint8_t)((binaryLength & 0xff00) >> 8);
	TempBuffer[3] = (uint8_t)(binaryLength & 0xff);

	Flash_Write_Page(ApplicationSizeBackupAddress, TempBuffer);

#endif 
}

void Running_Updating(void)
{
	uint8_t Temp = 0;

	uint8_t TempBuffer[X25Q_PAGE_SIZE];

	uint32_t ReceivedLen, WrittenLen, UpdatingArea;

#ifdef BOOT_MODE

	UpdatingArea = ApplicationAddress;

#endif

#ifdef APP_MODE

	UpdatingArea = ApplicationBackupAddress;

#endif

	ReceivedLen = Pipe_OTA.Count(&Pipe_OTA);
	WrittenLen = 0;

	while (WrittenLen != binaryLength)
	{
		//Write data to flash.
		while (Pipe_OTA.Count(&Pipe_OTA) >= X25Q_PAGE_SIZE || ReceivedLen == binaryLength)
		{
			if (Pipe_OTA.Count(&Pipe_OTA) < X25Q_PAGE_SIZE)
			{
				memset(TempBuffer, 0xFF, sizeof(TempBuffer));

				for (uint16_t i = 0; Pipe_OTA.Count(&Pipe_OTA) > 0; i++)
					TempBuffer[i] = Pipe_OTA.Read(&Pipe_OTA);

				Flash_Write_Page(UpdatingArea + WrittenLen, TempBuffer);

				WrittenLen = binaryLength;
			}
			else
			{
				Flash_Write_Page_From_Pipe(UpdatingArea + WrittenLen, &Pipe_OTA);

				WrittenLen += X25Q_PAGE_SIZE;
			}

			printf(">");

			if (WrittenLen == binaryLength)
			{
				printf("\r\nDone\r\n");

				break;
			}
		}

		//Receive data from socket
		while (Pipe_OTA.Full != 1 && getSn_RX_RSR(TCP_CLIENT_SOCKET) > 0 && ReceivedLen < binaryLength)
		{
			recv(TCP_CLIENT_SOCKET, &Temp, 1);

			Pipe_OTA.Write(&Pipe_OTA, Temp);

			ReceivedLen++;
		}
	}
}

void Cleaning_Updating(void)
{
	uint8_t Temp = 0;
	uint16_t Times = 500;

	//Useless message may follow data
	while (Times--)
		while (getSn_RX_RSR(TCP_CLIENT_SOCKET) > 0)
			recv(TCP_CLIENT_SOCKET, &Temp, 1);
}

void OTA_Updating(void)
{
	Init_Updating();

	Running_Updating();

	Cleaning_Updating();

	otaState = Done;
}

#endif 

void OTA_Task_Wait_For_Disconnect(void)
{
	if (otaState == InactivePending)
		otaState = Active;
}

void OTA_Task(void)
{
	switch (otaState)
	{

	case Active:

		OTA_Request_Firmware();

		break;

	case Inactive:

		OTA_Detect_Version();

		break;

	case Updating:

		OTA_Updating();

		break;

	case Done:

#ifdef BOOT_MODE

		//Jump App
		Jump_To_Area(ApplicationAddress);

#endif

#ifdef APP_MODE

		//Jump Boot
		Jump_To_Area(BootloaderAddress);

#endif

		break;

	}
}

#ifdef BOOT_MODE

void Boot_Task(void)
{
	uint8_t DetectData[16];

	uint8_t DataCopy[X25Q_PAGE_SIZE];

	uint32_t tempSize, outputSize;

	printf("Boot_Task is running.\r\n");

	//Detect backup area.
	memset(DetectData, 0x00, sizeof(DetectData));
	memcpy(DetectData, (uint8_t*)(ApplicationBackupAddress), sizeof(DetectData));

	for (uint8_t i = 0; i < sizeof(DetectData); i++)
	{
		//Copy firmware.
		if (DetectData[i] != 0xFF)
		{
			printf("Copying...\r\n");

			//Read size.
			memset(DetectData, 0x00, sizeof(DetectData));
			memcpy(DetectData, (uint8_t*)ApplicationSizeBackupAddress, 4);

			outputSize = 0x00;

			for (uint8_t j = 0; j < 4; j++)
			{
				tempSize = 0x00;
				tempSize |= DetectData[j];
				tempSize <<= (24 - j * 8);

				outputSize |= tempSize;
			}

			//Copy Data.
			Flash_Erase(ApplicationAddress, outputSize);

			for (uint32_t i = 0; i < outputSize; i += X25Q_PAGE_SIZE)
			{
				memcpy(DataCopy, (uint8_t*)(ApplicationBackupAddress + i), X25Q_PAGE_SIZE);

				Flash_Write_Page(ApplicationAddress + i, DataCopy);

				printf(">");
			}

			printf("\r\nDone\r\n");

			//Jump tp app.
			Jump_To_Area(ApplicationAddress);
		}
	}

	//Detect application area.
	memset(DetectData, 0x00, sizeof(DetectData));
	memcpy(DetectData, (uint8_t*)(ApplicationAddress), sizeof(DetectData));

	for (uint8_t i = 0; i < sizeof(DetectData); i++)
	{
		//Normal jump.
		if (DetectData[i] != 0xFF)
			Jump_To_Area(ApplicationAddress);
	}

	//Can't detect any firmware, download from Ethernet.
	printf("No App is found.\r\n");
}

#endif

#ifdef APP_MODE

void App_Task(void)
{
	printf("App_Task is running.\r\n");

	printf("Version V%d.%d\r\n", FW_VER_HIGH, FW_VER_LOW);
}

#endif

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

	setRTR(3000);
	setRCR(30);

	getSIPR(temp);
	printf("COMW-EVB IP : %d.%d.%d.%d\r\n", temp[0], temp[1], temp[2], temp[3]);

	getSUBR(temp);
	printf("COMW-EVB SN : %d.%d.%d.%d\r\n", temp[0], temp[1], temp[2], temp[3]);

	getGAR(temp);
	printf("COMW-EVB GW : %d.%d.%d.%d\r\n", temp[0], temp[1], temp[2], temp[3]);

	setKPALVTR(TCP_CLIENT_SOCKET, 0x02);
}

//OTA platform IP
uint8_t serverIP[4] = { 34,73,14,154 };

//OTA platform Port
uint16_t serverPort = 80;

///-------------------------------------------------------------------------------------------------
/// <summary>TCP Client Process Function.</summary>
///
/// <remarks>Tony Wang, 2021/6/25.</remarks>
///-------------------------------------------------------------------------------------------------

void TCP_Client()
{
	switch (getSn_SR(TCP_CLIENT_SOCKET))
	{

	case SOCK_INIT:

		connect(TCP_CLIENT_SOCKET, serverIP, serverPort);

		break;

	case SOCK_ESTABLISHED:

		if (getSn_IR(TCP_CLIENT_SOCKET) & Sn_IR_CON)
			setSn_IR(TCP_CLIENT_SOCKET, Sn_IR_CON);

		OTA_Task();

		break;

	case SOCK_CLOSE_WAIT:

		close(TCP_CLIENT_SOCKET);

		//Need to wait for disconnect, since these two requset are different.
		OTA_Task_Wait_For_Disconnect();

		break;

	case SOCK_CLOSED:

		socket(TCP_CLIENT_SOCKET, Sn_MR_TCP, 30000, Sn_MR_ND);

		break;

	}
}

int main(void)
{
	__enable_irq();

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

	Pipe_Init();

	printf("Pipe_Init\r\n");

	printf("COMW-EVB AutoUpdate\r\n");

#ifdef BOOT_MODE

	Boot_Task();

#endif

#ifdef APP_MODE

	App_Task();

#endif

	while (1)
		TCP_Client();
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
