#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MAXBUFLEN 256
/*
This file summerize 3 similar functions used in the two backen-servers
and edge server, only in the three servers, this function is used
*/

int ClienUdpConfig(char *PORTNUM, char *buff)
{//Configure as a UDP Client with PORT number parameter, after configuration,
//content in buffer will be sent.
//Codes in this function comes from Beej's tutorial with modification
	int sockfd,numbytes;
	struct addrinfo hints,*servinfo,*p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;// set fundamental parameters

	if ((rv = getaddrinfo("localhost", PORTNUM, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}//get local host name
        for(p=servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)  
		{
			perror("talker: socket");
			continue;
		}//get socket Number
		break;
	}
	if(p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return -1;
	}
	freeaddrinfo(servinfo);
	
        if((numbytes= sendto(sockfd, buff, strlen(buff),0,
	        p->ai_addr, p -> ai_addrlen)) <0)     {
		perror("talker: sendto");  exit(1);
	}//send content
	bzero(buff, MAXBUFLEN);
	return sockfd;
}
int ServUdpConfig(char *PORTNUM)//Configure as Udp server with Port number parameter
{//Codes in this function comes from Beej's tutorial with modification
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; //

	if ((rv = getaddrinfo(NULL, PORTNUM, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		exit(1);
	}
	freeaddrinfo(servinfo);
	
	return sockfd;
}
void RecvUdp(int sockfd, char* buff)// Receive Udp data
{//Codes in this function comes from Beej's tutorial with modification	
        struct sockaddr_storage their_addr;
	socklen_t addr_len;
	int numbytes;
        addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buff, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	buff[numbytes] = '\0';
}
