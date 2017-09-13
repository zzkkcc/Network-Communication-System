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

#define PORT_1 "23642"//tcp port number connect to client
#define PORT_OR "21642"//udp port number connect to server_or
#define PORT_AND "22642"//udp port number connect to server_and
#define PORT_EDGE "24642"//UDP port number for EDGE
#define MAXBUFLEN 256

char tcp_data[MAXBUFLEN];//for the received tcp data

void sigchld_handler(int s)//kill zombie threads
{//Codes in this function comes from Beej's tutorial with modification
       int saved_errno = errno;
       while(waitpid(-1,NULL,WNOHANG)>0);
       errno = saved_errno;
}

int serv_config()//Configure as server using TCP
{//Codes in this function comes from Beej's tutorial with modification
        int sockfd;
	struct addrinfo hints,*serverinfo,*p;
	int rv;
	int yes=1;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;//TCP
	hints.ai_flags = AI_PASSIVE;//set fundamental TCP parameters
	
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

int recv_tcp(int sockfd)//receive data from TCP as a server
{//Codes in this function comes from Beej's tutorial with modification
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
void send_tcp(int newsockfd, char *buff)//receive data from TCP as a server
{//Codes in this function comes from Beej's tutorial with modification
       int n;
       if ((n = send(newsockfd, buff, MAXBUFLEN,0))== -1)
       {
		perror("Error writing to socket.");
		exit(1);
       }
       //printf("Here is the message: %s\n", serv_or);
}
void Descision(char *servOR, char *servAND)// for received TCP file, make descision which
{//back-end server is sent
        int i,j,k,flag=5,count;
        i=j=k=count=0;
        do{//scan received data
        	if((tcp_data[i]=='o')&&(tcp_data[i+1]=='r')){//find OR
        		flag=1;//set flag in order to copy to serv_or;
        		servOR[j++]='#';//set '#' as symbol for line number content
        		servOR[j++]=count/10+'0';//transfer tens digit of count in to 'char'
        		servOR[j++]=count%10+'0';//transfer unit digit of count in to 'char'
        		count++;
        		i++;//move infor to the ','
        	}
        	else if((tcp_data[i]=='a')&&(tcp_data[i+1]=='n')&&(tcp_data[i+2]=='d')){//find AND
        		flag=0;////set flag in order to copy to serv_and;
        		servAND[k++]='#';//set '#' as symbol for line number content
        		servAND[k++]=count/10+'0';//transfer tens digit of count in to 'char'
        		servAND[k++]=count%10+'0';//transfer unit digit of count in to 'char'
        		count++;
        		i=i+2;
        	}
        	else if(flag==1){
        		servOR[j++]=tcp_data[i];//copy to serv_or
        	}
        	else if(!flag){
        		servAND[k++]=tcp_data[i];//copy to serv_and
        	}	
        }while(tcp_data[i++]!='\0');
        servOR[j]=servAND[k]='\0';
}
int CntLine(char *buf)//Count Line Number of content
{
        int i,count;
        i=count=0;
        while(buf[i++]!='\0'){
		if(buf[i]=='\n')
			count++;
        }
        return count;
}
void PrintData(char *buffer)//print received data, 
{//and not print contents of line number information
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
        
        sockServ = serv_config();//Configure as TCP Server
        tcpsockServ = recv_tcp(sockServ);//receive contents from client server
        linenum = CntLine(tcp_data);
        
        printf("The edge server has received %d lines from the client using TCP over port %s.\n", linenum, PORT_1);
        
        Descision(servOR, servAND);//make a descision which back-end server is sent
        cntOR = CntLine(servOR);
        cntAND = CntLine(servAND);//count number of lines for servOR and servAND
        
        sockCliOr = ClienUdpConfig(PORT_OR, servOR);//Configure as UDP Client,and sent data to servOR
        printf("The edge server has successfully sent %d lines to the Backend-Server OR.\n", cntOR);
        sockCliAnd=ClienUdpConfig(PORT_AND,servAND);//Configure as UDP Client,and sent data to servAnd
        printf("The edge server has successfully sent %d lines to the Backend-Server AND.\n", cntAND);
        close(sockCliOr);
        close(sockCliAnd);//close udp sending sockets
        
        bzero(servAND, MAXBUFLEN);
        bzero(servOR, MAXBUFLEN);//clear content
        sockServAnd=ServUdpConfig(PORT_EDGE);// Configure as UDP Server with EDGE port number
        printf("The edge server starts receiving the computation results from Backend-Server OR and Backend-Server And using UDP port %s .\n", PORT_EDGE);
        
        RecvUdp(sockServAnd, servAND);
        RecvUdp(sockServAnd, servOR);//receive data from both back-ends
        close(sockServAnd);//close udp receiving socket
        
        strcat(servOR, "\n");
        strcat(servOR, servAND);//store received data in servOR
        
        printf("The computation results are:\n");
        PrintData(servOR);//print results in correct format
        printf("The edge server has successfully finished receiving all computation results from Backend-Server OR and Backend-Server AND.\n");
        
        send_tcp(tcpsockServ, servOR);//send TCP data to client
        printf("The edge server has successfully finished sending all computation results to the client.\n");
        close(sockServ);
        
	return 0;
}
