#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <map>
#include "MyThread.h"
#include <openssl/sha.h>
#define join 2

using namespace std;
//int port=5000;


struct tableentry
{
    string start;
    string end;
    string succ;
};
struct fingertab
{
    tableentry te[161];
};
struct node
{
    string nodeid;
    fingertab ft;
    map<int,string> key;
    string succ;
    string pre;
};
int conn_ser=0;
int conn_cli=0;

string sha_hash(string id)
{
     int i = 0;
     int len=id.length();
    unsigned char temp[SHA_DIGEST_LENGTH];
    char buf[SHA_DIGEST_LENGTH*2];

    /*if ( argn != 2 ) {
        printf("Usage: %s string\n", argv[0]);
        return -1;
    }*/
    char ar[]=id.c_str();
    memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
    memset(temp, 0x0, SHA_DIGEST_LENGTH);

    SHA1((unsigned char *)ar, len, temp);

    for (i=0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf((char*)&(buf[i*2]), "%02x", temp[i]);
    }

    printf("SHA1 of %s is %s\n", id, buf);

    return buf;
}
int server()
{
      int listenfd = 0,connfd = 0;

  struct sockaddr_in serv_addr;

  char sendBuff[1025];
  int numrv;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  printf("socket retrieve success\n");

  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(conn_ser);


  bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

  if(listen(listenfd, 10) == -1){
      printf("Failed to listen\n");
      return -1;
  }
    char str[1024];
 connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
  while(1)
    {
        cout<<"\n Enter the message to pass : ";
        cin>>str;
        strcpy(sendBuff, str);
      write(connfd, sendBuff, strlen(sendBuff));


      sleep(1);
    }

      close(connfd);
  return 0;
}
int client()
{

  int sockfd = 0,n = 0;
  char recvBuff[1024];
  struct sockaddr_in serv_addr;

  memset(recvBuff, '0' ,sizeof(recvBuff));
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
      printf("\n Error : Could not create socket \n");
      return 1;
    }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(conn_cli);
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
      printf("\n Error : Connect Failed \n");
      //return 1;
    }

  while((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
      recvBuff[n] = 0;
      if(fputs(recvBuff, stdout) == EOF)
    {
      printf("\n Error : Fputs error");
    }
      printf("\n");
    }

  if( n < 0)
    {
      printf("\n Read Error \n");
    }


  return 0;
}
void nodes()
{
	string temp=itoa(conn_ser);
    string id="127.0.0.1:"+temp;
    node n=new node;
    n.nodeid=sha_hash(id);
    n.pre=0;
    n.succ=0;
    printf(" Id of server is  %s", n.nodeid);
    server();
}
void nodec()
{
    client();
 }

int main(int argc, char *argv[])
{
create(nodes);
create(nodec);
conn_ser=atoi(argv[1]);
conn_cli=atoi(argv[2]);
start();

}
