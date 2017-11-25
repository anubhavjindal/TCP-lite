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
#include <pthread.h>
//#include "structures.h"
//# include "sha.cpp"
#define M 4




using namespace std;

char chord[] = "chord> ";
int key;
string value = "";
int port_num;
string ip;
node * n = NULL;


void error()
{
	cout<<chord<<"**********Invalid Usage**********\n";
	cout<<chord<<"**********Use Help for proper Usage************\n";
}

void help()
{
	cout<<chord<<"commands: help, quit, port, create, join, dump, put, get, delete\n";
	cout<<"*************************************************************************\n";
	cout<<"(1) help : Provides a list of command and their usage details.\n";
	cout<<"(2) port <x>: Listen on this port for other instances of the program over different nodes.\n";
	cout<<"(3) create : creates the ring.\n";
	cout<<"(4) join <x>: Join the ring with x address.\n";
	cout<<"(5) quit: Shuts down the ring.\n";
	cout<<"(6) put <key> <value>: insert the given <key,value> pair in the ring.\n";
	cout<<"(7) get <key>: returns the value corresponding to the key, if one was previously inserted in the node.\n";
	cout<<"*************************************************************************\n";

}

void free()
{
	remove("create.txt");
	remove("port.txt");
}

void port(int pno)
{
	ofstream file("port.txt");
	file<<"1";
	file.close(); 
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

string getmyip()
{
	int fd;
 	struct ifreq ifr;

 fd = socket(AF_INET, SOCK_DGRAM, 0);

 /* I want to get an IPv4 IP address */
 ifr.ifr_addr.sa_family = AF_INET;

 /* I want IP address attached to "eth0" */
 strncpy(ifr.ifr_name, "p3p1", IFNAMSIZ-1);

 ioctl(fd, SIOCGIFADDR, &ifr);

 close(fd);

 /* display result */
 string s = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

 return s;

}

string getmyport()
{
	ifstream file1("portno.txt");
	char next[10];
	file1 >> next;
	file1.close();
	int n = atoi(next);
	n++;
	ofstream file2("portno.txt");
	file2 << n;
	file2.close();
	return (string)next;


}
void create()
{
	ofstream file("create.txt");
	file<<"1";
	file.close(); 
	
	ip = getmyip();
	//port_num = getmyport();
	stringstream ss;
	ss << port_num;
	string addr = ip + ":" + ss.str();
	cout << addr <<"\n";
	long int temp = sha(addr.c_str());
	

	// initialize node
	n = new node;
	n->port = port_num;
	n->nodeid = temp;
	n->succ = temp;
	n->pre = temp;
	for(int i=1; i<=M; i++)
	{
		n->ft.te[i].start = (int)(n->nodeid + pow(2,(i - 1))) %  (int)pow(2,M);
		n->ft.te[i].end = (int)(n->nodeid + pow(2,i) - 1) %  (int)pow(2,M);
		n->ft.te[i].succ = n->nodeid;
	}
	
}

string get(int key)
{
	return "HELLO";
}

void put(int key, string value)
{

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
  serv_addr.sin_port = htons(port_num);


  bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

  if(listen(listenfd, 10) == -1){
      printf("Failed to listen\n");
      return -1;
  }
    char str[1024];
 connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
 cout<<" \n Server  Accepted the request to connect \n";
  while(1)
    {
        cout<<"\n Enter the message to pass : ";
        //cin>>str;
        strcpy(sendBuff, "HELLO .... HOW R U?");
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
  serv_addr.sin_port = htons(::port_num);
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  cout<<"I am here111\n";
  while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
      printf("\n Error : Connect Failed \n");
      //return 1;
    }
    cout<<"I am here222\n";

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
void server_fun()
{
	server();
}

void client_fun()
{
	char input[100];
	
	cout<<chord<<"Welcome to Chord DHT\n";
	cout<<chord<<"Type \"help\" for a list of recognized commands\n";

	while(1)
	{
		cout<<chord;
		gets(input);
		if(strcmp(input,"help") == 0)
		{
			help();
		}
		else if(strcmp(input,"create") == 0)
		{
			string str1;
			ifstream f("create.txt");
			f>>str1;
			f.close();

			/*if(str1.compare("1") == 0)
			{
				error();
			}
			else
			{
				create();
				client();

			}*/
				create();
				client();


		}
		else if(strstr(input,"port") != NULL)
		{
			int c = 0;
			char* tokens[10];
			int port_num = 0;
			tokenize(input,tokens,c);
			string str2;
			ifstream f("port.txt");
			f>>str2;
			f.close();

			if(str2.compare("1") == 0)
			{
				error();
			}
			else
			{

				if(c == 2)
				{
					port_num = atoi(tokens[1]);
					if(port_num == 0)
					{
						error();
					}
					else
					{
						port(port_num);
					}
				}
				else
				{
					error();
				}
			}
			
		}
		else if(strstr(input,"join") != NULL)
		{

		}
		else if(strstr(input,"get") != NULL)
		{
			int c = 0;
			char* tokens[10];
			tokenize(input,tokens,c);
			if (c == 2)
			{
				key = atoi(tokens[1]);
				value = get(key);
				if(value != "")
				{
					cout<<"<<"<<value<<">>\n";
				}
				else
				{
					cout<<"Error : Given key does not exist\n";
				}
			}
			else
			{
				error();
			}
		}
		else if(strstr(input,"put") != NULL)
		{
			int c = 0;
			char* tokens[10];
			tokenize(input,tokens,c);
			if (c == 3)
			{
				key = atoi(tokens[1]);
				value = (string)tokens[2];
				put(key,value);
			}
			else
			{
				error();
			}
		}
		else if(strcmp(input,"quit") == 0)
		{
			free();
			exit(0);
		}
	}
}

int main(int argc, char *argv[])
{

    port_num=atoi(argv[1]);

	create(&server_fun);
	create(&client_fun);	

	
	start();
	return 0;
}