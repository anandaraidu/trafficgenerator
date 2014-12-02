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
#include <memory.h>
#include "conn.h"
#include "control.h"
using namespace std;


/**
 *
 * This is the main file. ie which contains the main function
 *
 */

/* Returns a string representation of a timestamp. */
//const char *timestamp_string(struct timeval ts);

/* Report a problem with dumping the packet with the given timestamp. */
//void problem_pkt(struct timeval ts, const char *reason);

/* Report the specific problem of a packet being too short. */
//void too_short(struct timeval ts, const char *truncated_hdr);


/* Note, this routine returns a pointer into a static buffer, and
 * so each call overwrites the value returned by the previous call.
 */
const char *timestamp_string(struct timeval ts)
{
	static char timestamp_string_buf[256];

	sprintf(timestamp_string_buf, "%d.%06d",
		(int) ts.tv_sec, (int) ts.tv_usec);

	return timestamp_string_buf;
}

void problem_pkt(struct timeval ts, const char *reason)
{
	fprintf(stderr, "%s: %s\n", timestamp_string(ts), reason);
}

void too_short(struct timeval ts, const char *truncated_hdr)
{
	fprintf(stderr, "packet with timestamp %s is truncated and lacks a full %s\n",
	timestamp_string(ts), truncated_hdr);
}

//void process_packet(connections &conns,unsigned const char *packPtr, struct timeval ts,
packet_t *process_packet(connections &conns,char *pack, struct timeval ts,
			unsigned int caplen,int nconcurent,connid *cid)
{
	int olen = caplen;
	//char *pack = new char[caplen]; //beware we may be mixing malloc and new.please change

	//memcpy(pack,packPtr,caplen); //copy the whole packet, till now it const as returned by
                                //pcap library, so you own and copy this so that you can
                              //completely modify as you like


	//cout << "PacketLen: " << caplen <<  endl;
	char *optr = pack; //original packet pointer
	struct ip *ip;
	struct UDP_hdr *udp = NULL;
	struct TCP_hdr *tcp = NULL;
	unsigned int IP_header_length;

	/* For simplicity, we assume Ethernet encapsulation. */

	if (caplen < sizeof(struct ether_header)) {
		/* We didn't even capture a full Ethernet header, so we
		 * can't analyze this any further.
		 */
		too_short(ts, "Ethernet header");
		return NULL;
	}

	/* Skip over the Ethernet header. */
	pack += sizeof(struct ether_header);
	caplen -= sizeof(struct ether_header);

	if (caplen < sizeof(struct ip)) { /* Didn't capture a full IP header */
		too_short(ts, "IP header");
		return NULL;
	}

	ip = (struct ip*) pack;
	
	//Print the address
	//cout << "SrcIp[" << ntohl(ip->ip_src.s_addr) << "]\n";
	//cout << "DstIp[" << inet_ntoa(ip->ip_dst) << "]\n";
	//cout << "SrcIp[" << inet_ntoa(ip->ip_src) << "]\n";
	//printf("srcaddr[%p] dstaddr[%p]\n",&ip->ip_src,&ip->ip_dst);
	IP_header_length = ip->ip_hl * 4;	/* ip_hl is in 4-byte words */
    pack += IP_header_length;

	if (caplen < IP_header_length) {
		too_short(ts, "IP header with options");
		return NULL;
	}

	packet_t *p = new packet_t;
    //todo: we need to create a packet with the new data structure
	//connid_t *cid = new connid_t;
	//memset(cid,0,sizeof(connid_t));

    bool lowtohigh = false;
	//Well this works only for IPV4, we should make the similar for IPV6
	cid->sAddr.ip = ntohl(ip->ip_src.s_addr);
	cid->dAddr.ip = ntohl(ip->ip_dst.s_addr);

	//cout << "BEFORE\n";
	//cid->printconn();
	if (cid->sAddr.ip > cid->dAddr.ip) {
		//no action here
	} else { //in this case swap the address
		int temp = cid->sAddr.ip;

		cid->sAddr.ip = cid->dAddr.ip;
		cid->dAddr.ip = temp;
	}
	//cout << "AFTER\n";
	//cid->printconn();
	

	//printf("addr= %s\n",inet_ntoa(ip->ip_src));
	//set the l3-offset and l4-offset
	p->m_pkt = optr; //set the pointer
	p->packetLen = olen;
    p->m_l3off = sizeof(struct ether_header);
    p->m_l4off = sizeof(struct ether_header) + IP_header_length;
    //p->m_l4off = (optr-pack);
	
	//p->l7off =  todo set the layer-7 offset
	//p->setL3off( sizeof(struct ether_header));
	//p->setL4off( sizeof(struct ether_header) + IP_header_length);
	//p->setL7off();
	//cout  << "L4 offset: " << p->m_l4off << endl;

	if (ip->ip_p == IPPROTO_UDP) {
		udp = (struct UDP_hdr*) pack;
		cid->sPort = ntohs(udp->uh_sport);
		cid->dPort = ntohs(udp->uh_dport);
		if (cid->sAddr.ip > cid->dAddr.ip) {
		} else {
			short temp = cid->sPort;
			cid->sPort = cid->dPort;
			cid->dPort = temp;
		}
		cid->l4Proto = IPPROTO_UDP;
	} else if (ip->ip_p == IPPROTO_TCP) {
		tcp = (struct TCP_hdr*) pack;
		//cout << "TCP Pointer1 is " << tcp << endl;
		cid->sPort = ntohs(tcp->th_sport);
		cid->dPort = ntohs(tcp->th_dport);
		cid->l4Proto = IPPROTO_TCP;
		if (tcp->th_flags & TH_SYN) {
			cout << "SYNFLAG seen \n";
		}
		if (tcp->th_flags & TH_FIN) {
			cout << "FINFLAG1 seen \n";
		}
		tcp = (struct TCP_hdr*) ((char*)p->m_pkt +  p->m_l4off);
		//cout << "TCP Pointer2 is " << tcp << endl;
		
		if (cid->sAddr.ip > cid->dAddr.ip) {
		} else {
			short temp = cid->sPort;
			cid->sPort = cid->dPort;
			cid->dPort = temp;
		}
	} else {	
		cout << "Unknown L4 protocol" << endl;
		return NULL;
	}
	cid->printconn();

	//conns.addPacket(cid,p,nconcurent);
	return p;
}
