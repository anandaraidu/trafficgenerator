#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

#include "p.h"
#include <pcap.h>
#include <map>
#include<list>
#include<iostream>
#include "conn.h"
using namespace std;

int bumpClientIp(char *p,void *cntrl,void *paket) {
	packet_t *pkt = (packet_t *)paket;
	struct ip *iph = (struct ip*) (p+ pkt->m_l3off);
	iph->ip_src.s_addr++; //simple
	return 0;
}

void appendPacket(conn_t *conn,packet_t *pack) {
	//printf("Appending packet \n");
	if (conn->packetTail == NULL) { //this is not the common case, let us flip the if,will it make any difference?
		//printf("Adding first packet \n");
	
		conn->packetTail = pack; //Init the pointers for the first packets in the connection
		conn->packetHead = pack; 

		pack->prev = NULL; //meaning this is the first packet for this connection
		pack->next = NULL; //at this point there are no more packets for this connection
	} else { //keep appending to the end
		//printf("Adding next few packets \n");
		conn->packetTail->next  = pack;
		pack->prev = conn->packetTail;
		pack->next = NULL; //we can make this circular any-time
		conn->packetTail = pack; 
	}
};

conn_t *connections::findconn(connid_t *cid) {
	map<connid_t *,conn_t *,conncomp>::iterator it = m_conns.find(cid);
	if (it == m_conns.end()) {
		return NULL;
	} 
	return it->second;
}

void connections::removeConn(conn_t *conn) { //if it is not there just ignore no problem for now
	map<connid_t *,conn_t *,conncomp>::iterator it = m_conns.find(conn->connid);
	if (conn != it->second) {
		cout << "ASSERT error in the map-fatal exiting \n";
		exit(0);
	} 

	delete it->second;
	connid_t *cid = conn->connid;
	m_conns.erase(it); //just erase it from the 
	//delete cid; //all memory cleaned now
}

bool isFin(packet_t *p) { 
	//cout  << "L4 offset: " << p->m_l4off << endl;
	struct TCP_hdr *tcp = (struct TCP_hdr*) ((char *)p->m_pkt + p->m_l4off);
	if (tcp->th_flags & TH_FIN) {
		return true;
	}
	return false;
}

//test stub -- dont go with the name insert, it inserts and also deletes the packet and all
bool connections::insertPacket(connid_t *cid,packet_t *p) {
	conn_t *conn = findconn(cid);
	if (isFin(p) == true) {
		cout  << "FINFOUND \n";
	}
	if (conn == NULL) {
		conn = new conn_t;
		conn->nPackets = 1;
		conn->connid = cid;
		m_conns.insert(make_pair(cid,conn));
		cout  << "NEW connection created\n";
		cid->printconn();
		cout << "Number of connections[" << m_conns.size() << "]" << endl;
		return true;
	} else if (isFin(p)) {
		++conn->nPackets;
		removeConn(conn);
		cout  << "Connection deleted\n";
		cout << "Number of connections[" << m_conns.size() << "]" << endl;
		delete cid;
		return true;
	} else {
		cout  << "Normal Packet\n";
		cout << "Number of connections[" << m_conns.size() << "]" << endl;
		++conn->nPackets;
	}
	//cout << "NumPackets: " << conn->nPackets << endl;
	delete p->m_pkt;
	delete p;
	delete cid;
	return true;
}

bool connections::addPacket(connid_t *cid,packet_t *p,int concurent) {
	//map<connid_t *,conn_t *,conncomp>::iterator it = m_conns.find(cid);
	conn_t *conn = findconn(cid);
	anyprocess_t *cntrl = new anyprocess_t;
	cntrl->process = bumpClientIp;
	cntrl->next = NULL; //we should move this away from this todo
	p->control = cntrl;
	//if (it == m_conns.end()) {
	if (conn == NULL) { //this is not the regular branch
		//printf("New connection:::[%d] [%d] [%d] [%d] [%d]\n",cid->sAddr.ip,cid->dAddr.ip,cid->sPort,cid->dPort,cid->l4Proto);
		//conn = new connection(cid,cid->l4Proto);
		conn = new conn_t;
		memset(conn,0,sizeof(conn_t));
		conn->current = p; //This is the packet currently getting played,since this is is the first packet to play the game
		conn->nPackets = 0;
		conn->connid = cid;
		conn->packetTail = conn->packetHead = NULL;
		conn->proto = cid->l4Proto;
		conn->control = NULL; //todo: important this is the way we can control each packet fields within the connection
		conn->nConcurent =  concurent; //todo: not sure how we should plan this numconcurrent per connection
		//conn->trainLen = planTheRun();
		//todo: populate l7 and application protocol
		m_conns.insert(make_pair(cid,conn));
	} else { //append to old connection
		//printf("Ezxisting connection:::[%d] [%d] [%d] [%d] [%d]\n",cid->sAddr.ip,cid->dAddr.ip,cid->sPort,cid->dPort,cid->l4Proto);
		delete cid;
		//conn = it->second;
		//cout << "Existing connection " << endl;
	}
	++conn->nPackets; //bump the number of packets
	printf("Num connections [%lu] NunPackets[%d]\n",m_conns.size(),conn->nPackets);
	appendPacket(conn,p);
}

void connections::printConnections() {
	map<connid_t *,conn_t *,conncomp>::iterator it = m_conns.begin();
	for (; it != m_conns.end();  it++) {
		connid_t *c = it->first;
		//cout << "New connection ";
		//printf("[%d] [%d] [%d] [%d] [%d]\n",c->sAddr.ip,c->dAddr.ip,c->sPort,c->dPort,c->l4Proto);
	}
}
