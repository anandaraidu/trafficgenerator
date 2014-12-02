clean:
	rm a.out
all:
	g++ main.cpp conn.cpp control.cpp pcapprocess.cpp server.cpp -lpcap -g -opacketwriter
serv:
	g++ servermain.cpp conn.cpp control.cpp pcapprocess.cpp -lpcap -g -opacketreader
