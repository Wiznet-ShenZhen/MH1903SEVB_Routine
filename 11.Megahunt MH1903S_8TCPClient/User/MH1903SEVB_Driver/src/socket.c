#include <stdio.h>
#include "w5500.h"
#include "socket.h"

static uint16_t local_port;

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This Socket function initialize the channel in particular mode, and set the port
/// 	and wait for W5200 done it.
/// </summary>
///
/// <remarks>	Tony Wang, 16:23 19/3/21. </remarks>
///
/// <param name="s">	   	The socket to process. </param>
/// <param name="protocol">	The protocol. </param>
/// <param name="port">	   	The port. </param>
/// <param name="flag">	   	The flag. </param>
///
/// <returns>	1 for success else 0. </returns>
///-------------------------------------------------------------------------------------------------

uint8_t socket(SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag)
{
	uint8_t ret;

	if (
		((protocol & 0x0F) == Sn_MR_TCP) ||
		((protocol & 0x0F) == Sn_MR_UDP) ||
		((protocol & 0x0F) == Sn_MR_IPRAW) ||
		((protocol & 0x0F) == Sn_MR_MACRAW) ||
		((protocol & 0x0F) == Sn_MR_PPPOE)
		)
	{
		close(s);

		IINCHIP_WRITE(Sn_MR(s), protocol | flag);

		if (port != 0)
		{
			IINCHIP_WRITE(Sn_PORT0(s), (uint8_t)((port & 0xff00) >> 8));

			IINCHIP_WRITE(Sn_PORT1(s), (uint8_t)(port & 0x00ff));
		}
		else
		{
			//if don't set the source port, set local_port number.
			local_port++;

			IINCHIP_WRITE(Sn_PORT0(s), (uint8_t)((local_port & 0xff00) >> 8));

			IINCHIP_WRITE(Sn_PORT1(s), (uint8_t)(local_port & 0x00ff));
		}

		//run sockinit Sn_CR
		IINCHIP_WRITE(Sn_CR(s), Sn_CR_OPEN);

		//wait to process the command... 
		while (IINCHIP_READ(Sn_CR(s)));

		ret = 1;
	}
	else
	{
		ret = 0;
	}

	return ret;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function close the socket and parameter is "s" which represent the socket
/// 	number.
/// </summary>
///
/// <remarks>	Tony Wang, 16:23 19/3/21. </remarks>
///
/// <param name="s">	The socket to process. </param>
///-------------------------------------------------------------------------------------------------

void close(SOCKET s)
{
	IINCHIP_WRITE(Sn_CR(s), Sn_CR_CLOSE);

	//wait to process the command... 
	while (IINCHIP_READ(Sn_CR(s)));

	//all clear 
	IINCHIP_WRITE(Sn_IR(s), 0xFF);
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function established  the connection for the channel in passive (server)
/// 	mode. This function waits for the request from the peer.
/// </summary>
///
/// <remarks>	Tony Wang, 16:23 19/3/21. </remarks>
///
/// <param name="s">	The socket to process. </param>
///
/// <returns>	1 for success else 0. </returns>
///-------------------------------------------------------------------------------------------------

uint8_t listen(SOCKET s)
{
	uint8_t ret;

	if (IINCHIP_READ(Sn_SR(s)) == SOCK_INIT)
	{
		IINCHIP_WRITE(Sn_CR(s), Sn_CR_LISTEN);

		//wait to process the command... 
		while (IINCHIP_READ(Sn_CR(s)));

		ret = 1;
	}
	else
	{
		ret = 0;
	}

	return ret;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function established  the connection for the channel in Active (client)
/// 	mode.
/// 	This function waits for the until the connection is established.
/// </summary>
///
/// <remarks>	Tony Wang, 16:24 19/3/21. </remarks>
///
/// <param name="s">   	The socket to process. </param>
/// <param name="addr">	[in,out] If non-null, the address. </param>
/// <param name="port">	The port. </param>
///
/// <returns>	1 for success else 0. </returns>
///-------------------------------------------------------------------------------------------------

uint8_t connect(SOCKET s, uint8_t* addr, uint16_t port)
{
	uint8_t ret;

	if (((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
		((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		(port == 0x00))
	{
		ret = 0;
	}
	else
	{
		ret = 1;

		//set destination IP
		IINCHIP_WRITE(Sn_DIPR0(s), addr[0]);
		IINCHIP_WRITE(Sn_DIPR1(s), addr[1]);
		IINCHIP_WRITE(Sn_DIPR2(s), addr[2]);
		IINCHIP_WRITE(Sn_DIPR3(s), addr[3]);

		IINCHIP_WRITE(Sn_DPORT0(s), (uint8_t)((port & 0xff00) >> 8));
		IINCHIP_WRITE(Sn_DPORT1(s), (uint8_t)(port & 0x00ff));

		IINCHIP_WRITE(Sn_CR(s), Sn_CR_CONNECT);

		//wait for completion 
		while (IINCHIP_READ(Sn_CR(s)));

		while (IINCHIP_READ(Sn_SR(s)) != SOCK_SYNSENT)
		{
			if (IINCHIP_READ(Sn_SR(s)) == SOCK_ESTABLISHED)
			{
				break;
			}
			if (getSn_IR(s) & Sn_IR_TIMEOUT)
			{
				//clear TIMEOUT Interrupt
				IINCHIP_WRITE(Sn_IR(s), (Sn_IR_TIMEOUT));

				ret = 0;

				break;
			}
		}
	}

	return ret;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function used for disconnect the socket and parameter is "s" which
/// 	represent the socket number.
/// </summary>
///
/// <remarks>	Tony Wang, 16:24 19/3/21. </remarks>
///
/// <param name="s">	The socket to disconnect. </param>
///-------------------------------------------------------------------------------------------------

void disconnect(SOCKET s)
{
	IINCHIP_WRITE(Sn_CR(s), Sn_CR_DISCON);

	//wait to process the command... 
	while (IINCHIP_READ(Sn_CR(s)));
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function used to send the data in TCP mode.
/// </summary>
///
/// <remarks>	Tony Wang, 16:24 19/3/21. </remarks>
///
/// <param name="s">  	The socket to process. </param>
/// <param name="buf">	The buffer. </param>
/// <param name="len">	The length. </param>
///
/// <returns>	Sum of bytes sent. </returns>
///-------------------------------------------------------------------------------------------------

uint16_t send(SOCKET s, const uint8_t* buf, uint16_t len)
{
	uint8_t status = 0;

	uint16_t ret = 0;

	uint16_t freesize = 0;

	//check size not to exceed MAX size.
	if (len > getIINCHIP_TxMAX(s))
		ret = getIINCHIP_TxMAX(s);
	else
		ret = len;

	//if freebuf is available, start.
	do
	{
		freesize = getSn_TX_FSR(s);

		status = IINCHIP_READ(Sn_SR(s));

		if ((status != SOCK_ESTABLISHED) && (status != SOCK_CLOSE_WAIT))
		{
			ret = 0;

			break;
		}
	} while (freesize < ret);


	//copy data
	send_data_processing(s, (uint8_t*)buf, ret);

	IINCHIP_WRITE(Sn_CR(s), Sn_CR_SEND);

	//wait to process the command... 
	while (IINCHIP_READ(Sn_CR(s)));

	while ((IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK)
	{
		status = IINCHIP_READ(Sn_SR(s));

		if ((status != SOCK_ESTABLISHED) && (status != SOCK_CLOSE_WAIT))
		{
			close(s);

			return 0;
		}
	}

	IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);

#ifdef __DEF_IINCHIP_INT__
	putISR(s, getISR(s) & (~Sn_IR_SEND_OK));
#else
	IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
#endif

	return ret;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function is an application I/F function which is used to receive the data
/// 	in TCP mode.
///		It continues to wait for data as much as the application wants to receive.
/// </summary>
///
/// <remarks>	Tony Wang, 16:24 19/3/21. </remarks>
///
/// <param name="s">  	The socket to process. </param>
/// <param name="buf">	[in,out] If non-null, the buffer. </param>
/// <param name="len">	The length. </param>
///
/// <returns>	Received data size for success else -1. </returns>
///-------------------------------------------------------------------------------------------------

uint16_t recv(SOCKET s, uint8_t* buf, uint16_t len)
{
	uint16_t ret = 0;

	if (len > 0)
	{
		recv_data_processing(s, buf, len);

		IINCHIP_WRITE(Sn_CR(s), Sn_CR_RECV);

		//wait to process the command... 
		while (IINCHIP_READ(Sn_CR(s)));

		ret = len;
	}

	return ret;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function is an application I/F function which is used to send the data for
/// 	other then TCP mode.
/// 	Unlike TCP transmission, The peer's destination address and the port is needed.
/// </summary>
///
/// <remarks>	Tony Wang, 16:24 19/3/21. </remarks>
///
/// <param name="s">   	The socket to process. </param>
/// <param name="buf"> 	The buffer. </param>
/// <param name="len"> 	The length. </param>
/// <param name="addr">	[in,out] If non-null, the address. </param>
/// <param name="port">	The port. </param>
///
/// <returns>	This function return send data size for success else -1. </returns>
///-------------------------------------------------------------------------------------------------

uint16_t sendto(SOCKET s, const uint8_t* buf, uint16_t len, uint8_t* addr, uint16_t port)
{
	uint16_t ret = 0;

	//check size not to exceed MAX size.
	if (len > getIINCHIP_TxMAX(s))
		ret = getIINCHIP_TxMAX(s);
	else
		ret = len;

	if (((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) || ((port == 0x00)))//||(ret == 0) )
	{
		//added return value 
		ret = 0;
	}
	else
	{
		IINCHIP_WRITE(Sn_DIPR0(s), addr[0]);
		IINCHIP_WRITE(Sn_DIPR1(s), addr[1]);
		IINCHIP_WRITE(Sn_DIPR2(s), addr[2]);
		IINCHIP_WRITE(Sn_DIPR3(s), addr[3]);

		IINCHIP_WRITE(Sn_DPORT0(s), (uint8_t)((port & 0xff00) >> 8));
		IINCHIP_WRITE(Sn_DPORT1(s), (uint8_t)(port & 0x00ff));

		//copy data
		send_data_processing(s, (uint8_t*)buf, ret);

		IINCHIP_WRITE(Sn_CR(s), Sn_CR_SEND);

		//wait to process the command... 
		while (IINCHIP_READ(Sn_CR(s)));

		while ((IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK)
		{
			if (IINCHIP_READ(Sn_IR(s)) & Sn_IR_TIMEOUT)
			{
				//clear interrupt 
				IINCHIP_WRITE(Sn_IR(s), (Sn_IR_SEND_OK | Sn_IR_TIMEOUT));

				return 0;
			}
		}

		IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
	}

	return ret;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	This function is an application I/F function which is used to receive the data
/// 	in other then TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW 
/// 	mode, and handle the header as well.
/// </summary>
///
/// <remarks>	Tony Wang, 16:25 19/3/21. </remarks>
///
/// <param name="s">   	The socket to process. </param>
/// <param name="buf"> 	[in,out] If non-null, the buffer. </param>
/// <param name="len"> 	The length. </param>
/// <param name="addr">	[in,out] If non-null, the address. </param>
/// <param name="port">	[in,out] If non-null, the port. </param>
///
/// <returns>	This function return received data size for success else -1. </returns>
///-------------------------------------------------------------------------------------------------

uint16_t recvfrom(SOCKET s, uint8_t* buf, uint16_t len, uint8_t* addr, uint16_t* port)
{
	uint8_t head[8];

	uint16_t data_len = 0;

	uint16_t ptr = 0;

	uint32_t addrbsb = 0;

	if (len > 0)
	{
		ptr = IINCHIP_READ(Sn_RX_RD0(s));

		ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_RX_RD1(s));

		addrbsb = (uint32_t)(ptr << 8) + (s << 5) + 0x18;

		switch (IINCHIP_READ(Sn_MR(s)) & 0x07)
		{
		case Sn_MR_UDP:

			wiz_read_buf(addrbsb, head, 0x08);

			ptr += 8;

			//read peer's IP address, port number.
			addr[0] = head[0];
			addr[1] = head[1];
			addr[2] = head[2];
			addr[3] = head[3];

			*port = head[4];
			*port = (*port << 8) + head[5];

			data_len = head[6];
			data_len = (data_len << 8) + head[7];

			addrbsb = (uint32_t)(ptr << 8) + (s << 5) + 0x18;

			wiz_read_buf(addrbsb, buf, data_len);

			ptr += data_len;

			IINCHIP_WRITE(Sn_RX_RD0(s), (uint8_t)((ptr & 0xff00) >> 8));
			IINCHIP_WRITE(Sn_RX_RD1(s), (uint8_t)(ptr & 0x00ff));

			break;

		case Sn_MR_IPRAW:

			wiz_read_buf(addrbsb, head, 0x06);

			ptr += 6;

			addr[0] = head[0];
			addr[1] = head[1];
			addr[2] = head[2];
			addr[3] = head[3];

			data_len = head[4];
			data_len = (data_len << 8) + head[5];

			addrbsb = (uint32_t)(ptr << 8) + (s << 5) + 0x18;

			wiz_read_buf(addrbsb, buf, data_len);

			ptr += data_len;

			IINCHIP_WRITE(Sn_RX_RD0(s), (uint8_t)((ptr & 0xff00) >> 8));
			IINCHIP_WRITE(Sn_RX_RD1(s), (uint8_t)(ptr & 0x00ff));

			break;

		case Sn_MR_MACRAW:

			wiz_read_buf(addrbsb, head, 0x02);

			ptr += 2;

			data_len = head[0];
			data_len = (data_len << 8) + head[1] - 2;

			if (data_len > 1514)
			{
				printf("data_len over 1514\r\n");

				while (1);
			}

			addrbsb = (uint32_t)(ptr << 8) + (s << 5) + 0x18;

			wiz_read_buf(addrbsb, buf, data_len);

			ptr += data_len;

			IINCHIP_WRITE(Sn_RX_RD0(s), (uint8_t)((ptr & 0xff00) >> 8));
			IINCHIP_WRITE(Sn_RX_RD1(s), (uint8_t)(ptr & 0x00ff));

			break;

		default:

			break;
		}

		IINCHIP_WRITE(Sn_CR(s), Sn_CR_RECV);

		//wait to process the command... 
		while (IINCHIP_READ(Sn_CR(s)));
	}

	return data_len;
}

#ifdef __MACRAW__

void macraw_open(void)
{
	uint8_t sock_num;

	uint16_t dummyPort = 0;

	uint8_t mFlag = 0;

	sock_num = 0;

	//Close the 0-th socket
	close(sock_num);

	//Open the 0-th socket with MACRAW mode
	socket(sock_num, Sn_MR_MACRAW, dummyPort, mFlag);
}

uint16_t macraw_send(const uint8_t* buf, uint16_t len)
{
	uint16_t ret = 0;

	uint8_t sock_num;

	sock_num = 0;

	//check size not to exceed MAX size.
	if (len > getIINCHIP_TxMAX(sock_num))
		ret = getIINCHIP_TxMAX(sock_num);
	else
		ret = len;

	send_data_processing(sock_num, (uint8_t*)buf, len);

	//W5500 SEND COMMAND
	IINCHIP_WRITE(Sn_CR(sock_num), Sn_CR_SEND);

	while (IINCHIP_READ(Sn_CR(sock_num)));

	while ((IINCHIP_READ(Sn_IR(sock_num)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK);

	IINCHIP_WRITE(Sn_IR(sock_num), Sn_IR_SEND_OK);

	return ret;
}

uint16_t macraw_recv(uint8_t* buf, uint16_t len)
{
	uint8_t sock_num;

	uint16_t data_len = 0;

	uint16_t dummyPort = 0;

	uint16_t ptr = 0;

	uint8_t mFlag = 0;

	sock_num = 0;

	if (len > 0)
	{
		data_len = 0;

		ptr = IINCHIP_READ(Sn_RX_RD0(sock_num));
		ptr = (uint16_t)((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_RX_RD1(sock_num));

		//read data
		data_len = IINCHIP_READ_RXBUF(0, ptr);

		ptr++;

		data_len = ((data_len << 8) + IINCHIP_READ_RXBUF(0, ptr)) - 2;

		ptr++;

		if (data_len > 1514)
		{
			printf("data_len over 1514\r\n");

			printf("\r\nptr: %X, data_len: %X", ptr, data_len);

			//recommand : close and open 

			//Close the 0-th socket
			close(sock_num);

			//Open the 0-th socket with MACRAW mode
			socket(sock_num, Sn_MR_MACRAW, dummyPort, mFlag);

			return 0;
		}

		IINCHIP_READ_RXBUF_BURST(sock_num, ptr, data_len, (uint8_t*)(buf));

		ptr += data_len;

		IINCHIP_WRITE(Sn_RX_RD0(sock_num), (uint8_t)((ptr & 0xff00) >> 8));
		IINCHIP_WRITE(Sn_RX_RD1(sock_num), (uint8_t)(ptr & 0x00ff));

		IINCHIP_WRITE(Sn_CR(sock_num), Sn_CR_RECV);

		while (IINCHIP_READ(Sn_CR(sock_num)));
	}

	return data_len;
}

#endif

void close_all_socket(void)
{
	uint8_t i;

	for (i = 0; i < 7; i++)
	{
		disconnect(i);

		close(i);
	}
}
