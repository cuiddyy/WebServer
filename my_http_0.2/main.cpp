#include <iostream>

#include "NetServer.h"
#include "ComAddress.h"
//#include "NewRequest.h"
//#include "Epoll.h"
//#include "NewResponse.h"
using namespace std;


int main(int argc,char** argv){
//	ThreadPool pool(3);
	uint16_t port = 2020;

	if(argc >= 2){
		port = atoi(argv[1]);
	}
	int numThreads = 3;
	if(argc >= 3){
		numThreads = atoi(argv[2]);
	}
	string serverIp = "127.0.0.1";
	ComAddress serverAddr(serverIp,port);
	NetServer server(serverAddr,numThreads);
	server.start();

	return 0;
}
