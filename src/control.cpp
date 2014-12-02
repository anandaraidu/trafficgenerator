#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include "p.h"
#include "control.h"
#include<iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#include<iostream>
using namespace std;
extern int running;
static int sfd = 0;
int opensocket( char *ip)
{
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 


    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);  //port 5000 is the port

    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
	   exit(0); //exit now
       return 1;
    } 

	cout << "Socket opened succesfully \n";
	return sockfd;
}

int sendpkt(int sockfd,char *p,int len) {

    char *sendbuf = new char[len + sizeof(int)];
	int netlen = htonl(len);

	memcpy(sendbuf,(char *)&netlen,sizeof(int));

	//sendbuf += sizeof(int);

	memcpy(sendbuf+4,p,len);

	int sendlen = len + sizeof(int);
	int off = 0;

	int total = sendlen;

	while (1) { //keep sending 
		int n = write(sockfd,(sendbuf+off),(sendlen-off));
		off += n;
		if (off == sendlen) {
			break;
		}
	}
	
    return 0;
}

//Simplest possible generating/altering function for ports, we can actually
//write any arbitrarily complicated logic here.
short getDestPort(short val) {
	return val+1;
}

short getSrcPort(short val) {
	return 0;
}

#if 0
int bumpClientIp(char *p,void *cntrl,void *paket) {
	packet_t *pkt = (packet_t *)paket;
	struct ip *iph = (struct ip*) (p+ pkt->m_l3off);
	iph->ip_src.s_addr++; //simple
	return 0;
}
#endif
#if 0
typedef struct anyprocess {
	struct anyprocess *next;
	int (*process)(char *basep,void *any,void *pkt);//function pointer
} anyprocess_t;
#endif

int planTheRun(int nPkts,int nConcurent) {
	//each packet should be out nConcurent times before the first FIN is sent
	int trainLen = 0;
	if (nConcurent >= nPkts) {
		trainLen = nPkts;
	} else {
		trainLen = nConcurent;
	}
	return trainLen;
}

/** There are more strategies here:
 *  1. Play one packet per connection for each iteration of this function and return.
 *  2.Or else play the trainlen once
 *   3.play the complete trainlen as many times:
 *    The 3rd one again have 2 options:-
	     1.Play the packet n times and move to next
         2.Play the whole trainlen n -times
 * or even better: (We will implement this feature for now)---**implemented feature for now
 * 1 (10) -line1 (1) once
 * 1 2 (10) -line2 (1,2) once
 * 1 2 3 (10) line3 (1,2,3) once and so on
 * play (1 1 2 1 2 3) repeat this 10 times
 */

void playPacketNtimes(int n,packet_t *pack) {
	while (n--) {	
		cout << __func__ << endl;
		sendpkt(sfd,pack->m_pkt,pack->packetLen);
		//sendPacket(); the final whistle blower to send the packets to the driver
		
		anyprocess_t *any = pack->control;
		while (any) {
			any->process(pack->m_pkt,NULL,(void *)pack); //this will change the packet
			any = any->next;
		}
		
	}
	pack->nTimesPerIteration  -= n;
} 
//Starting from the given packet.
//n packets are sent out.Generally
//useful to simulate 
void playTillNPackets(packet_t *pack,int n) {
	while (n--) {
		playPacketNtimes(1,pack);
		pack = pack->next;
	}
}

void rampUpConnection(conn_t *conn) {
	for (int i=1;i <= conn->nPackets;i++) {
		cout << __func__ << endl;
		playTillNPackets(conn->packetHead,i);
	}
	cout << "Starting to sleep and will exit now\n";
	sleep(10);
	exit(0);
}

void rampUpThread(connections &conns,int times) {
	while (times--) {
		cout << __func__ << endl;
		map<connid_t *,conn_t *,conncomp>::iterator it = conns.m_conns.begin(); //may be we can make them tcp/udp connections
		for (;it != conns.m_conns.end();it++) {
			conn_t *conn = it->second;
			rampUpConnection(conn);
		}
	}
}

void playConnection() {
}
void normalPlayThread(connections &conns,int times) {

	cout << "Entering the normal run-model \n";
	while (1) {
		map<connid_t *,conn_t *,conncomp>::iterator it = conns.m_conns.begin(); //may be we can make them tcp/udp connections
		for (;it != conns.m_conns.end();it++) {
			conn_t *conn = it->second;

			playTillNPackets(conn->packetHead,conn->nPackets);
		}
		sleep(1);
	}
}

void playNormal(conn_t *conn) { //Sends all the packets in the connection
	packet_t *p = conn->packetHead;
	while (p) {
		playPacketNtimes(1,p);
		p = p->next;
	}
}


//We should think more on the mixing capabilities and alternatives
//We should be able to mix packets from different connection interspersed
//probbably we can still do that as a second step.
//Assume we have a randomizer which randomizes the final send-Queue without touching the mixing
//algorithm(similar in line with another level of indirection will solve the current problem)
//So we are indirecting randomization on the final queue as a step of another level
//of indirection
void walkOverPackets(conn_t *conn) {
	int npkts = conn->nPackets;
	for (int i=0;i<npkts;i++) {
		packet_t *p = conn->packetHead;
		for (int j=0;j<=0;j++) {
			playPacketNtimes(1,p);
			p = p->next;
		}
	}
	--conn->cycles;
}

void walkOverConnections(connections &conns) {
	while (running) {
		map<connid_t *,conn_t *,conncomp>::iterator it = conns.m_conns.begin(); //may be we can make them tcp/udp connections
		while (it != conns.m_conns.end()) {
			conn_t *conn = it->second; //We got the pointer to the connection
			if (conn->cycles) {
				walkOverPackets(conn);
			} else {
				playNormal(conn); //just send each packet once, after applying necessary modification
			}
		}
	}
}

int configureConnections(connections &conns) {
	sfd = opensocket("127.0.0.1");
	map<connid_t *,conn_t *,conncomp>::iterator it = conns.m_conns.begin(); //may be we can make them tcp/udp connections
	int tLen = 0;
	while (it != conns.m_conns.end()) {
		conn_t *conn = it->second; //We got the pointer to the connection
		conn->trainLen = planTheRun(conn->nPackets,conn->nConcurent);
		tLen = conn->trainLen;
		cout << "Numpacket[" << conn->nPackets << "] TrainLen[" << conn->trainLen << "]\n";
		//playTillNPackets(conn->packetHead,conn->nPackets);
		++it;
	}
	
	//pp
	rampUpThread(conns,tLen);
	normalPlayThread(conns,1);
}


//
