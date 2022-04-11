#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mhscpu.h"
#include "delay.h"
#include "W5500.h"
#include "socket.h"
#include "upnp.h"
#include "pipe.h"
#include "usart.h"
#include "hyperterminal.h"

/*******************************************************************************
* Function Name  : Int2Str
* Description    : Convert an Integer to a string
* Input          : - str: The string
*                  - intnum: The intger to be converted
* Output         : None
* Return         : None
*******************************************************************************/
void Int2Str(char* str, uint32_t intnum)
{
	int i, Div = 1000000000, j = 0, Status = 0;

	for (i = 0; i < 10; i++)
	{
		str[j++] = (intnum / Div) + 48;
		intnum = intnum % Div;
		Div /= 10;
		if ((str[j - 1] == '0') & (Status == 0))
		{
			j = 0;
		}
		else
		{
			Status++;
		}
	}
}

/*******************************************************************************
* Function Name  : Str2Int
* Description    : Convert a string to an integer
* Input 1        : - inputstr: The string to be converted
*                  - intnum: The intger value
* Output         : None
* Return         : 1: Correct
*                  0: Error
*******************************************************************************/
uint8_t Str2Int(char* inputstr, uint32_t* intnum)
{
	uint8_t i = 0, res = 0;
	uint32_t val = 0;

	if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X'))
	{
		if (inputstr[2] == '\0')
		{
			return 0;
		}
		for (i = 2; i < 11; i++)
		{
			if (inputstr[i] == '\0')
			{
				*intnum = val;
				res = 1; /* return 1; */
				break;
			}
			if (ISVALIDHEX(inputstr[i]))
			{
				val = (val << 4) + CONVERTHEX(inputstr[i]);
			}
			else
			{
				/* return 0; Invalid input */
				res = 0;
				break;
			}
		}
		if (i >= 11) res = 0; /* over 8 digit hex --invalid */
	}
	else /* max 10-digit decimal input */
	{
		for (i = 0; i < 11; i++)
		{
			if (inputstr[i] == '\0')
			{
				*intnum = val;
				/* return 1; */
				res = 1;
				break;
			}
			else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0))
			{
				val = val << 10;
				*intnum = val;
				res = 1;
				break;
			}
			else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0))
			{
				val = val << 20;
				*intnum = val;
				res = 1;
				break;
			}
			else if (ISVALIDDEC(inputstr[i]))
				val = val * 10 + CONVERTDEC(inputstr[i]);
			else
			{
				/* return 0; Invalid input */
				res = 0;
				break;
			}
		}
		if (i >= 11) res = 0; /* Over 10 digit decimal --invalid */
	}
	return res;
}

/*******************************************************************************
* Function Name  : GetIntegerInput
* Description    : Get an integer from the HyperTerminal
* Input          : - num: The inetger
* Output         : None
* Return         : 1: Correct
*                  0: Error
*******************************************************************************/
uint8_t GetIntegerInput(uint32_t* num)
{
	char inputstr[16];
	while (1)
	{
		GetInputString(inputstr);
		if (inputstr[0] == '\0') continue;
		if ((inputstr[0] == 'a' || inputstr[0] == 'A') && inputstr[1] == '\0')
		{
			SerialPutString("User Cancelled \r\n");
			return 0;
		}
		if (Str2Int(inputstr, num) == 0)
		{
			SerialPutString("Error, Input again: \r\n");
		}
		else
		{
			return 1;
		}
	}
}

/*******************************************************************************
* Function Name  : SerialPutChar
* Description    : Print a character on the HyperTerminal
* Input          : - c: The character to be printed
* Output         : None
* Return         : None
*******************************************************************************/
void SerialPutChar(char c)
{
	USART_Send((uint8_t)c);
}

/*******************************************************************************
* Function Name  : SerialPutString
* Description    : Print a string on the HyperTerminal
* Input          : - s: The string to be printed
* Output         : None
* Return         : None
*******************************************************************************/
void SerialPutString(char* s)
{
	while (*s != '\0')
	{
		SerialPutChar(*s);
		s++;
	}
}

/*******************************************************************************
* Function Name  : GetInputString
* Description    : Get Input string from the HyperTerminal
* Input          : - buffP: The input string
* Output         : None
* Return         : None
*******************************************************************************/
void GetInputString(char* buffP)
{
	uint16_t i = 0;

	while (Pipe_Usart.Count(&Pipe_Usart))
		buffP[i++] = Pipe_Usart.Read(&Pipe_Usart);
}

char* STRTOK(char* strToken, const char* strDelimit)
{
	static char* pCurrent;
	char* pDelimit;

	if (strToken != NULL)
		pCurrent = strToken;
	else
		strToken = pCurrent;

	while (*pCurrent)
	{
		pDelimit = (char*)strDelimit;

		while (*pDelimit)
		{
			if (*pCurrent == *pDelimit)
			{
				//*pCurrent = NULL;
				*pCurrent = 0;
				++pCurrent;

				return strToken;
			}

			++pDelimit;
		}

		++pCurrent;
	}

	return strToken;
}

uint8_t Check_Input_Ending(void)
{
	if (Pipe_Usart.Count(&Pipe_Usart) > 2)
		if (Pipe_Usart.Buffer[Pipe_Usart.WriteOffset == 0 ? Pipe_Usart.Size - 1 : Pipe_Usart.WriteOffset - 1] == '\n')
			if (Pipe_Usart.Buffer[Pipe_Usart.WriteOffset == 1 ? Pipe_Usart.Size - 1 : Pipe_Usart.WriteOffset - 2] == '\r')
				return 1;

	return 0;
}

typedef enum _DISPLAY_STATUS
{
	Display_Menu, Wait_Input
}DISPLAY_STATUS;

DISPLAY_STATUS menuStatus = Display_Menu;

/*******************************************************************************
* Function Name  : Main_Menu
* Description    : Display/Manage a Menu on HyperTerminal Window
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Main_Menu(void)
{
	uint8_t choice = 0;
	char msg[256], ipaddr[12], protocol[4];
	unsigned short ret, external_port, internal_port;
	uint8_t tempIp[4];

	switch (menuStatus)
	{

	case Display_Menu:

		SerialPutString("\r\n====================== W5500_Control_Point ===================\r\n");
		SerialPutString("This Application is basic example of UART interface with\r\n");
		SerialPutString("Windows Hyper Terminal. \r\n");
		SerialPutString("\r\n==============================================================\r\n");
		SerialPutString("                          APPLICATION MENU :");
		SerialPutString("\r\n==============================================================\r\n");
		SerialPutString(" 1 - Show network setting\r\n");
		SerialPutString(" 2 - UPnP PortForwarding: AddPort\r\n");
		SerialPutString(" 3 - UPnP PortForwarding: DeletePort\r\n\r\n");

		SerialPutString("Enter your choice : (end with \\r\\n)\r\n");

		menuStatus = Wait_Input;

		break;

	case Wait_Input:

		if (Check_Input_Ending())
		{
			//Process command

			memset(msg, 0x00, sizeof(msg));
			GetInputString(msg);

			//Check input
			if (strlen(msg) == 3)
			{
				msg[1] = '\0';

				choice = atoi(msg);

				switch (choice)
				{

				case 1:

					memset(tempIp, 0x00, sizeof(tempIp));

					getSIPR(tempIp);
					printf("IP : %d.%d.%d.%d\r\n",
						tempIp[0],
						tempIp[1],
						tempIp[2],
						tempIp[3]);

					getSUBR(tempIp);
					printf("SN : %d.%d.%d.%d\r\n",
						tempIp[0],
						tempIp[1],
						tempIp[2],
						tempIp[3]);

					getGAR(tempIp);
					printf("GW : %d.%d.%d.%d\r\n",
						tempIp[0],
						tempIp[1],
						tempIp[2],
						tempIp[3]);

					break;

				case 2:

					SerialPutString("\r\nType a Protocol(TCP/UDP) : (end with \\r\\n)");

					while (!Check_Input_Ending());

					memset(msg, 0x00, sizeof(msg));
					GetInputString(msg);

					if (strstr(msg, "TCP") == NULL && strstr(msg, "UDP") == NULL)
						SerialPutString("Invaild Input\r\n");
					else
					{
						memset(protocol, 0x00, sizeof(protocol));

						if (strstr(msg, "TCP") != NULL)
							sprintf(protocol, "TCP");

						if (strstr(msg, "UDP") != NULL)
							sprintf(protocol, "UDP");

						SerialPutString("\r\nType a External Port Number : (end with \\r\\n)");

						while (!Check_Input_Ending());

						memset(msg, 0x00, sizeof(msg));
						GetInputString(msg);

						external_port = atoi(msg);

						SerialPutString("\r\nType a Internal Port Number : (end with \\r\\n)\r\n");

						while (!Check_Input_Ending());

						memset(msg, 0x00, sizeof(msg));
						GetInputString(msg);

						internal_port = atoi(msg);

						getSIPR(tempIp);
						sprintf(ipaddr, "%d.%d.%d.%d",
							tempIp[0],
							tempIp[1],
							tempIp[2],
							tempIp[3]);

						// Try to Add Port Action
						if ((ret = AddPortProcess(SOCK_UPNP, protocol, external_port, ipaddr, internal_port, "W5500_uPnPGetway")) == 0)
						{
							printf("AddPort Success!!\r\n");

							close(SOCK_LOOPBACK);
							while (getSn_SR(SOCK_LOOPBACK) != SOCK_CLOSED);

							if (strstr(protocol, "TCP") != NULL)
							{
								socket(SOCK_LOOPBACK, Sn_MR_TCP, internal_port, Sn_MR_ND);
								while (getSn_SR(SOCK_LOOPBACK) != SOCK_INIT);
							}
							else
							{
								socket(SOCK_LOOPBACK, Sn_MR_UDP, internal_port, 0x00);
								while (getSn_SR(SOCK_LOOPBACK) != SOCK_UDP);
							}

							printf("LoopBack is running.(%d.%d.%d.%d:%d External port:%d %s)\r\n",
								tempIp[0],
								tempIp[1],
								tempIp[2],
								tempIp[3],
								internal_port,
								external_port,
								protocol
							);
						}
						else
							printf("AddPort Error Code is %d\r\n", ret);
					}

					break;

				case 3:

					SerialPutString("\r\nType a Protocol(TCP/UDP) : (end with \\r\\n)");

					while (!Check_Input_Ending());

					memset(msg, 0x00, sizeof(msg));
					GetInputString(msg);

					if (strstr(msg, "TCP") == NULL && strstr(msg, "UDP") == NULL)
						SerialPutString("Invaild Input\r\n");
					else
					{
						memset(protocol, 0x00, sizeof(protocol));

						if (strstr(msg, "TCP") != NULL)
							sprintf(protocol, "TCP");

						if (strstr(msg, "UDP") != NULL)
							sprintf(protocol, "UDP");

						SerialPutString("\r\nType a External Port Number : (end with \\r\\n)\r\n");

						while (!Check_Input_Ending());

						memset(msg, 0x00, sizeof(msg));
						GetInputString(msg);

						external_port = atoi(msg);

						// Try to Delete Port Action
						if ((ret = DeletePortProcess(SOCK_UPNP, protocol, external_port)) == 0)
						{
							printf("DeletePort Success!!\r\n");

							if (strstr(protocol, "TCP") != NULL)
								getSn_SR(SOCK_LOOPBACK) == SOCK_ESTABLISHED ? \
								disconnect(SOCK_LOOPBACK) : \
								close(SOCK_LOOPBACK);
							else
								close(SOCK_LOOPBACK);

							printf("LoopBack is stop\r\n");
						}
						else
							printf("DeletePort Error Code is %d\r\n", ret);
					}

					break;

				default:

					SerialPutString("Invaild Input\r\n");

					break;

				}
			}

			menuStatus = Display_Menu;
		}

		break;

	}
}

void Loop_Back_Data_Process(void)
{
	uint8_t  peerIP[4], tempBuffer[256], tempLength = 0;
	uint16_t peerPort;

	tempLength = getSn_RX_RSR(SOCK_LOOPBACK);

	if (tempLength)
	{
		memset(tempBuffer, 0x00, sizeof(tempBuffer));

		if (getSn_SR(SOCK_LOOPBACK) == SOCK_UDP)
			tempLength = recvfrom(SOCK_LOOPBACK, tempBuffer, tempLength, peerIP, &peerPort);
		else
			tempLength = recv(SOCK_LOOPBACK, tempBuffer, tempLength);

		if (tempLength)
		{
			printf("LoopBack Receive:\r\n");

			for (uint8_t i = 0; i < tempLength; i++)
				printf("%c", (char)tempBuffer[i]);

			printf("\r\n");

			getSn_SR(SOCK_LOOPBACK) == SOCK_UDP ? \
				sendto(SOCK_LOOPBACK, tempBuffer, tempLength, peerIP, peerPort) : \
				send(SOCK_LOOPBACK, tempBuffer, tempLength);
		}
	}
}

void Loop_Back_Task(void)
{
	switch (getSn_SR(SOCK_LOOPBACK))
	{

	case SOCK_UDP:

		if (getSn_IR(SOCK_LOOPBACK) & Sn_IR_RECV)
			setSn_IR(SOCK_LOOPBACK, Sn_IR_RECV);

		Loop_Back_Data_Process();

		break;

	case SOCK_INIT:

		listen(SOCK_LOOPBACK);

		break;

	case SOCK_ESTABLISHED:

		if (getSn_IR(SOCK_LOOPBACK) & Sn_IR_CON)
			setSn_IR(SOCK_LOOPBACK, Sn_IR_CON);

		Loop_Back_Data_Process();

		break;

	case SOCK_CLOSE_WAIT:

		disconnect(SOCK_LOOPBACK);

		break;

	case SOCK_FIN_WAIT:
	case SOCK_CLOSED:

		close(SOCK_LOOPBACK);

		break;

	}
}

void Control_Point_Task(void)
{
	Main_Menu();

	eventing_listener(SOCK_UPNP_EVENTING);

	Loop_Back_Task();
}
