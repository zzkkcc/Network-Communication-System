#ifndef _UDPFUN_H
#define _FUN_H

int ClienUdpConfig(char *PORTNUM, char *buff);
int ServUdpConfig(char *PORTNUM);
void RecvUdp(int sockfd, char* buff);

#endif
