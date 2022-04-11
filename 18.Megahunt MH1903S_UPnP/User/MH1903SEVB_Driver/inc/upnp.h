#ifndef __UPNP_H
#define __UPNP_H

#include "type.h"

#define SOCK_SSDP									1
#define SOCK_UPNP									2	
#define SOCK_UPNP_EVENTING							3
#define SOCK_LOOPBACK								4

signed char SSDPProcess(SOCKET sockfd);				/**< This function processes the SSDP message. */
signed char GetDescriptionProcess(SOCKET sockfd);	/**< This function gets the description message from IGD(Internet Gateway Device). */
signed char SetEventing(SOCKET sockfd);				/**< This function subscribes to the eventing message from IGD(Internet Gateway Device). */
void eventing_listener(SOCKET s);					/**< This function listenes the eventing message from IGD(Internet Gateway Device). */

/**< This function processes the delete port to IGD(Internet Gateway Device). */
signed short DeletePortProcess(SOCKET sockfd, const char* protocol, const unsigned int extertnal_port);
/**< This function processes the add port to IGD(Internet Gateway Device). */
signed short AddPortProcess(SOCKET sockfd, const char* protocol, const unsigned int extertnal_port, const char* internal_ip, const unsigned int internal_port, const char* description);

signed char parseHTTP(const char* xml);				/**< This function parses the HTTP header. */
signed char parseSSDP(const char* xml);				/**< This function parses the received SSDP message from IGD(Internet Gateway Device). */
signed char parseDescription(const char* xml);		/**< This function parses the received description message from IGD(Internet Gateway Device). */
void parseEventing(const char* xml);				/**< This function parses and prints the received eventing message from IGD(Internet Gateway Device). */
signed short parseError(const char* xml);			/**< This function parses the received UPnP error message from IGD(Internet Gateway Device). */
signed short parseDeletePort(const char* xml);		/**< This function parses the received delete port message from IGD(Internet Gateway Device). */
signed short parseAddPort(const char* xml);			/**< This function parses the received add port message from IGD(Internet Gateway Device). */

void Upnp(void);

#endif
