
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <iostream>
#define MSS 4096//assume MSS as 4096

using namespace std;


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



};

class TCPPacket
{

    TCPHeader head;
    char data[MSS];
};
int main()
{
      int welcomeSocket, newSocket;
      char buffer[1024]={0},send_data[1024]={0};
      struct sockaddr_in serverAddr;
      struct sockaddr_storage serverStorage;
      socklen_t addr_size;

      /*---- Create the socket. The three arguments are: ----*/
      /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
      welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

      if(welcomeSocket <0)
      {
          perror("cannot create socket");
               return 0;

      }

      /*---- Configure settings of the server address struct ----*/
      /* Address family = Internet */
      serverAddr.sin_family = AF_INET;
      /* Set port number, using htons function to use proper byte order */
      serverAddr.sin_port = htons(7891);
      /* Set IP address to localhost */
      serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
      /* Set all bits of the padding field to 0 */
      memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

      /*---- Bind the address struct to the socket ----*/
      bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

      /*---- Listen on the socket, with 5 max connection requests queued ----*/
      if(listen(welcomeSocket,5)==0)
        printf("Listening\n");
      else
        printf("Error\n");

      /*---- Accept call creates a new socket for the incoming connection ----*/
      addr_size = sizeof serverStorage;


      newSocket=accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

      while(strcmp(buffer,"stop")!=0)
      {
        cout<<"Enter the data to send : ";
        cin>>send_data;
        strcpy(buffer,send_data);
        send(newSocket,buffer,13,0);

      }

      /*---- Send message to the socket of the incoming connection ----*/


      return 0;

         //getchar();
}
