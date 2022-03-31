#include <stdio.h>
#include "mhscpu.h"
#include "delay.h"
#include "w5500.h"

/// <summary>	-------------W5500 pin based on MH1903S------------ </summary>

#define W5500_INT_PORT								GPIOC
#define W5500_INT_Pin								GPIO_Pin_12

#define W5500_RESET_PORT							GPIOE
#define W5500_RESET_Pin								GPIO_Pin_3

#define W5500_CS_PORT								GPIOB
#define W5500_CS_Pin								GPIO_Pin_13

/// <summary>	-------------W5500 pin based on MH1903S------------ </summary>

/// <summary>	-----------SPI specific based on MH1903S----------- </summary>

#define W5500SPIInterface_SPIx						SPIM0

/// <summary>	-----------SPI specific based on MH1903S----------- </summary>

/// <summary>	Interrupt Service Routine Disable. </summary>
#define IINCHIP_ISR_DISABLE()						/*taskENTER_CRITICAL()*/

/// <summary>	Interrupt Service Routine Enable. </summary>
#define IINCHIP_ISR_ENABLE()						/*taskEXIT_CRITICAL()*/

static uint8_t I_STATUS[MAX_SOCK_NUM];

/// <summary>	Max Tx buffer size by each channel. </summary>
static uint16_t SSIZE[MAX_SOCK_NUM];

/// <summary>	Max Rx buffer size by each channel . </summary>
static uint16_t RSIZE[MAX_SOCK_NUM];

/// <summary>	-----------SPI function based on MH1903S----------- </summary>

void W5500PIN_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);

	GPIO_InitStruct.GPIO_Pin = W5500_INT_Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;

	GPIO_Init(W5500_INT_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = W5500_RESET_Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;

	GPIO_Init(W5500_RESET_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = W5500_CS_Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;

	GPIO_Init(W5500_CS_PORT, &GPIO_InitStruct);

	GPIO_SetBits(W5500_RESET_PORT, W5500_RESET_Pin);
}

void W5500SPIInterface_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;

	SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO | SYSCTRL_APBPeriph_SPI0, ENABLE);
	SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI0, ENABLE);

	//SPI0
	GPIO_PinRemapConfig(GPIOB, GPIO_Pin_12 /*| GPIO_Pin_13*/ | GPIO_Pin_14 | GPIO_Pin_15, GPIO_Remap_0);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_0;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_RXFIFOFullThreshold = SPI_RXFIFOFullThreshold_1;
	SPI_InitStructure.SPI_TXFIFOEmptyThreshold = SPI_TXFIFOEmptyThreshold_10;

	SPI_Init(W5500SPIInterface_SPIx, &SPI_InitStructure);
	SPI_Cmd(W5500SPIInterface_SPIx, ENABLE);
}

void W5500_Init(void)
{
	W5500PIN_Init();

	W5500SPIInterface_Init();
}

void W5500_Reset(void)
{
	GPIO_ResetBits(W5500_RESET_PORT, W5500_RESET_Pin);

	Delay_us(2);

	GPIO_SetBits(W5500_RESET_PORT, W5500_RESET_Pin);

	Delay_ms(50);
}

void W5500SPIInterface_CS(int state)
{
	if (state)
	{
		GPIO_SetBits(W5500_CS_PORT, W5500_CS_Pin);
	}
	else
	{
		GPIO_ResetBits(W5500_CS_PORT, W5500_CS_Pin);
	}
}

uint8_t W5500SPIInterface_SendByte(uint8_t byte)
{
	uint8_t data;

	SPI_SendData(W5500SPIInterface_SPIx, byte);
	while (RESET == SPI_GetFlagStatus(W5500SPIInterface_SPIx, SPI_FLAG_RXNE));
	data = SPI_ReceiveData(W5500SPIInterface_SPIx);

	return data;
}

/// <summary>	-----------SPI function based on MH1903S----------- </summary>

uint8_t getISR(uint8_t s)
{
	return I_STATUS[s];
}

void putISR(uint8_t s, uint8_t val)
{
	I_STATUS[s] = val;
}

uint16_t getIINCHIP_RxMAX(uint8_t s)
{
	return RSIZE[s];
}

uint16_t getIINCHIP_TxMAX(uint8_t s)
{
	return SSIZE[s];
}

void IINCHIP_CSoff(void)
{
	W5500SPIInterface_CS(0);
}

void IINCHIP_CSon(void)
{
	W5500SPIInterface_CS(1);
}

uint8_t IINCHIP_SpiSendData(uint8_t dat)
{
	return(W5500SPIInterface_SendByte(dat));
}

void IINCHIP_WRITE(uint32_t addrbsb, uint8_t data)
{
	//Interrupt Service Routine Disable
	IINCHIP_ISR_DISABLE();

	//CS=0, SPI start
	IINCHIP_CSoff();

	//Address byte 1
	IINCHIP_SpiSendData((addrbsb & 0x00FF0000) >> 16);

	//Address byte 2
	IINCHIP_SpiSendData((addrbsb & 0x0000FF00) >> 8);

	//Data write command and Write data length 1
	IINCHIP_SpiSendData((addrbsb & 0x000000F8) + 4);

	//Data write (write 1byte data)
	IINCHIP_SpiSendData(data);

	//CS=1,  SPI end
	IINCHIP_CSon();

	//Interrupt Service Routine Enable
	IINCHIP_ISR_ENABLE();
}

uint8_t IINCHIP_READ(uint32_t addrbsb)
{
	uint8_t data = 0;

	//Interrupt Service Routine Disable
	IINCHIP_ISR_DISABLE();

	//CS=0, SPI start
	IINCHIP_CSoff();

	//Address byte 1
	IINCHIP_SpiSendData((addrbsb & 0x00FF0000) >> 16);

	//Address byte 2
	IINCHIP_SpiSendData((addrbsb & 0x0000FF00) >> 8);

	//Data read command and Read data length 1
	IINCHIP_SpiSendData((addrbsb & 0x000000F8));

	//Data read (read 1byte data)
	data = IINCHIP_SpiSendData(0x00);

	//CS=1,  SPI end
	IINCHIP_CSon();

	//Interrupt Service Routine Enable
	IINCHIP_ISR_ENABLE();

	return data;
}

uint16_t wiz_write_buf(uint32_t addrbsb, uint8_t* buf, uint16_t len)
{
	uint16_t idx = 0;

	if (len == 0)
		printf("Unexpected2 length 0\r\n");

	IINCHIP_ISR_DISABLE();

	//CS=0, SPI start
	IINCHIP_CSoff();

	//Address byte 1
	IINCHIP_SpiSendData((addrbsb & 0x00FF0000) >> 16);

	//Address byte 2
	IINCHIP_SpiSendData((addrbsb & 0x0000FF00) >> 8);

	//Data write command and Write data length 1
	IINCHIP_SpiSendData((addrbsb & 0x000000F8) + 4);

	//Write data in loop
	for (idx = 0; idx < len; idx++)
	{
		IINCHIP_SpiSendData(buf[idx]);
	}

	//CS=1, SPI end
	IINCHIP_CSon();

	//Interrupt Service Routine Enable   
	IINCHIP_ISR_ENABLE();

	return len;
}

uint16_t wiz_read_buf(uint32_t addrbsb, uint8_t* buf, uint16_t len)
{
	uint16_t idx = 0;

	if (len == 0)
	{
		printf("Unexpected2 length 0\r\n");
	}

	IINCHIP_ISR_DISABLE();

	//SPI MODE I/F

	//CS=0, SPI start
	IINCHIP_CSoff();

	//Address byte 1
	IINCHIP_SpiSendData((addrbsb & 0x00FF0000) >> 16);

	//Address byte 2
	IINCHIP_SpiSendData((addrbsb & 0x0000FF00) >> 8);

	//Data write command and Write data length 1
	IINCHIP_SpiSendData((addrbsb & 0x000000F8));

	//Write data in loop
	for (idx = 0; idx < len; idx++)
	{
		buf[idx] = IINCHIP_SpiSendData(0x00);
	}

	//CS=1, SPI end
	IINCHIP_CSon();

	//Interrupt Service Routine Enable
	IINCHIP_ISR_ENABLE();

	return len;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function is for resetting of the iinchip. Initializes the iinchip to work in
/// 	whether DIRECT or INDIRECT mode.
/// </summary>
///
/// <remarks>	Tony Wang, 14:57 19/3/21. </remarks>
///-------------------------------------------------------------------------------------------------

void iinchip_init(void)
{
	setMR(MR_RST);

#ifdef __DEF_IINCHIP_DBG__
	printf("MR value is %02x \r\n", IINCHIP_READ_COMMON(MR));
#endif
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function set the transmit &amp; receive buffer size as per the channels is
/// 	used Note for TMSR and RMSR bits are as follows<br/>
/// 	bit 1-0 : memory size of channel #0 <br/>
/// 	bit 3-2 : memory size of channel #1 <br/>
/// 	bit 5-4 : memory size of channel #2 <br/>
/// 	bit 7-6 : memory size of channel #3 <br/>
/// 	bit 9-8 : memory size of channel #4 <br/>
/// 	bit 11-10 : memory size of channel #5 <br/>
/// 	bit 12-12 : memory size of channel #6 <br/>
/// 	bit 15-14 : memory size of channel #7 <br/>
/// 	Maximum memory size for Tx, Rx in the W5500 is 16K Bytes,<br/>
/// 	In the range of 16KBytes, the memory size could be allocated dynamically by each
/// 	channel.<br/>
/// 	Be attentive to sum of memory size shouldn't exceed 8Kbytes<br/>
/// 	and to data transmission and receiption from non-allocated channel may cause some
/// 	problems.<br/>
/// 	If the 16KBytes memory is already  assigned to centain channel, <br/>
/// 	other 3 channels couldn't be used, for there's no available memory.<br/>
/// 	If two 4KBytes memory are assigned to two each channels, <br/>
/// 	other 2 channels couldn't be used, for there's no available memory.<br/>
/// </summary>
///
/// <remarks>	Tony Wang, 14:57 19/3/21. </remarks>
///
/// <param name="tx_size">	[in,out] If non-null, size of the transmit. </param>
/// <param name="rx_size">	[in,out] If non-null, size of the receive. </param>
///-------------------------------------------------------------------------------------------------

void sysinit(uint8_t* tx_size, uint8_t* rx_size)
{
	int16_t i;
	int16_t ssum, rsum;

#ifdef __DEF_IINCHIP_DBG__
	printf("sysinit()\r\n");
#endif

	ssum = 0;
	rsum = 0;

	// Set the size, masking and base address of Tx & Rx memory by each channel
	for (i = 0; i < MAX_SOCK_NUM; i++)
	{
		IINCHIP_WRITE((Sn_TXMEM_SIZE(i)), tx_size[i]);
		IINCHIP_WRITE((Sn_RXMEM_SIZE(i)), rx_size[i]);

#ifdef __DEF_IINCHIP_DBG__
		printf("tx_size[%d]: %d, Sn_TXMEM_SIZE = %d\r\n", i, tx_size[i], IINCHIP_READ(Sn_TXMEM_SIZE(i)));
		printf("rx_size[%d]: %d, Sn_RXMEM_SIZE = %d\r\n", i, rx_size[i], IINCHIP_READ(Sn_RXMEM_SIZE(i)));
#endif

		SSIZE[i] = (int16_t)(0);
		RSIZE[i] = (int16_t)(0);


		if (ssum <= 16384)
		{
			switch (tx_size[i])
			{
			case 1:
				SSIZE[i] = (int16_t)(1024);
				break;
			case 2:
				SSIZE[i] = (int16_t)(2048);
				break;
			case 4:
				SSIZE[i] = (int16_t)(4096);
				break;
			case 8:
				SSIZE[i] = (int16_t)(8192);
				break;
			case 16:
				SSIZE[i] = (int16_t)(16384);
				break;
			default:
				RSIZE[i] = (int16_t)(2048);
				break;
			}
		}

		if (rsum <= 16384)
		{
			switch (rx_size[i])
			{
			case 1:
				RSIZE[i] = (int16_t)(1024);
				break;
			case 2:
				RSIZE[i] = (int16_t)(2048);
				break;
			case 4:
				RSIZE[i] = (int16_t)(4096);
				break;
			case 8:
				RSIZE[i] = (int16_t)(8192);
				break;
			case 16:
				RSIZE[i] = (int16_t)(16384);
				break;
			default:
				RSIZE[i] = (int16_t)(2048);
				break;
			}
		}

		ssum += SSIZE[i];
		rsum += RSIZE[i];
	}
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This function sets up gateway IP address. </summary>
///
/// <remarks>	Tony Wang, 14:57 19/3/21. </remarks>
///
/// <param name="addr">	[in,out] a pointer to a 4 -byte array responsible to set the Gateway IP address. </param>
///-------------------------------------------------------------------------------------------------

void setGAR(uint8_t* addr)
{
	wiz_write_buf(GAR0, addr, 4);
}

void getGWIP(uint8_t* addr)
{
	wiz_read_buf(GAR0, addr, 4);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	It sets up SubnetMask address. </summary>
///
/// <remarks>	Tony Wang, 14:58 19/3/21. </remarks>
///
/// <param name="addr">	[in,out] If non-null, the address. </param>
///-------------------------------------------------------------------------------------------------

void setSUBR(uint8_t* addr)
{
	wiz_write_buf(SUBR0, addr, 4);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	@brief  This function sets up MAC address. </summary>
///
/// <remarks>	Tony Wang, 14:59 19/3/21. </remarks>
///
/// <param name="addr">	[in,out] a pointer to a 6 -byte array responsible to set the MAC address. </param>
///-------------------------------------------------------------------------------------------------

void setSHAR(uint8_t* addr)
{
	wiz_write_buf(SHAR0, addr, 6);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This function sets up Source IP address. </summary>
///
/// <remarks>	Tony Wang, 14:54 19/3/21. </remarks>
///
/// <param name="addr">	[in,out] a pointer to a 4 -byte array responsible to set the Source IP address. </param>
///-------------------------------------------------------------------------------------------------

void setSIPR(uint8_t* addr)
{
	wiz_write_buf(SIPR0, addr, 4);
}

void setDIPR(SOCKET n, uint8_t* addr)
{
	wiz_write_buf(Sn_DIPR0(n), addr, 4);
}

void setDHAR(SOCKET n, uint8_t* addr)
{
	wiz_write_buf(Sn_DHAR0(n), addr, 6);
}

void setDPORT(SOCKET n, uint16_t  port)
{
	IINCHIP_WRITE(Sn_DPORT1(n), (uint8_t)(port & 0x00FF));
	IINCHIP_WRITE(Sn_DPORT0(n), (uint8_t)((port & 0xFF00) >> 8));
}

void setPHAR(uint8_t* addr)
{
	wiz_write_buf(PDHAR0, addr, 6);
}

void getPHAR(uint8_t* addr)
{
	wiz_read_buf(PDHAR0, addr, 6);
}

void setPSID(uint16_t addr)
{
	IINCHIP_WRITE(PSID0, (uint8_t)(addr >> 8));
	IINCHIP_WRITE(PSID1, (uint8_t)addr);
}

void getPSID(uint8_t* addr)
{
	wiz_read_buf(PSID0, addr, 2);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This function sets up Source IP address. </summary>
///
/// <remarks>	Tony Wang, 14:51 19/3/21. </remarks>
///-------------------------------------------------------------------------------------------------

void getGAR(uint8_t* addr)
{
	wiz_read_buf(GAR0, addr, 4);
}

void getSUBR(uint8_t* addr)
{
	wiz_read_buf(SUBR0, addr, 4);
}

void getSHAR(uint8_t* addr)
{
	wiz_read_buf(SHAR0, addr, 6);
}

void getSIPR(uint8_t* addr)
{
	wiz_read_buf(SIPR0, addr, 4);
}

void setMR(uint8_t val)
{
	IINCHIP_WRITE(MR, val);
}

void getMR(void)
{
	IINCHIP_READ(MR);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This function gets Interrupt register in common register. </summary>
///
/// <remarks>	Tony Wang, 14:52 19/3/21. </remarks>
///
/// <returns>	The ir. </returns>
///-------------------------------------------------------------------------------------------------

uint8_t getIR(void)
{
	return IINCHIP_READ(IR);
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function sets up Retransmission time.
/// 	
/// 	If there is no response from the peer or delay in response then retransmission will be
/// 	there as per RTR (Retry Time-value Register)setting.
/// </summary>
///
/// <remarks>	Tony Wang, 14:52 19/3/21. </remarks>
///
/// <param name="timeout">	The timeout. </param>
///-------------------------------------------------------------------------------------------------

void setRTR(uint16_t timeout)
{
	IINCHIP_WRITE(RTR0, (uint8_t)((timeout & 0xff00) >> 8));
	IINCHIP_WRITE(RTR1, (uint8_t)(timeout & 0x00ff));
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function gets Retransmission time.
/// 	
/// 	If there is no response from the peer or delay in response then retransmission will be
/// 	there as per RTR (Retry Time-value Register)setting.
/// </summary>
///
/// <remarks>	Tony Wang, 14:52 19/3/21. </remarks>
///
/// <returns>	The rtr. </returns>
///-------------------------------------------------------------------------------------------------

uint16_t getRTR(void)
{
	return (IINCHIP_READ(RTR0) << 8) | IINCHIP_READ(RTR1);
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function set the number of Retransmission.
/// 	
/// 	If there is no response from the peer or delay in response then recorded time as per RTR
/// 	&amp; RCR register seeting then time out will occur.
/// </summary>
///
/// <remarks>	Tony Wang, 14:52 19/3/21. </remarks>
///
/// <param name="retry">	The retry. </param>
///-------------------------------------------------------------------------------------------------

void setRCR(uint8_t retry)
{
	IINCHIP_WRITE(WIZ_RCR, retry);
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function get the number of Retransmission.
/// 	
/// 	If there is no response from the peer or delay in response then recorded time as per RTR
/// 	&amp; RCR register seeting then time out will occur.
/// </summary>
///
/// <remarks>	Tony Wang, 14:53 19/3/21. </remarks>
///
/// <returns>	The rcr. </returns>
///-------------------------------------------------------------------------------------------------

uint8_t getRCR(void)
{
	return IINCHIP_READ(WIZ_RCR);
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function set the interrupt mask Enable/Disable appropriate Interrupt. ('1' :
/// 	interrupt enable)
/// 	
/// 	If any bit in IMR is set as '0' then there is not interrupt signal though the bit is set
/// 	in IR register.
/// </summary>
///
/// <remarks>	Tony Wang, 14:53 19/3/21. </remarks>
///
/// <param name="mask">	The mask. </param>
///-------------------------------------------------------------------------------------------------

void clearIR(uint8_t mask)
{
	IINCHIP_WRITE(IR, ~mask | getIR()); // must be setted 0x10.
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This sets the maximum segment size of TCP in Active Mode), while in Passive Mode
/// 	this is set by peer.
/// </summary>
///
/// <remarks>	Tony Wang, 14:53 19/3/21. </remarks>
///
/// <param name="s">	  	A SOCKET to process. </param>
/// <param name="Sn_MSSR">	The serial number mssr. </param>
///-------------------------------------------------------------------------------------------------

void setSn_MSS(SOCKET s, uint16_t Sn_MSSR)
{
	IINCHIP_WRITE(Sn_MSSR0(s), (uint8_t)((Sn_MSSR & 0xff00) >> 8));
	IINCHIP_WRITE(Sn_MSSR1(s), (uint8_t)(Sn_MSSR & 0x00ff));
}

void setSn_TTL(SOCKET s, uint8_t ttl)
{
	IINCHIP_WRITE(Sn_TTL(s), ttl);
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	get socket interrupt status
/// 	
/// 	These below functions are used to read the Interrupt &amp; Soket Status register.
/// </summary>
///
/// <remarks>	Tony Wang, 14:53 19/3/21. </remarks>
///
/// <param name="s">	A SOCKET to process. </param>
///
/// <returns>	The serial number ir. </returns>
///-------------------------------------------------------------------------------------------------

uint8_t getSn_IR(SOCKET s)
{
	return IINCHIP_READ(Sn_IR(s));
}

///-------------------------------------------------------------------------------------------------
/// <summary>	get socket status. </summary>
///
/// <remarks>	Tony Wang, 14:53 19/3/21. </remarks>
///
/// <param name="s">	A SOCKET to process. </param>
///
/// <returns>	The serial number sr. </returns>
///-------------------------------------------------------------------------------------------------

uint8_t getSn_SR(SOCKET s)
{
	return IINCHIP_READ(Sn_SR(s));
}

uint8_t getSn_CR(SOCKET s)
{
	return IINCHIP_READ(Sn_CR(s));
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	get socket TX free buf size
/// 	
/// 	This gives free buffer size of transmit buffer. This is the data size that user can
/// 	transmit. User shuold check this value first and control the size of transmitting data.
/// </summary>
///
/// <remarks>	Tony Wang, 14:53 19/3/21. </remarks>
///
/// <param name="s">	A SOCKET to process. </param>
///
/// <returns>	The serial number transmit fsr. </returns>
///-------------------------------------------------------------------------------------------------

uint16_t getSn_TX_FSR(SOCKET s)
{
	uint16_t val = 0, val1 = 0;

	do
	{
		val1 = IINCHIP_READ(Sn_TX_FSR0(s));
		val1 = (val1 << 8) + IINCHIP_READ(Sn_TX_FSR1(s));

		if (val1 != 0)
		{
			val = IINCHIP_READ(Sn_TX_FSR0(s));
			val = (val << 8) + IINCHIP_READ(Sn_TX_FSR1(s));
		}

	} while (val != val1);

	return val;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	get socket RX recv buf size
/// 	
/// 	This gives size of received data in receive buffer.
/// </summary>
///
/// <remarks>	Tony Wang, 14:53 19/3/21. </remarks>
///
/// <param name="s">	A SOCKET to process. </param>
///
/// <returns>	The serial number receive rsr. </returns>
///-------------------------------------------------------------------------------------------------

uint16_t getSn_RX_RSR(SOCKET s)
{
	uint16_t val = 0, val1 = 0;

	do
	{
		val1 = IINCHIP_READ(Sn_RX_RSR0(s));
		val1 = (val1 << 8) + IINCHIP_READ(Sn_RX_RSR1(s));

		if (val1 != 0)
		{
			val = IINCHIP_READ(Sn_RX_RSR0(s));
			val = (val << 8) + IINCHIP_READ(Sn_RX_RSR1(s));
		}

	} while (val != val1);

	return val;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function is being called by send() and sendto() function also.
/// 	
/// 	This function read the Tx write pointer register and after copy the data in buffer update
/// 	the Tx write pointer register. User should read upper byte first and lower byte later to
/// 	get proper value.
/// </summary>
///
/// <remarks>	Tony Wang, 14:54 19/3/21. </remarks>
///
/// <param name="s">   	A SOCKET to process. </param>
/// <param name="data">	[in,out] If non-null, the data. </param>
/// <param name="len"> 	The length. </param>
///-------------------------------------------------------------------------------------------------

void send_data_processing(SOCKET s, uint8_t* data, uint16_t len)
{
	uint16_t ptr = 0;
	uint32_t addrbsb = 0;

	if (len == 0)
	{
		printf("CH: %d Unexpected1 length 0\r\n", s);
		return;
	}


	ptr = IINCHIP_READ(Sn_TX_WR0(s));
	ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_TX_WR1(s));

	addrbsb = (uint32_t)(ptr << 8) + (s << 5) + 0x10;
	wiz_write_buf(addrbsb, data, len);

	ptr += len;
	IINCHIP_WRITE(Sn_TX_WR0(s), (uint8_t)((ptr & 0xff00) >> 8));
	IINCHIP_WRITE(Sn_TX_WR1(s), (uint8_t)(ptr & 0x00ff));
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function is being called by recv() also.
/// 	
/// 	This function read the Rx read pointer register and after copy the data from receive
/// 	buffer update the Rx write pointer register. User should read upper byte first and lower
/// 	byte later to get proper value.
/// </summary>
///
/// <remarks>	Tony Wang, 14:54 19/3/21. </remarks>
///
/// <param name="s">   	A SOCKET to process. </param>
/// <param name="data">	[in,out] If non-null, the data. </param>
/// <param name="len"> 	The length. </param>
///-------------------------------------------------------------------------------------------------

void recv_data_processing(SOCKET s, uint8_t* data, uint16_t len)
{
	uint16_t ptr = 0;
	uint32_t addrbsb = 0;

	if (len == 0)
	{
		printf("CH: %d Unexpected2 length 0\r\n", s);
		return;
	}

	ptr = IINCHIP_READ(Sn_RX_RD0(s));
	ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_RX_RD1(s));

	addrbsb = (uint32_t)(ptr << 8) + (s << 5) + 0x18;
	wiz_read_buf(addrbsb, data, len);
	ptr += len;

	IINCHIP_WRITE(Sn_RX_RD0(s), (uint8_t)((ptr & 0xff00) >> 8));
	IINCHIP_WRITE(Sn_RX_RD1(s), (uint8_t)(ptr & 0x00ff));
}

void setSn_IR(uint8_t s, uint8_t val)
{
	IINCHIP_WRITE(Sn_IR(s), val);
}

uint8_t getPHYStatus(void)
{
	return IINCHIP_READ(PHYCFGR);;
}

void setKPALVTR(SOCKET i, uint8_t val)
{
	IINCHIP_WRITE(Sn_KPALVTR(i), val);
}
