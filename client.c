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

//buffer variable is used for store files from .txt & edge
char buffer[MAXBUFLEN];
//to get results
char result[MAXBUFLEN][100];
int ClienConfig(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo;
	int rv;

    //clear hints contents
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    //sent fundamental parameters of TCP Client
	hints.ai_socktype = SOCK_STREAM;
	
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
//used for open "job.txt" and read content in txt
//assumption: format of "job.txt":
//AND 10101 10101
void OpenFile(char *input)
{
    FILE *file;
    size_t size;
    //open "job.txt"
    if((file=fopen(input,"r"))==NULL){
        perror("open file");
        exit(1);
    }
    if((size=fread((void*)buffer, MAXBUFLEN,1,file))<0){//read file into buffer
        perror("empty file, nothing to send");
        exit(1);
    }
    fclose(file);
}
void SendData(int sockfd)
{
    int numbytes;
	if((numbytes= send(sockfd, buffer, strlen(buffer),0))==-1){
		perror("talker: sendto");
        exit(1);
	}
}
//Count Line Number of content
int CntLine()
{
    int i,count;
    i=count=0;
    while(buffer[i++]!='\0'){
    if(buffer[i]=='\n')
        count++;
    }
    return count;
}
void RecvData(int sockfd)
{
    int numbytes;
	numbytes = recv(sockfd, buffer, MAXBUFLEN-1,0);
    if (numbytes == -1){
        perror("Error reading from socket.");
        exit(0);
	}
	buffer[numbytes]='\0';
}
//use for processing, including ordering received data &
//print data in correct ways
void ProceData(void)
{
	int i,j,linenum,order[MAXBUFLEN],t;
	char temp[100],str[20];
	linenum=j=i=0;
	do{
		if(buffer[i]=='='){
			j=0;
			i=i+2;
            //get computation outcomes for result[linenum]
			do{
				result[linenum][j++]=buffer[i++];
			}while(buffer[i]!='#');
			result[linenum][j]='\0';
			j=0;
			i++;
            //get line number informations for the result
			do{
				str[j++]=buffer[i++];
			}while(buffer[i]!='\n'&&buffer[i]!='\0');
			str[j]='\0';
			order[linenum]=atoi(str);
			//move to the next line
            linenum++;
		}
	}while(buffer[i++]!='\0');
	//the for loop use for order & print received data
	for(i=0;i<linenum;i++){
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
    //corner case: test format of input is wrong
    if(argc <2){
        perror("invalid input");
        exit(1);
    }
    printf("The client is up an running.\n");
    //configure TCP Client & result sockfd number
    if((sockfd=ClienConfig())==-1){
        perror("does not receive words");  exit(1);
    }
    //clear buffer, and then read contents of txt file into buffer.
    bzero(buffer,256);
    OpenFile(argv[1]);
    //Count line numbers
    lineNum=CntLine();
    //Send data to the edge server
    SendData(sockfd);
    printf("The client has successfully finished sending %d lines to the edge server.\n",lineNum);
    
    bzero(buffer,MAXBUFLEN);
    //waiting to receive data
    RecvData(sockfd);
    printf("The client has successfully finished receiving all computation results form the edge server.\n");
	printf("The final computation results are:\n");
    //process received data and print them
    ProceData();
    //close socket
    close(sockfd);
	return 0;
}
