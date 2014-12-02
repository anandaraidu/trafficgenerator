#ifndef ADD_H
#define ADD_H
//#include "p.h"

typedef union address {
	int ip;
	struct v4 {
		unsigned int ip;
	};
	struct v6 {
		unsigned long long f; //first part
		unsigned long long s; //first part
	};
	short type; //short or long doesn't save any thing unless you are making arrays of these
	//ipv6:: todo
} address_t;
#endif //corresponding to the header guard
