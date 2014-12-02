#ifndef PCAP_PROCESS_H
#define PCAP_PROCESS_H
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

const char *timestamp_string(struct timeval ts);
void problem_pkt(struct timeval ts, const char *reason);
void too_short(struct timeval ts, const char *truncated_hdr);
//void process_packet(connections &conns,unsigned const char *packPtr, struct timeval ts,
packet_t *process_packet(connections &conns,char *packPtr, struct timeval ts,
			unsigned int caplen,int nconcurent,connid *);
#endif
