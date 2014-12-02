#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include "p.h"
#include "conn.h"
#include "pcapprocess.h"

#include <iostream>

using namespace std;

char *readpayload(int connfd,int lentoread) {

	char *ptr = new char[lentoread];
	char *optr = ptr;
	int olen = lentoread;
    //cout  << "LEN TO read: " << lentoread << endl;
 	while(1) { //add error handling later, currently no error handling for now
		int len = read(connfd,ptr,lentoread); //just reads the size
		//cout  << "Reading for length: " << len << endl;
		if (len == lentoread) {
#if 0
			cout << "LENGTH is: " << olen << endl;
			for (int i=0;i < olen;i++) {
				cout << ptr[i] << endl;
			}
#endif
			return optr; //read is complete, got what we asked for
		}
		ptr += len;
		lentoread -= len;
	}
}

int readlen(int connfd) {
	int lenlen = 0;
	int toread = sizeof(int);
	char *ptr = (char *)&lenlen;
	char *optr = ptr;
	while (1) {
		int len = read(connfd,ptr,toread); //just reads the size
		if (len == toread) {
			//delete optr;
			return ntohl(lenlen);
		}
		ptr += len;
		toread -= len;
	}
	printf("Somerror happened while reading length of the packet\n");
	exit(0); //just exit
	return 0;//just keep the comilation going
}

void start(connections &conns) {
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

	//For now accept only 1 connection
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
   	while(1) {
		unsigned int l = readlen(connfd);
		//printf("Reading len [%d]\n",l);
		char *p = readpayload(connfd,l);
		struct timeval ts;
	    connid_t *cid = new connid_t;
	    memset(cid,0,sizeof(connid_t));
		//when reading we dont care for timestamp, caplen is nothing but the packet length,concurent=0
		packet_t *pkt = process_packet(conns,p,ts,l,0,cid); //when reading we dont care for timestamp, caplen is nothing but the packet length,concurent=0
		//printf("%s\n",p);
		//delete p;
		//we got the packet from the other end
	}
}
