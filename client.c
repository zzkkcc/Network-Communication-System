#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "23642"//TCP PORT NUMBER 
#define MAXBUFLEN 256

char buffer[MAXBUFLEN];//store files from .txt & edge
char result[MAXBUFLEN][100];//get results

int ClienConfig(void)//the folloing codes in this function comes from Beej's 
{//tutorial with modifications in order to have tCP Client Configuration
	int sockfd;
	struct addrinfo hints, *servinfo;
	int rv;

	memset(&hints, 0, sizeof hints);//clear hints contents
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;//sent fundamental parameters of TCP Client
	
	if((rv = getaddrinfo("localhost", PORT, &hints, &servinfo))!=0){
	    fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(rv));
	    return -1;
	}
	
	if((sockfd = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol))==-1){
	perror("Error in client: opening socket\n");	return -1;
	}
	if((connect(sockfd,servinfo->ai_addr,servinfo->ai_addrlen))==-1){
	perror("Error in client: opening socket\n");	return -1;
	}
	if(servinfo==NULL){
	fprintf(stderr,"client:failed to connect\n");	return -1;
	}
	return sockfd;
}
void OpenFile(char *input)//Open File function, used for Openning "job.txt" and read content
{
        FILE *file;
        size_t size;
        if((file=fopen(input,"r"))==NULL){//open "job.txt"
                perror("open file");	exit(1);
        }
        if((size=fread((void*)buffer, MAXBUFLEN,1,file))<0){//read file into buffer
                perror("empty file, nothing to send");	exit(1);
        }
        fclose(file);
}
void SendData(int sockfd)//used for sending data, codes in this function also comes from Beej's
{//tutorial with modification.
        int numbytes;
	if((numbytes= send(sockfd, buffer, strlen(buffer),0))==-1){
		perror("talker: sendto");  exit(1);
	}
}
int CntLine()//Count Line Number of content
{
        int i,count;
        i=count=0;
        while(buffer[i++]!='\0'){
		if(buffer[i]=='\n')
			count++;
        }
        return count;
}
void RecvData(int sockfd)//for receive data from edge server
{// codes in this function comes from Beej's tutorial with modification.	
        int numbytes;
	numbytes = recv(sockfd, buffer, MAXBUFLEN-1,0);
        if (numbytes == -1){       
                perror("Error reading from socket.");    exit(0);
	}
	buffer[numbytes]='\0';
}
void ProceData(void)//use for processing, including ordering received data & 
{//print data in correct ways
	int i,j,linenum,order[MAXBUFLEN],t;
	char temp[100],str[20];
	linenum=j=i=0;
	do{
		if(buffer[i]=='='){
			j=0;
			i=i+2;
			do{//get computation outcomes for result[linenum] 
				result[linenum][j++]=buffer[i++];
			}while(buffer[i]!='#');
			result[linenum][j]='\0';
			j=0;
			i++;
			do{//get line number informations for the result
				str[j++]=buffer[i++];
			}while(buffer[i]!='\n'&&buffer[i]!='\0');
			str[j]='\0';
			order[linenum]=atoi(str);
			linenum++;//change to the next line
		}
	}while(buffer[i++]!='\0');
	
	for(i=0;i<linenum;i++){//the for loop use for order & print received data
		for(j=0;j<linenum;j++){
			if(order[j]==i&&(j!=i)){
				strcpy(temp,result[i]);
				strcpy(result[i],result[j]);
				strcpy(result[j],temp);
				t=order[i];
				order[i]=order[j];
				order[j]=t;
			}
		}
		printf("%s	\n",result[i]);
	}
}
int main(int argc, char *argv[])
{
        int sockfd,lineNum;
        if(argc <2){// test format of input
        	perror("invalid input");exit(1);
        }
        printf("The client is up an running.\n");
        if((sockfd=ClienConfig())==-1){//Configure TCP Client & result sockfd #
		perror("does not receive words");  exit(1);  
        }
        bzero(buffer,256);
        OpenFile(argv[1]);//clear buffer, and then read contents of txt file into buffer.
        lineNum=CntLine();//Count line numbers
        SendData(sockfd);//Send data to the edge server
        printf("The client has successfully finished sending %d lines to the edge server.\n",lineNum);
        bzero(buffer,MAXBUFLEN);
        RecvData(sockfd);//waiting to receive data
	printf("The client has successfully finished receiving all computation results form the edge server.\n");
	printf("The final computation results are:\n");
	ProceData();//process received data and print them
	close(sockfd);//close socket
	return 0;
}
