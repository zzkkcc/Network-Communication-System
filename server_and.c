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

#define PORT_AND "22642" //server_and udp port number
#define PORT_EDGE "24642"//udp port number connect to edge

#define MAXBUFLEN 256

int itob(int num)//convert integer to requried binary number
{//Eg. 1101 -> 13
        int i=0,result=0;
        do{
            result+=(num%10)<<i++;
            num=num/10;
        }while(num);
        return result;
}
void btoa(int num, char result[20])//convert integer to requried string
{//Eg. 13-> "1101"
        int count=0,j;
        char tran[20];
        
        do{
            if(num%2)
                tran[count++]= 1+'0';
            else
                tran[count++]= 0+'0';//get reverse result. Eg. 13-->"1011"
            num=num>>1;
        }while(num);
        tran[count]='\0';
        for(j=0;j<count;j++)
        {
            result[j]=tran[count-1-j];
        }//make reverse result back. Eg. "1011"-->"1101"
        result[count]='\0';
}

int process(char buf[MAXBUFLEN],char buffnew[MAXBUFLEN])//compute received data and print them correctly
{
        int i,j,k,flag,linenum,totlen;
        int fircom[10],seccom[10],linend[11];
        char oper[10][5],fir_str[10][MAXBUFLEN],sec_str[10][20],reschar[10][20];
        int first[10],second[10],fir_bit[10],sec_bit[10],result[10];
        totlen=linenum=flag=0;
        linend[0]=-1;
        
        while(buf[totlen]!='\0'){//scan received data and get position & linenumber information
            if(buf[totlen]==','){//flag=0,first comma detected.    
                 if(!flag){//flag=0,first comma detected.
                 fircom[linenum]=totlen;//record position of first comma
                 flag=1;
                 }
                 else if(flag){//flag=1, second comma detected.
                 seccom[linenum]=totlen;//record position of second comma
                 }
            }
            else if(buf[totlen]=='\n'){//this line end, ready for next line
                 flag=0;
                 linend[linenum+1]=totlen;//record position of len ending
                 linenum++;
            }
            totlen++;  
        }
        for(i=0;i<linenum;i++){
            j=0;
            for(k=linend[i]+1;k<fircom[i];k++)
                oper[i][j++]=buf[k];//copy line number information
            oper[i][j]='\0';
            j=0;
            for(k=fircom[i]+1;k<seccom[i];k++)
                fir_str[i][j++]=buf[k];//copy first string
            fir_str[i][j]='\0';
            j=0;
            for(k=seccom[i]+1;k<linend[i+1];k++)
                sec_str[i][j++]=buf[k];//copy second string
            sec_str[i][j]='\0';//complete one line
            first[i]=atoi(fir_str[i]);
            second[i]=atoi(sec_str[i]);//convert string to integer number
            fir_bit[i]=itob(first[i]);
            sec_bit[i]=itob(second[i]);//conver interger number to requried one, Eg. 1101 -> 13
            result[i] = fir_bit[i]&sec_bit[i];//computation
            btoa(result[i],reschar[i]);
            printf("%s	and	%s	=	%s\n",fir_str[i],sec_str[i],reschar[i]);//print result
            strcat(reschar[i],oper[i]);
            strcat(fir_str[i],"	and	");
            strcat(fir_str[i],sec_str[i]);
            strcat(fir_str[i],"	=	");
            strcat(fir_str[i],reschar[i]);//generate sending back messages for one line
        }
        strcpy(buffnew,fir_str[0]);
        for(i=1;i<linenum;i++){//generage sending back messages for the whole
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
        
        sockfd=ServUdpConfig(PORT_AND);//Configure as UDP Server with Port number PORT_AND
        RecvUdp(sockfd,data);//receive udp data from edge
        printf("The Server AND starts receiving lines from the edge server for AND computation. The computation results are:\n");
        close(sockfd);

        count=process(data,buf);//process data, and return line number
        printf("The Server AND has successfully received %d line from the edge server and finished all AND computations.\n",count);
        
        sockSend=ClienUdpConfig(PORT_EDGE,buf);//Configure as UDP Client,and sent data to edge
        printf("The Server AND has successfully finished sending all computation results to edge server.");
        close(sockSend);
	
	return 0;
}
