#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <deque>
#include <iomanip>
#include <time.h>
#include <pthread.h>
#define MSS 4096
#define M 4
#define SERVER_PORT 5000
using namespace std;

char s_sendBuff[1000];
char c_sendBuff[1000];
char s_recvBuff[1000];
char c_recvBuff[1000];


string int_to_string(int i)
{

    stringstream ss;
    ss << i;
    return(ss.str());
}


void tokenize(char str[],char *tok[10],int &count)
{
    char * pch;
    int i = 0;
    pch = strtok (str," ");
    while (pch != NULL)
    {
        tok[i] = pch;
        pch = strtok (NULL," ");
        i++;
    }
    count = i;

}

struct sockets
{
    string ip_addr;
    int port_number;
}so;

class TCPHeader
{
    int sourceport;
    int destiport;
    long long int seq_no;
    long long int ack_no;
    long long int data_offset;
    long long int window;
    int reserved;
    char ecn[3];
    char URG;
    char ACK;
    char PSH;
    char RST;
    char SYN;
    char FIN;
    int checksum;
    int urg_pointer;
    char options[40*8];   //40 bytes of options


    public:

        TCPHeader()
        {
                //Default Constructor
        }

        TCPHeader(int sp,int dp, int seq,int ack,int offset,int winsiz, int reser, char ecn[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, int urg_pointer,char options[])
        {
            this->sourceport = sp;
            this->destiport = dp;
            this->seq_no = seq ;
            this->ack_no = ack;
            this->data_offset = offset;
            this->window = winsiz;
            this->reserved = reser;
            strcpy(ecn,this->ecn);
            this->URG = URG;
            this->ACK = ACK;
            this->PSH = PSH;
            this->RST = RST;
            this->SYN = SYN;
            this->FIN = FIN;
            this->checksum = checksum;
            this->urg_pointer = urg_pointer;
            strcpy(options,this->options); 
        }

            //getters and setters

        int spval()
        {
            return sourceport;
        }

        int dpval()
        {
            return destiport;
        }

        int seqval()
        {
            return seq_no;
        }

};

class TCPPacket
{
    public :

    TCPHeader head;
    char data[MSS];
};

void server_menu(char s_recvBuff[])
{
    printf("%s\n", s_recvBuff);
}

void *myserver(void *vargs)
{
    int listenfd = 0,connfd = 0,m = 0;
    struct sockaddr_in serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    //printf("socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERVER_PORT);

    bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

    if(listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        return NULL;
    }

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
        //cout<<" \n Server  Accepted the request to connect \n";

        if((m = read(connfd, s_recvBuff, sizeof(s_recvBuff)-1)) > 0)
        {
            s_recvBuff[m] = 0;
            server_menu(s_recvBuff);
        }

        sleep(2);

        write(connfd, s_sendBuff, strlen(s_sendBuff));

    }

    close(connfd);
    return NULL;
    /* int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

  
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(s_sendBuff, '0', sizeof(s_sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl("IPA");
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    while(1)
    {
        fflush(stdout);
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

        ticks = time(NULL);
        snprintf(s_sendBuff, sizeof(s_sendBuff), "%.24s\r\n", ctime(&ticks));
        write(connfd, s_sendBuff, strlen(s_sendBuff)); 

        close(connfd);
        sleep(1);
     }*/
}


void * myclient( void *vargs)
{
   

    int port_no = so.port_number;
    string ip_addr = so.ip_addr;

    int sockfd = 0,n = 0;
    struct sockaddr_in serv_addr;


    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return NULL;
    }


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_no);
    serv_addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());



    while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return NULL;
    }

    char in[1024]="";

    gets(in);
    strcpy(c_sendBuff,in);
    write(sockfd,c_sendBuff, strlen(c_sendBuff));
    sleep(1);

    memset(c_recvBuff, '0' ,sizeof(c_recvBuff));
    while((n = read(sockfd, c_recvBuff, sizeof(c_recvBuff)-1)) < 0);

    c_recvBuff[n] = 0;
/*
        if(fputs(c_recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error");
        }
        printf("\n");
*/
    if( n < 0)
    {
        printf("\n Read Error \n");
    }

    close(sockfd);
    return 0;
}

int main(int argc, char *argv[])
{
    TCPPacket *packet = new TCPPacket;   // To send TCP header as application data
   // packet->head=NULL;
    //packet->data=NULL;

    
        memset(s_recvBuff, '0' ,sizeof(s_recvBuff));
        memset(s_sendBuff, '0', sizeof(s_sendBuff));
        memset(c_recvBuff, '0' ,sizeof(c_recvBuff));
        memset(c_sendBuff, '0', sizeof(c_sendBuff));

            if(argc<3)
            {
                printf("Invalid Usage : Specify IP address and Port Number \n");
            }

    if(argc != 3)
    {
        cout<<"Server Started >>\n";
        pthread_t serve;
        
        pthread_create(&serve, NULL, myserver, NULL);
        pthread_join(serve, NULL);
        
        
        
        
    }
    else if (argc==3)
    {
         cout<<"Client Started >>\n";
        so.ip_addr = argv[1];
        so.port_number = atoi(argv[2]);

        pthread_t client;
        pthread_create(&client, NULL, myclient, NULL);
        pthread_join(client, NULL);

       

    }

        
       
        return 0;

   
}