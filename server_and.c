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

#include "udpfun.h"

//server_and udp port number
#define PORT_AND "22642"
//udp port number connect to edge
#define PORT_EDGE "24642"

#define MAXBUFLEN 256

//convert integer to requried binary number
//Eg. 1101 -> 13
int itob(int num)
{
    int i=0,result=0;
    do{
        result+=(num%10)<<i++;
        num=num/10;
    }while(num);
    return result;
}
//convert integer to requried string
//Eg. 13-> "1101"
void btoa(int num, char result[20])
{
    int count=0,j;
    char tran[20];
    do{
        if(num%2)
            tran[count++]= 1+'0';
        else
            //get reverse result. Eg. 13-->"1011"
            tran[count++]= 0+'0';
        num=num>>1;
    }while(num);
    tran[count]='\0';
    //make reverse result back. Eg. "1011"-->"1101"
    for(j=0;j<count;j++)
    {
        result[j]=tran[count-1-j];
    }
    result[count]='\0';
}

//compute received data and print them correctly
int process(char buf[MAXBUFLEN],char buffnew[MAXBUFLEN])
{
    int i,j,k,flag,linenum,totlen;
    int fircom[10],seccom[10],linend[11];
    char oper[10][5],fir_str[10][MAXBUFLEN],sec_str[10][20],reschar[10][20];
    int first[10],second[10],fir_bit[10],sec_bit[10],result[10];
    totlen=linenum=flag=0;
    linend[0]=-1;
    
    //scan received data and get position & linenumber information
    while(buf[totlen]!='\0'){
        //flag=0,first comma detected.
        if(buf[totlen]==','){
            //flag=0,first comma detected.
            if(!flag){
                //record position of first comma
                fircom[linenum]=totlen;
                flag=1;
            }
            //flag=1, second comma detected.
            else if(flag){
                //record position of second comma
                seccom[linenum]=totlen;
            }
        }
        //this line end, ready for next line
        else if(buf[totlen]=='\n'){
            flag=0;
            //record position of len ending
            linend[linenum+1]=totlen;
            linenum++;
        }
        totlen++;
    }
    for(i=0;i<linenum;i++){
        j=0;
        for(k=linend[i]+1;k<fircom[i];k++){
            //copy line number information
            oper[i][j++]=buf[k];
        }
        oper[i][j]='\0';
        j=0;
        for(k=fircom[i]+1;k<seccom[i];k++){
            //copy first string
            fir_str[i][j++]=buf[k];
        }
        fir_str[i][j]='\0';
        j=0;
        for(k=seccom[i]+1;k<linend[i+1];k++){
            //copy second string
            sec_str[i][j++]=buf[k];
        }
        //complete one line
        sec_str[i][j]='\0';
        first[i]=atoi(fir_str[i]);
        //convert string to integer number
        second[i]=atoi(sec_str[i]);
        fir_bit[i]=itob(first[i]);
        //convert interger number to requried one, Eg. 1101 -> 13
        sec_bit[i]=itob(second[i]);
        //computation
        result[i] = fir_bit[i]&sec_bit[i];
        btoa(result[i],reschar[i]);
        //print result
        printf("%s	and	%s	=	%s\n",fir_str[i],sec_str[i],reschar[i]);
        strcat(reschar[i],oper[i]);
        strcat(fir_str[i],"	and	");
        //generate sending back messages for one line
        strcat(fir_str[i],sec_str[i]);
        strcat(fir_str[i],"	=	");
        strcat(fir_str[i],reschar[i]);
    }
    strcpy(buffnew,fir_str[0]);
    //generage sending back messages for the whole
    for(i=1;i<linenum;i++){
        strcat(buffnew,"\n");
        strcat(buffnew,fir_str[i]);
    }
    return linenum;
}
int main(void)
{
    int sockfd,count,sockSend;
    char data[MAXBUFLEN],buf[MAXBUFLEN];
    printf("The Server AND is up and running using UDP on port 22642.\n");
    //configure as UDP Server with Port number PORT_AND
    sockfd=ServUdpConfig(PORT_AND);
    //receive udp data from edge
    RecvUdp(sockfd,data);
    printf("The Server AND starts receiving lines from the edge server for AND computation. The computation results are:\n");
    close(sockfd);
    //process data, and return line number
    count=process(data,buf);
    printf("The Server AND has successfully received %d line from the edge server and finished all AND computations.\n",count);
    //configure as UDP Client,and sent data to edge
    sockSend=ClienUdpConfig(PORT_EDGE,buf);
    printf("The Server AND has successfully finished sending all computation results to edge server.");
    close(sockSend);
	
	return 0;
}
