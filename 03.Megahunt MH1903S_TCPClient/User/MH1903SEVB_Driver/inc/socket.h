#ifndef	__SOCKET_H
#define	__SOCKET_H

#include "type.h"

uint8_t socket(SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag);

void close(SOCKET s);

uint8_t connect(SOCKET s, uint8_t* addr, uint16_t port);

void disconnect(SOCKET s);

uint8_t listen(SOCKET s);

uint16_t send(SOCKET s, const uint8_t* buf, uint16_t len);

uint16_t recv(SOCKET s, uint8_t* buf, uint16_t len);

uint16_t sendto(SOCKET s, const uint8_t* buf, uint16_t len, uint8_t* addr, uint16_t port);

uint16_t recvfrom(SOCKET s, uint8_t* buf, uint16_t len, uint8_t* addr, uint16_t* port);

#ifdef __MACRAW__

void macraw_open(void);

//Send data (MACRAW)
uint16_t macraw_send(const uint8_t* buf, uint16_t len);

//Recv data (MACRAW)
uint16_t macraw_recv(uint8_t* buf, uint16_t len);

#endif

void close_all_socket(void);

#endif
