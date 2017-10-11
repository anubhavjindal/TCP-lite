#include<iostream>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/time.h>
#include<queue>
#include<iterator>
#include<map>
#include<utility>
#include<vector>
#include<time.h>
#include<string>
// end  
#include<bits/stdc++.h>
using namespace std;
int sourceport;	// 
string source_ip;	// 

#define seconds 1000LL
#define micro 1000
#define recieveSize 25


//timeout

struct node
{
	string ip;
	int port;
	int sockfd;
};


//end timeout

//  
#define MSS 536 // as per default value
//function prototypes

long lrand();
bool validataIP(string IP);
bool validatePort (int port);

//end   


bool validateIP(string IP)  
{

  char *pch = strtok((char *)IP.c_str(),".");
  int c=0;

  while(pch!=NULL)
  {
      int no = atoi(pch);
      if(no>=0 && no<=255)  c=c+1;
      else
        { cout<<"\n Enter Valid IP address\n "; return false; }
      pch=strtok(NULL,".");
  }

  if(c==4)
    return true;
  else
    { cout<<"\n Enter Valid IP address \n"; return false; }

}

bool validatePort(int port) //   
{
    if(port>=0 and port < 1024)
    { cout<<"\n It is registered port. Enter Ephemeral port no.\n "; return false ; }
    else if(port>=1024 && port <65536)
    {
        return true;
    }

}

class TCPPacket
{
	public:
	int sourceport;
    int destiport;
    unsigned long long int seq_no;	// 
    unsigned long long int ack_no;	// 
    long long int data_offset;
    long long int window;
    char reserved[6];
    char URG;
    char ACK;
    char PSH;
    char RST;
    char SYN;
    char FIN;
    int checksum;
    char urg_pointer[16];
    char options[40*8];   //40 bytes of options
    char padding[32];   //40 bytes of option
    char data[MSS+5];
   
    //pseudo-header fields
    char src_ip[20];
    char desti_ip[20];
    int data_length;
    
    
    public:

       

        TCPPacket(int desport, char * data)
        {

        	this->destiport = desport;
        	strcpy(this->data,data);
        }
        TCPPacket()  //  
        {
                //Default Constructor to initialize for a packet to send
				
                destiport= 0;
                seq_no=0;
                ack_no=0;
                data_offset=0;
                window=0;
                strcpy(this->reserved,"");

                URG=ACK=PSH=RST=SYN=FIN='0';
                checksum=0;
                strcpy(this->urg_pointer,"");
                strcpy(this->options,"");
                strcpy(this->padding,"");
                strcpy(data,"");

                strcpy(src_ip,(char*)source_ip.c_str());
                
                data_length=0;
                

        }// end   

        TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
        {
            this->sourceport = sp;
            this->destiport = dp;
            this->seq_no = seq ;
            this->ack_no = ack;
            this->data_offset = offset;
            this->window = winsiz;
            
            strcpy(this->reserved,reser);	//  change order of source and destination
           
            this->URG = URG;
            this->ACK = ACK;
            this->PSH = PSH;
            this->RST = RST;
            this->SYN = SYN;
            this->FIN = FIN;
            this->checksum = checksum;
            
            strcpy(data,"");
            strcpy(src_ip,"");
            strcpy(desti_ip,"");
            
            strcpy(this->urg_pointer,urg_pointer); //  change order of source and destination
            strcpy(this->options,options); //  change order of source and destination
            data_length=0;
        }

        int getport()
        {
        	return destiport;
        }

        string getmessage()
        {
        	return data;
        }

};
///////////////////////////////////////////  
int findchecksum(TCPPacket* packet);
string ipadd(string ip);
string adddata(string str);
string toBinary(int num, int bits);
string addBitStrings( string first, string second );
int makeEqualLength(string &str1, string &str2);


enum ConnectionState {LISTEN, SYNSENT, SYNRCVD, ESTAB, FINWAIT1,FINWAIT2, CLOSEWAIT, CLOSING, LASTACK, TIMEWAIT,CLOSED};

class Connection
{
	// define the size of the send and recieve window
	public:
	string IP;
	int port;
	ConnectionState state;
	unsigned long long int sequence_no;
	unsigned long long int acknowledgement_no;
	vector<TCPPacket*> sendWindow;
	vector<TCPPacket*> recieveWindow;
	vector<TCPPacket*> :: iterator cwnd_start;
	vector<TCPPacket*> :: iterator cwnd_end;
	vector <pair<int,int> > sw;
	int slidingwindow;		//QUESTION
	vector<pair<TCPPacket*,long long> > timestamp;
	float cwnd_size;
	int ssthresh;
	int rcvWindSize;	//  v4
	long long int rtt;
	long long int rto;
	float weightage;
	int terminate;
	
	
	
	Connection(){}
	
	
	Connection(string IP,int port,ConnectionState state, int sequence_no, int ack_no)
	{
		this->IP=IP;
		this->port=port;
		this->state=state;
		this->sequence_no=sequence_no;
		this->acknowledgement_no=ack_no;  //  
		//this->sendWindow;		//  
		//this->recieveWindow=NULL;		//  
		//this->cwnd_start=send.begin();				//  
		//this->cwnd_end=send.end();				//  
		slidingwindow=0;
		cwnd_size=1;  //  in terms of packets = No.ofPackets*(Sizeof(TCPPacket))  //  
		
		this->rcvWindSize=recieveSize*MSS;	//  v4
		this->slidingwindow=0;
		weightage=0.9;
		terminate=0;
	}


	
	
};
int packet=0;
int terminated=0;
queue<TCPPacket*> recieveQ;
map<pair<string,int>,Connection> connection;
void* throughputThread(void* arg)
{
	int count=0;
	int timeint=100000;//in milliseconds
	while(1)
	{
		usleep(timeint);
		count++;
		int bytesSent=packet*MSS;
	//	cout<<endl<<"Packets = "<<packet<<endl<<"Count = "<<count<<endl<<"Packets = "<<(bytesSent)<<endl;
		packet=0;
		if(terminated==1)
			pthread_exit(NULL);
	}






}
void *timeoutThread(void *argument)
{
	struct node* p=(struct node*)argument;
	string ip=p->ip;
	int port=p->port;
	int sockfd=p->sockfd;
	map<pair<string,int>,Connection>::iterator itr=connection.find(pair<string,int>(ip,port));
	while(1)
	{
		
		if(itr->second.timestamp.size()>0)
		{
			struct timeval te; 
			gettimeofday(&te, NULL); // get current tim
			long long currenttime = te.tv_sec*seconds + te.tv_usec/micro; // caculate milliseconds 
			//cout<<endl<<"difference --- "<<currenttime-itr->second.timestamp[0].second<<endl;
			if((currenttime-itr->second.timestamp[0].second)>itr->second.rto)	//timeout occurred
			{
				TCPPacket *packet=itr->second.timestamp[0].first;
				itr->second.timestamp.erase(itr->second.timestamp.begin(),itr->second.timestamp.begin()+1);
				struct sockaddr_in addr;
				bzero(&addr,sizeof(addr));
		   
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr=inet_addr(ip.c_str()); //   -- change source_ip to obj->src_ip
				addr.sin_port=htons(port);
		   
				sendto(sockfd,packet,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) ;
				packet++;
				gettimeofday(&te, NULL); // get current tim
				long long time2 = te.tv_sec*seconds + te.tv_usec/micro; // caculate milliseconds 
				
				itr->second.timestamp.push_back(pair<TCPPacket*,long long>(packet,time2));
				itr->second.ssthresh=itr->second.cwnd_size/2;
				itr->second.cwnd_size=1;
				cout<<endl<<"TIMEOUT occurred : "<<packet->seq_no<<endl;
			
			}
			
			
			
		}
		
			//return milliseconds;
	}

}




void *recieveQService(void *sock_fd)
{
	TCPPacket *obj;
	while(true)
	{
		if(recieveQ.size()>0)
		{
			
			obj=recieveQ.front();
			
			recieveQ.pop();
			string packetsource=obj->src_ip;
			
			if(connection.find(pair<string,int>(packetsource,obj->sourceport))!=connection.end())	//   -- change source_ip to obj->src_ip
			// connection already established with this ip + port
			{
				map<pair<string,int>,Connection>::iterator itr=connection.find(pair<string,int>(packetsource,obj->sourceport));
				Connection conObj=itr->second; 
				if(obj->SYN=='1' && obj->ACK=='1')
				{
					cout<<endl<<"SYN+ACK recieved";
					cout<<endl<<"Sequence number of packet recieved: "<<obj->seq_no;
					(itr->second).sequence_no++;
					
					//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
					TCPPacket *tcpobj=new TCPPacket(sourceport,obj->sourceport,(itr->second).sequence_no,obj->seq_no+1,0,0,(char*)"",'0','1','0','0','0','0',0,(char*)"",(char*)"");
					strcpy(tcpobj->src_ip,(char*)source_ip.c_str());	
					strcpy(tcpobj->desti_ip,(char*)packetsource.c_str()); //   -- change source_ip to obj->src_ip
					(itr->second).state=ESTAB;
					(itr->second).ssthresh=obj->window;
					(itr->second).acknowledgement_no=obj->seq_no+1;	//  v5
					int sockfd=*((int*)sock_fd);
					struct sockaddr_in addr;
					bzero(&addr,sizeof(addr));
					//(itr->second).sequence_no++;
					addr.sin_family = AF_INET;
					addr.sin_addr.s_addr=inet_addr(packetsource.c_str());
					addr.sin_port=htons(obj->sourceport);
			   
					sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) ;
					packet++;
					struct timeval te; 
					gettimeofday(&te, NULL); // get current tim
					long long milliseconds2 = te.tv_sec*seconds + te.tv_usec/micro;
					itr->second.rtt=milliseconds2-itr->second.rtt;
					itr->second.rto=2*itr->second.rtt;
					
					cout<<"\n ACK sent for SYN+ACK";
					cout<<endl<<"ACK number: "<<(itr->second).sequence_no<<endl;
					struct node p;
					p.ip=source_ip;	
					p.port=obj->sourceport;
					p.sockfd=*((int*)sock_fd);
					pthread_t timeoutthread;
					pthread_create( &timeoutthread, NULL, timeoutThread, &p);
					cout<<endl<<"Connection state: "<<(ConnectionState)(itr->second).state<<endl;
					
				}
				else if(obj->FIN=='1' && obj->ACK=='0')	
				/*	recieve FIN.. mayb be 3 cases
				 *  1. May be in ESTAB state and enter CLOSE WAIT phase
				 *  2. May be in FINWAIT2 phase and enter TIMEWAIT phase
				 *  3. May be in FINWAIT1 phase and enter CLOSING phase
				 *  Entry in map should be deleted when going to CLOSED phase
				 */
				{
					cout<<endl<<"FIN Recieved"<<endl;
					int state=(itr->second).state;
					if(state==ESTAB)	// case 1
					{
						(itr->second).state=CLOSEWAIT;
						cout<<"\n ACK sent for FIN\n Connection state: CLOSEWAIT "<<endl;
						
					}
					else if(state==FINWAIT1)	//case 3
					{	
						(itr->second).state=CLOSING;
						cout<<"\n ACK sent for FIN\n Connection state: CLOSING"<<endl;
						
					}
					else if(state==FINWAIT2)	// case 2
					{
						(itr->second).state=TIMEWAIT;
						cout<<"\n ACK sent for FIN\n Connection state: TIMEWAIT"<<endl;
						(itr->second).state=CLOSED;
						connection.erase(itr);
						if(connection.find(pair<string,int>(packetsource,obj->sourceport))==connection.end())	//   -- change source_ip to obj->src_ip
							cout<<endl<<"CLOSED and deleted from map"<<endl;
					}
					//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
					TCPPacket *tcpobj=new TCPPacket(sourceport,obj->sourceport,0,obj->seq_no+1,0,0,(char*)"",'0','1','0','0','0','0',0,(char*)"",(char*)"");
					strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
					strcpy(tcpobj->desti_ip,(char*)packetsource.c_str());
					int sockfd=*((int*)sock_fd);
					struct sockaddr_in addr;
					bzero(&addr,sizeof(addr));
			   
					addr.sin_family = AF_INET;
					addr.sin_addr.s_addr=inet_addr(packetsource.c_str());
					addr.sin_port=htons(obj->sourceport);
			   
					sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) ;
					packet++;
					
				}
				else if(obj->FIN=='0' && obj->SYN=='0' && obj->ACK=='1')
				/*
				 * case 1: ACK for SYN+ACK - will be in SYNRCVD phase
				 * case 2: recieve ACK --- is in FINWAIT1 phase and goes to FINWAIT2 phase
				 * case 3: recieve ACK --- is in CLOSING phase and goes to TIMEWAIT phase
				 * case 4: recieve ACK --- is in LASTACK phase and goes to CLOSED phase
				 */
				{
					int state=(itr->second).state;
					if(state==SYNRCVD)
					{
						//if(obj->seq_no==(itr->second).acknowledgement_no-1)	//sequence no matching what is expceted
						{
							cout<<endl<<"ACK recieved fro SYN+ACK";
							
							(itr->second).state=ESTAB;
							cout<<endl<<"Connection state: "<<(itr->second).state<<endl;
							struct timeval te; 
							gettimeofday(&te, NULL); 
							long long milliseconds2 = te.tv_sec*seconds + te.tv_usec/micro;
							itr->second.rtt=milliseconds2-itr->second.rtt;
							itr->second.rto=itr->second.rtt*2;
							//cout<<endl<<"RTO = "<<itr->second.rto<<endl;
							
							struct node p;
							p.ip=packetsource;	
							p.port=obj->sourceport;
							p.sockfd=*((int*)sock_fd);
							pthread_t timeoutthread;
							pthread_create( &timeoutthread, NULL, timeoutThread, &p);
							
						}
						//else
						{
							//not matching so drop ACK
						}
					}
					else if(state==FINWAIT1)
					{
						(itr->second).state=FINWAIT2;
						cout<<"\n ACK recieved for FIN\n Connection state: FINWAIT2"<<endl;
					}
					else if(state==CLOSING)
					{
						(itr->second).state=TIMEWAIT;
						cout<<"\n ACK recieved for FIN\n Connection state: TIMEWAIT state"<<endl;
						(itr->second).state=CLOSED;
						connection.erase(itr);
						if(connection.find(pair<string,int>(packetsource,obj->sourceport))==connection.end())	
							cout<<endl<<"CLOSED and deleted from map"<<endl;
					}
					else if(state==LASTACK)
					{
						(itr->second).state=CLOSED;
						connection.erase(itr);
						if(connection.find(pair<string,int>(packetsource,obj->sourceport))==connection.end())	
							cout<<endl<<"CLOSED and deleted from map"<<endl;
					}
					
					else if(state == ESTAB)
					{
						
						int ack_number = obj->ack_no;
						int start_seq=(itr->second.sendWindow[0])->seq_no;
						int c=0;

							cout<<"\nACK recieved for sequence number: "<<ack_number;//<<"  Seq Number :: "<<start_seq<<"  "<<'\n';


						if(itr->second.sequence_no ==ack_number)
						{
							struct timeval te; 
							gettimeofday(&te, NULL); 
							long long milliseconds2 = te.tv_sec*seconds + te.tv_usec/micro;
							
							milliseconds2=milliseconds2-(itr->second.timestamp.end()-1)->second;
							if(milliseconds2<1000)
							{
								itr->second.rtt=itr->second.weightage*itr->second.rtt+(1-itr->second.weightage)*milliseconds2;
								itr->second.rto=itr->second.rtt*2;
							}
							
							if(itr->second.cwnd_size*MSS<=itr->second.ssthresh)
							{
								if(itr->second.cwnd_size*MSS+MSS<obj->window)
									itr->second.cwnd_size++;
								else
									itr->second.cwnd_size=(obj->window)/MSS;
							}
							else
							{
								if(itr->second.cwnd_size+1/itr->second.cwnd_size<(obj->window/MSS))
									itr->second.cwnd_size=itr->second.cwnd_size+1/itr->second.cwnd_size;
								else
									itr->second.cwnd_size=(obj->window/MSS);
							}	
							
							
							itr->second.sendWindow.erase(itr->second.sendWindow.begin(),itr->second.sendWindow.end());
							itr->second.sw.erase(itr->second.sw.begin(),itr->second.sw.end());
							itr->second.timestamp.erase(itr->second.timestamp.begin(),itr->second.timestamp.end());
							
							
											
						}
						else if(start_seq<=ack_number && ack_number <=(start_seq+itr->second.cwnd_size*MSS))
						{
							
							if(itr->second.cwnd_size*MSS<=itr->second.ssthresh)
							{
								if(itr->second.cwnd_size*MSS+MSS<obj->window)
									itr->second.cwnd_size++;
								else
									itr->second.cwnd_size=obj->window/MSS;
							}
							else
							{
								if(itr->second.cwnd_size+1/itr->second.cwnd_size<obj->window/MSS)
									itr->second.cwnd_size=itr->second.cwnd_size+1/itr->second.cwnd_size;
								else
									itr->second.cwnd_size=obj->window/MSS;
							}	
							vector<pair<int,int> > :: iterator itr1;
							for(itr1=itr->second.sw.begin();itr1!=itr->second.sw.end();++itr1)
							{

								if(itr1->first==ack_number)
								{
									itr->second.sw.erase(itr->second.sw.begin(),itr1);
									itr->second.sendWindow.erase(itr->second.sendWindow.begin(), itr->second.sendWindow.begin()+c);
									itr1->second=itr1->second+1;
									
									vector<pair<TCPPacket*,long long> >::iterator itr2;
									for(itr2  = itr->second.timestamp.begin(); itr2!=itr->second.timestamp.end();itr2++)
									{
										int seq_number=itr2->first->seq_no;
										if(seq_number<ack_number)	
										{   
											struct timeval te; 
											gettimeofday(&te, NULL); 
											long long milliseconds2 = te.tv_sec*seconds + te.tv_usec/micro;
											milliseconds2=milliseconds2-itr2->second;
											if(milliseconds2<1000)
											{
												itr->second.rtt=itr->second.weightage*itr->second.rtt+(1-itr->second.weightage)*milliseconds2;
												itr->second.rto=itr->second.rtt*2;
											}
											itr2=itr->second.timestamp.erase(itr2);
											itr2--;
											
										}
										
										
									}
									

										if(itr1->second==4)
										{
											cout<<"\nTriple Duplicate Ack recieved.";
											TCPPacket *ob1=itr->second.sendWindow[c];
											int sockfd=*((int*)sock_fd);
											struct sockaddr_in addr;
											bzero(&addr,sizeof(addr));
									   
											addr.sin_family = AF_INET;
											addr.sin_addr.s_addr=inet_addr((char*)ob1->desti_ip); 
											addr.sin_port=htons(ob1->destiport);
									   
											sendto(sockfd,ob1,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) ;
											packet++;
											itr->second.ssthresh=itr->second.cwnd_size/2;
											itr->second.cwnd_size=itr->second.cwnd_size/2;
										}
										break;
								}
								c=c+1;
							}
						}
					

					}
					
					
				}
				else //data
				{
					//nitika
					//int packetChecksum=findchecksum(obj);
					//if(packetChecksum==obj->checksum)
					{
						Connection *connObj=&(itr->second);
						int seq=obj->seq_no;
						
						
						cout<<endl<<"Sequence number of packet recieved: "<<obj->seq_no;
						if(seq>=connObj->acknowledgement_no && seq<=connObj->acknowledgement_no+connObj->rcvWindSize-1)
						{
							vector<TCPPacket*>::iterator tcpitr;
						
							if(seq==connObj->acknowledgement_no)	// first packet in window recieved
							{
								vector<TCPPacket*>::iterator tcpitr;
								
								
								int count=0;
								seq=seq+MSS;
								for(tcpitr=(connObj->recieveWindow).begin();tcpitr!=(connObj->recieveWindow).end();tcpitr++)
								{
									
									if((*tcpitr)->seq_no==seq)
									{
										seq=seq+(*tcpitr)->data_length;
										count++;
									}
									else if((*tcpitr)->seq_no>seq)
										break;
									
								}
								if(count>0)
									(connObj->recieveWindow).erase((connObj->recieveWindow).begin(),(connObj->recieveWindow).begin()+count+1);
								cout<<endl<<"Sending ACK for sequence number: "<<seq<<endl;
								connObj->acknowledgement_no=seq;
								//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
								TCPPacket *tcpobj=new TCPPacket(sourceport,obj->sourceport,0,seq,0,0,(char*)"",'0','1','0','0','0','0',0,(char*)"",(char*)"");
								strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
								strcpy(tcpobj->desti_ip,(char*)packetsource.c_str());//   -- change source_ip to obj->src_ip
								tcpobj->window=connObj->rcvWindSize-(connObj->recieveWindow.size())*MSS;
								
								int sockfd=*((int*)sock_fd);
								struct sockaddr_in addr;
								bzero(&addr,sizeof(addr));
						   
								addr.sin_family = AF_INET;
								addr.sin_addr.s_addr=inet_addr(packetsource.c_str()); //   -- change source_ip to obj->src_ip
								addr.sin_port=htons(obj->sourceport);
						   
								sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) ;
								packet++;
							}
							else // send ACK for connObj->acknowledgement_no
							{
								if(connObj->recieveWindow.size()==0)
								{
									connObj->recieveWindow.push_back(obj);
									cout<<endl<<"Sending ACK for sequence number: "<<connObj->acknowledgement_no<<endl;
									//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
									TCPPacket *tcpobj=new TCPPacket(sourceport,obj->sourceport,0,connObj->acknowledgement_no,0,0,(char*)"",'0','1','0','0','0','0',0,(char*)"",(char*)"");
									tcpobj->window=connObj->rcvWindSize-(connObj->recieveWindow.size())*MSS;
									strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
									strcpy(tcpobj->desti_ip,(char*)packetsource.c_str()); 
									int sockfd=*((int*)sock_fd);
									struct sockaddr_in addr;
									bzero(&addr,sizeof(addr));
							   
									addr.sin_family = AF_INET;
									addr.sin_addr.s_addr=inet_addr(packetsource.c_str()); 
									addr.sin_port=htons(obj->sourceport);
							   
									sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) ;
									packet++;
								}
								else
								{
									int flag=0;
									
									for(vector<TCPPacket*>::iterator tcpitr=(connObj->recieveWindow).begin();tcpitr!=(connObj->recieveWindow).end();tcpitr++)
									{
										if((*tcpitr)->seq_no>seq)
										{
											(connObj->recieveWindow).insert(tcpitr,obj);
											cout<<endl<<"Sending ACK for sequence number: "<<connObj->acknowledgement_no<<endl;
											//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
											TCPPacket *tcpobj=new TCPPacket(sourceport,obj->sourceport,0,connObj->acknowledgement_no,0,0,(char*)"",'0','1','0','0','0','0',0,(char*)"",(char*)"");
											tcpobj->window=connObj->rcvWindSize-(connObj->recieveWindow.size())*MSS;
											strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
											strcpy(tcpobj->desti_ip,(char*)packetsource.c_str()); 
											int sockfd=*((int*)sock_fd);
											struct sockaddr_in addr;
											bzero(&addr,sizeof(addr));
									   
											addr.sin_family = AF_INET;
											addr.sin_addr.s_addr=inet_addr(packetsource.c_str());
											addr.sin_port=htons(obj->sourceport);
									   
											sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) ;
											packet++;
											flag=1;
											break;
										}
										if((*tcpitr)->seq_no==seq)
										{
											flag=1;
											break;
											// DROP as duplicate packet
										}
								
									}
									if(flag==0)
									{
										connObj->recieveWindow.push_back(obj);
										cout<<endl<<"Sending ACK for sequence number: "<<connObj->acknowledgement_no<<endl;
										//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
										TCPPacket *tcpobj=new TCPPacket(sourceport,obj->sourceport,0,connObj->acknowledgement_no,0,0,(char*)"",'0','1','0','0','0','0',0,(char*)"",(char*)"");
										tcpobj->window=connObj->rcvWindSize-(connObj->recieveWindow.size())*MSS;
										strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
										strcpy(tcpobj->desti_ip,(char*)packetsource.c_str());
										int sockfd=*((int*)sock_fd);
										struct sockaddr_in addr;
										bzero(&addr,sizeof(addr));
								   
										addr.sin_family = AF_INET;
										addr.sin_addr.s_addr=inet_addr(packetsource.c_str()); 
										addr.sin_port=htons(obj->sourceport);
								   
										sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) ;
										packet++;
									}
								}
								
							}
						}
						else
						{
							//not in window so drop!
						}
						
					
					}
					/*nitika
					 * else
					{
						cout<<endl<<"Checksum does not match!"<<endl;
					}
					
					*/
					
					
					
				}
				
				
			}
			else if(obj->SYN=='1' && obj->ACK=='0')	
			{
				cout<<endl<<"SYN recieved";
				cout<<"\nSequence number of packet recieved: "<<obj->seq_no;
				srand(time(NULL));
				Connection *conObj=new Connection(packetsource,obj->sourceport,SYNRCVD,rand()%10000,obj->seq_no+1);	
				
				conObj->ssthresh=obj->window;
				
				struct timeval te; 
				gettimeofday(&te, NULL); // get current tim
				long long milliseconds2 = te.tv_sec*seconds + te.tv_usec/micro;
				conObj->rtt=milliseconds2;
				
				connection[pair<string,int>(packetsource,obj->sourceport)]=*conObj;
				//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
				TCPPacket *tcpobj=new TCPPacket(sourceport,obj->sourceport,conObj->sequence_no,obj->seq_no+1,0,0,(char*)"",'0','1','0','0','1','0',0,(char*)"",(char*)"");
				strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
				strcpy(tcpobj->desti_ip,(char*)packetsource.c_str());	
				tcpobj->window=conObj->rcvWindSize;
				
				
				int sockfd=*((int*)sock_fd);
				struct sockaddr_in addr;
				bzero(&addr,sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr=inet_addr(packetsource.c_str()); 
				addr.sin_port=htons(obj->sourceport);
				
				sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) ;
				packet++;
				cout<<"\n SYN+ACK sent";
				cout<<"\n Sequence number of packet sent: "<<tcpobj->seq_no<<endl;
			}
			
		}
	}
	
	
	
	
	
}





void *listen(void *sock_fd)
{
	int sockfd=*((int*)sock_fd);
	struct sockaddr_in addr;
	socklen_t len;
	int n;
 
	TCPPacket *obj= new TCPPacket();
	while(true)
	{

      
	  len = sizeof(addr);

      n = recvfrom(sockfd,obj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,&len);
      recieveQ.push(obj);	      
      
      
     
	}
	
}



void *send(void *sock_fd)
{
	int sockfd=*((int*)sock_fd);
	struct sockaddr_in addr;
	struct sockaddr_in servaddr,cliaddr;
		
	string ip,msg;
	int portInt;
	

	while(1)
	{
		


		//do
	    //{
			
			string msg="";
		//do
		{
	      cout<<endl<<" \n Enter Destination IP address :: "<<endl;
	      cin>>ip;
	      cout<<endl<<"\n Enter Destination Port no. :: "<<endl;
	      cin>>portInt;
		}//while(validateIP(ip)==false || validatePort(portInt)==false);
		
	     map<pair<string,int>,Connection>::iterator itr;


		TCPPacket *obj1;
		// 
		if(connection.find(pair<string,int>(ip,portInt))==connection.end())
		{
			
			//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
			srand(time(NULL));
			Connection *conObj=new Connection(ip,portInt,SYNSENT,rand()%100,0);
			//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
			struct timeval te; 
			gettimeofday(&te, NULL); // get current tim
			long long milliseconds2 = te.tv_sec*seconds + te.tv_usec/micro;
			conObj->rtt=milliseconds2;
			TCPPacket *obj2=new TCPPacket(sourceport,portInt,conObj->sequence_no,0,0,0,(char*)"",'0','0','0','0','1','0',0,(char*)"",(char*)"");
			strcpy(obj2->src_ip,(char*)source_ip.c_str());
			strcpy(obj2->desti_ip,(char*)ip.c_str());
			obj2->data_length=0; //Length of data as of now
			obj2->window=conObj->rcvWindSize;
			
			connection[pair<string,int>(ip,portInt)]=*conObj;
			bzero(&addr,sizeof(addr));

			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr=inet_addr((char*)ip.c_str());//inet_addr("127.0.0.1");//htonl(INADDR_ANY);
			addr.sin_port=htons(portInt);
			

			sendto(sockfd,obj2,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr));
			packet++;
			
			cout<<endl<<"SYN SENT \n"<<"Sequence number of packet sent: "<<(*conObj).sequence_no<<endl;
		}
		else
		{
			char filename[50];

			ifstream input_file;
  			 cout<<"\n Enter filename to send :: ";
		     cin>>filename;
		    
		     if(strcmp(filename,"FIN")==0)
		     {
				cout<<endl<<"FIN Sent"<<endl;
				itr =connection.find(pair<string,int>(ip,portInt));
				
				if(itr->second.state==ESTAB)
					itr->second.state=FINWAIT1;
				else if(itr->second.state==CLOSEWAIT)
					itr->second.state=LASTACK;
				int stemp=itr->second.sequence_no + MSS;//length of last segment pushed in sendQ then update
				//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
				TCPPacket *tcpobj= new TCPPacket(sourceport,portInt,itr->second.sequence_no,0,0,0,(char*)"",'0','0','0','0','0','1',0,(char*)"",(char*)"");
				strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
				strcpy(tcpobj->desti_ip,(char*)ip.c_str());
				tcpobj->data_length=MSS;
				tcpobj->checksum=0;
				
				bzero(&addr,sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr=inet_addr((char*)ip.c_str());
				addr.sin_port=htons(portInt);
				
				cout<<"Sequence number of packet sent : "<<tcpobj->seq_no<<endl;
				cout << sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) << " chars send successfully " << endl;
				 
			 }
			 else
			 {
					 input_file.open (filename, ios::in);
					 if ( input_file.fail() )     
					  {
					  cout << "open for input.txt failed";
					  exit(-1);
					  }

					pthread_t throughputthread;
					pthread_create( &throughputthread , NULL, throughputThread  , (void*)NULL);
					
					
		   
					 unsigned char buffer[MSS+5]="";
					 int segno=0;
					while (input_file.read((char *)buffer, MSS))
					{
							segno+=1;
							getchar();
							string msg((char*)buffer);
							 int msglen=msg.size();


					
						if(msg.size()!=0)	
						{
							int len=msg.size();

							if(len<=MSS)   //No segmentation required
							{
								itr =connection.find(pair<string,int>(ip,portInt));
								//make packet

								//To assign sequence number to next packet
								int stemp=itr->second.sequence_no + MSS;//length of last segment pushed in sendQ then update
							
								//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
								TCPPacket *tcpobj= new TCPPacket(sourceport,portInt,itr->second.sequence_no,0,0,0,(char*)"",'0','0','0','0','0','0',0,(char*)"",(char*)"");
								strcpy(tcpobj->data,msg.c_str());
								strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
								strcpy(tcpobj->desti_ip,(char*)ip.c_str());
								tcpobj->data_length=MSS; 
								tcpobj->checksum=0;
								tcpobj->checksum=findchecksum(tcpobj);
								
								//update sequence number
								itr->second.sequence_no=stemp;
								// If queue is full, dont push packet.
								(itr->second.sendWindow).push_back(tcpobj);
								itr->second.cwnd_start = (itr->second.sendWindow).end()-1;
								itr->second.cwnd_end = (itr->second.sendWindow).end()-1;
								bzero(&addr,sizeof(addr));

								addr.sin_family = AF_INET;
								addr.sin_addr.s_addr=inet_addr((char*)ip.c_str());
								addr.sin_port=htons(portInt);
								
								sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr));//<< " chars send successfully " << endl;
								cout<<endl<<"Number of bytes sent: "<<MSS;
								cout<<"\nSequence number of packet sent: "<<tcpobj->seq_no<<endl;
								packet++;
								if(itr->second.sw.size()==0)
								{
									itr->second.sw.push_back(pair<int,int>(itr->second.sequence_no,1));
								}
								else
								itr->second.sw.push_back(pair<int,int>(itr->second.sequence_no,0));
								
								struct timeval te; 
								gettimeofday(&te, NULL); // get current tim
								long long milliseconds = te.tv_sec*seconds + te.tv_usec/micro;
								itr->second.timestamp.push_back(pair<TCPPacket*,long long>(tcpobj,milliseconds));
							}

							else      //segmentation required
							{
							itr=connection.find(pair<string,int>(ip,portInt));
								int len=msg.size(), stemp=0;
								int i=0;
								while(len>0)
								{
									string temp="";
									if(len>=MSS)
									{
									len=len-MSS;
									stemp=itr->second.sequence_no + MSS;
									temp=msg.substr(i*MSS,MSS);
									}
									else
									{
									stemp=itr->second.sequence_no+MSS; //change
									temp=msg.substr(i*MSS);
									len=0;
									
									}
				
									TCPPacket *tcpobj= new TCPPacket(sourceport,portInt,itr->second.sequence_no,0,0,0,(char*)"",'0','0','0','0','0','0',0,(char*)"",(char*)"");
									strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
									strcpy(tcpobj->desti_ip,(char*)ip.c_str());
									//update sequence number
									itr->second.sequence_no=stemp;
									strcpy(tcpobj->data,(char*)temp.c_str());
									tcpobj->data_length=MSS;
									(itr->second.sendWindow).push_back(tcpobj);
				
									
								
						
									// If queue is full, dont update end iterator
				
									if(itr->second.cwnd_size < itr->second.slidingwindow) //Packets pushed in outstanding window
									{
										itr->second.slidingwindow+=1;
										itr->second.cwnd_end = (itr->second.sendWindow).end()-1;
										bzero(&addr,sizeof(addr));
				
										addr.sin_family = AF_INET;
										addr.sin_addr.s_addr=inet_addr((char*)ip.c_str());//inet_addr("127.0.0.1");//htonl(INADDR_ANY);
										addr.sin_port=htons(portInt);
				
										struct timeval te; 
										gettimeofday(&te, NULL); // get current tim
										long long milliseconds = te.tv_sec*seconds + te.tv_usec/micro;
										itr->second.timestamp.push_back(pair<TCPPacket*,long long>(tcpobj,milliseconds));
										tcpobj->checksum=0;
										tcpobj->checksum=findchecksum(tcpobj);
										sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr));
										
										cout<<endl<<"Number of bytes sent: "<<MSS;
										cout<<"\nSequence number of packet sent: "<<tcpobj->seq_no<<endl;
										
										packet++;
										if(itr->second.sw.size()==0)
										{
											itr->second.sw.push_back(pair<int,int>(itr->second.sequence_no,1));
										}
										else
											itr->second.sw.push_back(pair<int,int>(itr->second.sequence_no,0));
										cout << __LINE__ << endl;
									}
									else  //Packets outside outstanding window
									{
				
									//Do nothing
									}
									//itr->cwnd_end = tcpobj;
									i++;
								}

							}



						}
						else   // wants to send a FIN
						{
							/*
							* To send a FIN , there are 2 cases:
							* 1. is in ESTAB phase and enters FINWAIT1 phase
							* 2. is in CLOSEWAIT phase and enters LASTACK phase
							*/
							map<pair<string,int>,Connection>::iterator itr=connection.find(pair<string,int>(ip,portInt));
							int state=(itr->second).state;
							//cout<<endl<<"Current state: "<<state<<endl;
							if(state==ESTAB)	//case 1
							{
								(itr->second).state=FINWAIT1;
								cout<<"\n FIN sent --  enters FINWAIT1 stage"<<endl;
							}
							else if(state==CLOSEWAIT)	//case 2
							{
								(itr->second).state=LASTACK;
								cout<<"\n FIN sent --  enters LASTACK stage"<<endl;
							}
							//TCPPacket(int sp,int dp, int seq,int ack,int offset,int winsiz, char reser[],char URG, char ACK, char PSH, char RST, char SYN, char FIN, int checksum, char urg_pointer[],char options[])
							TCPPacket *tcpobj=new TCPPacket(sourceport,portInt,(itr->second).sequence_no,(itr->second).acknowledgement_no,0,0,(char*)"",'0','0','0','0','0','1',0,(char*)"",(char*)"");
							
							
								//   -- change source_ip to obj->src_ip
							strcpy(tcpobj->src_ip,(char*)source_ip.c_str());
							strcpy(tcpobj->desti_ip,(char*)ip.c_str());
							
							bzero(&addr,sizeof(addr));
					   
							addr.sin_family = AF_INET;
							addr.sin_addr.s_addr=inet_addr((char*)ip.c_str());//inet_addr("127.0.0.1");//htonl(INADDR_ANY);
							addr.sin_port=htons(portInt);
					   
							
							cout<< "sending the data  " << tcpobj->getmessage() << endl;
							cout << sendto(sockfd,tcpobj,sizeof(TCPPacket),0,(struct sockaddr *)&addr,sizeof(addr)) << " chars send successfully " << endl;
							packet++;
							cout << __LINE__ << endl;
							
							
							
						}
						
					}
					terminated=1;
			 }
		
		}
		

		
   }
	
	
	
	
}



int main(int argc, char**argv)
{
	pthread_t listenThread,sendThread;
	if (argc != 3)
   {
      cout<<"Give IP and port as arguments.\n";
      exit(1);
   }
   
   int sockfd,n;
   struct sockaddr_in addr,destaddr;
   socklen_t len;
   
   sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
   
   bzero(&addr,sizeof(addr));
   addr.sin_family = AF_INET;
   //cout<<(htonl(INADDR_ANY))<<endl;
   addr.sin_addr.s_addr=inet_addr(argv[1]);//inet_addr("127.0.0.1");//htonl(INADDR_ANY);
   addr.sin_port=htons(atoi(argv[2]));
   sourceport=atoi(argv[2]);	// 
   source_ip=argv[1];	// 
   cout<<"\n Sourceport = "<<sourceport<<"\n"<<"Source ip = "<<source_ip<<endl;
   if(bind(sockfd,(struct sockaddr *)&addr,sizeof(addr))==-1)
		cout<<"\nError in connecting.\n";
   
   /*
   //other address
   bzero(&destaddr,sizeof(destaddr));
   destaddr.sin_family = AF_INEqT;
   //cout<<(htonl(INADDR_ANY))<<endl;
   destaddr.sin_addr.s_addr=inet_addr(argv[3]);//inet_addr("127.0.0.1");//htonl(INADDR_ANY);
   destaddr.sin_port=htons(atoi(argv[4]));
*/

//  
	pthread_t recieveQServiceThread;
   pthread_create(&recieveQServiceThread,NULL,recieveQService,&sockfd);
  //End   
   
   
   
   pthread_create(&listenThread,NULL,listen,&sockfd);
   pthread_create(&sendThread,NULL,send,&sockfd);
   pthread_join(listenThread,NULL);
   pthread_join(sendThread,NULL);
   pthread_join(recieveQServiceThread,NULL);	// 
	cout<<endl<<"end";

}
   
   
	
//adds the two bit strings and return the result
// The main function that adds two bit sequences and returns the addition
string addBitStrings( string first, string second )
{
    string result;  // To store the sum bits
 
    // make the lengths same before adding
    int length = makeEqualLength(first, second);
 
    int carry = 0;  // Initialize carry
 
    // Add all bits one by one
    for (int i = length-1 ; i >= 0 ; i--)
    {
        int firstBit = first.at(i) - '0';
        int secondBit = second.at(i) - '0';
 
        // boolean expression for sum of 3 bits
        int sum = (firstBit ^ secondBit ^ carry)+'0';
 
        result = (char)sum + result;
 
        // boolean expression for 3-bit addition
        carry = (firstBit & secondBit) | (secondBit & carry) | (firstBit & carry);
    }
 
    // if overflow, then add a leading 1
    if (carry)
        result = '1' + result;
 
    return result;
}
 
// Helper method: given two unequal sized bit strings, converts them to
// same length by aadding leading 0s in the smaller string. Returns the
// the new length
int makeEqualLength(string &str1, string &str2)
{
    int len1 = str1.size();
    int len2 = str2.size();
    if (len1 < len2)
    {
        for (int i = 0 ; i < len2 - len1 ; i++)
            str1 = '0' + str1;
        return len2;
    }
    else if (len1 > len2)
    {
        for (int i = 0 ; i < len1 - len2 ; i++)
            str2 = '0' + str2;
    }
    return len1; // If len1 >= len2
}
 

//converting int to binary
 string toBinary(int num, int bits) {
    vector<char> digits;
    for (int i = 0; i < bits; ++i) {
        digits.push_back(num % 2 + '0');
        num >>= 1;
    }
    return string(digits.rbegin(), digits.rend());
}

//adding the bits of data
string adddata(string str)
{
	int i;
	vector<string> out;
	string str3;
	for(i=0;i<=str.length()-1;)
	{		if(i==str.length()-1)
                          {
			   string str4="00000000";
                           str3=str4+toBinary(str[i],8);
			}
			else
			{ 
				string str1=toBinary(str[i],8);
				string str2=toBinary(str[i+1],8);
				str3=str1+str2;
			}
		//cout<<"str3 is"<<str3<<endl;
		out.push_back(str3);
		i=i+2;
	}
	while(out.size()!=1)
	{
		string first=out[0];
		string second=out[1];
		string a=addBitStrings(first,second);
		out.erase(out.begin(),out.begin()+2);
		out.push_back(a);
	}
	return out[0];
}

//adding ip addresses
string ipadd(string ip)
{
int len=ip.length(),i;
vector<string> vec;
string str1,str2,s;
for(i=0;i<len;i++)
{
if(ip[i]=='.')
{
vec.push_back(s);
s.clear();

}
else
s=s+ip[i];
}
vec.push_back(s);

string stri=toBinary(atoi(vec[0].c_str()),8);
string strin=toBinary(atoi(vec[1].c_str()),8);
string st=toBinary(atoi(vec[2].c_str()),8);
string m=toBinary(atoi(vec[3].c_str()),8);
str1=stri+strin;
str2=st+m;
string ret=addBitStrings(str1,str2);
	return ret;




}
///finding the checksum value...................
int findchecksum(TCPPacket* packet2)
{
	TCPPacket packet=*packet2;
	
	vector<string> toadd;
	string str=toBinary(packet.sourceport,16);
	string str2=toBinary(packet.destiport,16);
	//cout<<str2<<endl;
	//cout<<str<<endl;
	toadd.push_back(str);
	toadd.push_back(str2);
	string str3=toBinary(packet.data_length,16);
	//cout<<str3<<endl;
	toadd.push_back(str3);
	string strin(packet.data);
	string s=adddata(strin);
	//cout<<"s is"<<s<<endl;
	while(toadd.size()!=1)
	{
		
		string a=addBitStrings(toadd[0],toadd[1]);
		//cout<<a<<endl;
		toadd.erase(toadd.begin(),toadd.begin()+2);
		toadd.push_back(a);
	}
        string srcip=ipadd(packet.src_ip);
        string destip=ipadd(packet.desti_ip);
        string ipadd= addBitStrings(srcip,destip);
	string ret=addBitStrings(toadd[0],s);
        string fin=addBitStrings(ipadd,ret);
    return 0;
	//return stoi(fin,nullptr,2);
}

	
