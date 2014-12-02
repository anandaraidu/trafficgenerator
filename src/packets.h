#ifndef PACKETS_H
#define PACKETS_H

#define CLIENT_TO_SERVER 0
#define SERVER_TO_CLIENT 1

typedef struct anyprocess { //linked list of function pointers
	struct anyprocess *next;
	int (*process)(char *basep,void *any,void *pkt);//function pointer
} anyprocess_t;

//self contained structure for now
typedef struct t_packet {
	char *m_pkt; //actual packet
	int packetLen;
	int m_l3off;
	int m_l4off;
	int m_l7off;
	
	int pNum;  //number of this packet in this connection
	int connId; //This can keep incrementing
	struct t_packet *next; //next packet link
    struct t_packet *prev; //previous packet link
    short c2s; //in which direction is this packet going
    //todo: do we need a parent pointer back to the connection;any maniplulation required here
	int nTimesPerIteration; //send this packet n times per iteration
	//conn_t *conn;//back reference to connection
	anyprocess_t *control; //controls this packet
} packet_t;
#endif //for header guards
