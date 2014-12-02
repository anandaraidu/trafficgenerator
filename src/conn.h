#ifndef CONN_H
#define CONN_H
#include "packets.h"
#include "addr.h"
#include <map>
#include<stdlib.h>
#include<memory.h>

using namespace std;

typedef struct connid {
	address sAddr;
	address dAddr;
	u_int   sPort;
	u_int   dPort;
	short   l4Proto; //pro
    void printconn() {
		struct ip i;
		memset(&i,0,sizeof(struct ip));
		i.ip_src.s_addr = htonl(sAddr.ip);
		i.ip_dst.s_addr = htonl(dAddr.ip);

		//printf("Src[%s] Sport[%d] ",inet_ntoa(i.ip_src),sPort);
		//printf("Dst[%s] Dport[%d]\n",inet_ntoa(i.ip_dst),dPort);
	};
} connid_t;

typedef struct t_conn {
	connid_t *connid; //connection identifier of the structure

	struct t_conn *next; //next connection link if any
	struct t_conn *prev; //prev conection link if any

	packet_t *packetHead; //list of packets
    packet_t *packetTail; //last packet in the list of packets
	void (*control)(packet_t *p,void *any); //see the implementation for why this is needed?
    //is void the correct return type for this function
	unsigned long long cId; //ID of the connnection
    //properties should also be one of the field, not sure what should come here
	short proto;
	int l7proto; //this is l7 protocol like http
	int app; //this is the application gmail,yahoo-mail etc....
	int trainLen;
	int nConcurent;
	int nPackets; //number of packets
	packet_t *current;
	int cycles;
} conn_t;


//Todo: This needs some improvement and correctness testing
struct conncomp {
	bool operator()(const connid_t *c1,const connid_t *c2) const {
		if (c1->sAddr.ip == c2->sAddr.ip) {
			if (c1->dAddr.ip == c2->dAddr.ip) {
				if (c1->l4Proto == c2->l4Proto) {
					return false;
				} else if (c1->l4Proto < c2->l4Proto) {
					return true;
				}
			} else if (c1->dAddr.ip < c2->dAddr.ip) {
				return true;
			}
		} else if (c1->sAddr.ip < c2->sAddr.ip) {
			return true;
		}
		return false;
	}
};

class connections {
public:
	connections() {
	};
	bool addPacket(connid_t *cid,packet_t *pack,int);
	void printConnections();
	map<connid_t *,conn_t *,conncomp> m_conns; //may be we can make them tcp/udp connections
    conn_t * findconn(connid_t *cid);
	void removeConn(conn_t *conn);
	bool insertPacket(connid_t *cid,packet_t *p);
};

#endif
