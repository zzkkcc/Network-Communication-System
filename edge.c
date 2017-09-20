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

#include "udpfun.h"
//tcp port number connect to client
#define PORT_1 "23642"
//udp port number connect to server_or
#define PORT_OR "21642"
//udp port number connect to server_and
#define PORT_AND "22642"
//udp port number for EDGE
#define PORT_EDGE "24642"
#define MAXBUFLEN 256

//for the received tcp data
char tcp_data[MAXBUFLEN];

//kill zombie threads
void sigchld_handler(int s)
{
       int saved_errno = errno;
       while(waitpid(-1,NULL,WNOHANG)>0);
       errno = saved_errno;
}

//Configure as server using TCP
int serv_config()
{
    int sockfd;
	struct addrinfo hints,*serverinfo,*p;
	int rv;
	int yes=1;
	
    //set fundamental TCP parameters
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
    
	if((rv= getaddrinfo(NULL,PORT_1,&hints,&serverinfo))!=0){
	     fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(rv));
	     return -1;
	}
	p=serverinfo;
	if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==-1){
	      perror("server:socket"); close(sockfd);
	}
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1){
	      perror("setsockopt");    close(sockfd);
	}
	if (bind(sockfd, p->ai_addr, p->ai_addrlen) ==-1){
	      close(sockfd);  perror("Error on binding.");
	}
	
	return sockfd;
}

//receive data from TCP as a server
int recv_tcp(int sockfd)
{
    struct sockaddr_in cli_addr;
    int clilen,newsockfd,n;
        
	listen(sockfd, 10);
	clilen = sizeof(cli_addr);

	if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1){
		perror("Error on accept.");   exit(1);
	}

	bzero(tcp_data, 256);
	if ((n= recv(newsockfd, tcp_data, MAXBUFLEN,0)) == -1){
		perror("Error reading from socket.");	exit(1);
	}
	tcp_data[n]='\0';
	return newsockfd;
}
//send data to TCP as a server
void send_tcp(int newsockfd, char *buff)
{
    int n;
    if ((n = send(newsockfd, buff, MAXBUFLEN,0))== -1)
    {
        perror("Error writing to socket.");
		exit(1);
    }
    //printf("Here is the message: %s\n", serv_or);
}
// for received TCP file, make descision which back-end server is sent
void Descision(char *servOR, char *servAND)
{
    int i,j,k,flag=5,count;
    i=j=k=count=0;
    //scan received data
    do{
        //find 'OR' word
        if((tcp_data[i]=='o')&&(tcp_data[i+1]=='r')){
            //set flag in order to copy to serv_or;
            flag=1;
            //set '#' as symbol for line number content
            servOR[j++]='#';
            //transfer tens digit of count in to 'char'
            servOR[j++]=count/10+'0';
            //transfer unit digit of count in to 'char'
            servOR[j++]=count%10+'0';
            count++;
            //move infor to the ','
            i++;
        }
        //find 'AND' words
        else if((tcp_data[i]=='a')&&(tcp_data[i+1]=='n')&&(tcp_data[i+2]=='d')){
            //set flag in order to copy to serv_and;
            flag=0;
            //set '#' as symbol for line number content
            servAND[k++]='#';
            //transfer tens digit of count in to 'char'
            servAND[k++]=count/10+'0';
            //transfer unit digit of count in to 'char'
            servAND[k++]=count%10+'0';
            
            count++;
            i=i+2;
        }
        else if(flag==1){
            //copy to serv_or
            servOR[j++]=tcp_data[i];
        }
        else if(!flag){
            //copy to serv_and
            servAND[k++]=tcp_data[i];
        }
    }while(tcp_data[i++]!='\0');
    servOR[j]=servAND[k]='\0';
}
//Count Line Number of content
int CntLine(char *buf)
{
        int i,count;
        i=count=0;
        while(buf[i++]!='\0'){
		if(buf[i]=='\n')
			count++;
        }
        return count;
}
//print received data,
//without printing contents of line number information
void PrintData(char *buffer)
{
	int i=0;
	do{
		if(buffer[i]=='#'){
			while(buffer[i]!='\n'&&buffer[i]!='\0')
			{	i++;}
		}
		printf("%c",buffer[i]);
	}while(buffer[i++]!='\0');
	printf("\n");
}

int main(void)
{
    int sockServ,sockCliOr,sockCliAnd,sockServOr,sockServAnd,tcpsockServ;
    char servOR[MAXBUFLEN],servAND[MAXBUFLEN],recv[MAXBUFLEN];
    char line[100][MAXBUFLEN];
    int linenum,cntOR,cntAND;
    printf("The edge server is up and running.\n");
    
    //Configure as TCP Server
    sockServ = serv_config();
    //receive contents from client server
    tcpsockServ = recv_tcp(sockServ);
    linenum = CntLine(tcp_data);
    
    printf("The edge server has received %d lines from the client using TCP over port %s.\n", linenum, PORT_1);
    
    //make a descision which back-end server is sent
    Descision(servOR, servAND);
    cntOR = CntLine(servOR);
    //count number of lines for servOR and servAND
    cntAND = CntLine(servAND);
    
    //Configure as UDP Client,and sent data to servOR
    sockCliOr = ClienUdpConfig(PORT_OR, servOR);
    printf("The edge server has successfully sent %d lines to the Backend-Server OR.\n", cntOR);
    //Configure as UDP Client,and sent data to servAnd
    sockCliAnd=ClienUdpConfig(PORT_AND,servAND);
    printf("The edge server has successfully sent %d lines to the Backend-Server AND.\n", cntAND);
    close(sockCliOr);
    //close udp sending sockets
    close(sockCliAnd);
    
    bzero(servAND, MAXBUFLEN);
    //clear content
    bzero(servOR, MAXBUFLEN);
    // Configure as UDP Server with EDGE port number
    sockServAnd=ServUdpConfig(PORT_EDGE);
    printf("The edge server starts receiving the computation results from Backend-Server OR and Backend-Server And using UDP port %s .\n", PORT_EDGE);
    
    RecvUdp(sockServAnd, servAND);
    //receive data from both back-ends
    RecvUdp(sockServAnd, servOR);
    //close udp receiving socket
    close(sockServAnd);
    
    strcat(servOR, "\n");
    //store received data in servOR
    strcat(servOR, servAND);
    
    printf("The computation results are:\n");
    //print results in correct format
    PrintData(servOR);
    printf("The edge server has successfully finished receiving all computation results from Backend-Server OR and Backend-Server AND.\n");
    
    //send TCP data to client
    send_tcp(tcpsockServ, servOR);
    printf("The edge server has successfully finished sending all computation results to the client.\n");
    close(sockServ);
    
    return 0;
}
