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
#include "pcapprocess.h"
int running = 1;
using namespace std;

int main(int argc, char *argv[])
{
	pcap_t *pcap;
	const unsigned char *packet;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr header;
	connections m_connections;

	/* Skip over the program name. */
	++argv; --argc;

	/* We expect exactly one argument, the name of the file to dump. */
	if ( argc != 2 )
	{
		fprintf(stderr, "program requires 2 arguments, ./a.out <NumConcurrent> <PcapFile>\n");
		exit(1);
	}
	int concurent = atoi(argv[0]);

	printf("PCAP file name is concurent[%d] [%s] \n",concurent,argv[1]);
	pcap = pcap_open_offline(argv[1], errbuf);
	if (pcap == NULL)
	{
		fprintf(stderr, "error reading pcap file: %s\n", errbuf);
		exit(1);
	}

	/* Now just loop through extracting packets as long as we have
	 * some to read.
	 */
	while ((packet = pcap_next(pcap, &header)) != NULL) {
		char *pack = new char[header.caplen];

	    memcpy(pack,packet,header.caplen); //copy the whole packet, so that it can be manipulated as you like
		//process_packet(m_connections,packet, header.ts, header.caplen,concurent);
	    connid_t *cid = new connid_t;
	    memset(cid,0,sizeof(connid_t));
		packet_t *p = process_packet(m_connections,pack, header.ts, header.caplen,concurent,cid);
		if (p != NULL) {
			m_connections.addPacket(cid,p,concurent);
		} else {
			delete cid;
		}
	}

	//todo: At this point we should close the pcap file so that all packets
    //are freed up and all the fds are closed

	cout  << "All done " << endl;
	configureConnections(m_connections);
	
	//Todo: start generating the traffic now
	//m_connections.printConnections();

	return 0;
}
