#ifndef _XMLMESSAGES_H
#define _XMLMESSAGES_H

#ifdef	__cplusplus
extern "C" {
#endif

char* receiveXMLMessage(int socket);
void sendXMLMessage(int socket, const char* msg);

#ifdef	__cplusplus
}
#endif

#endif /* XMLMESSAGES_H */
