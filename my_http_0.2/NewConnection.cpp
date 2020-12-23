#include "NewConnection.h"
#include "NewRequest.h"



NewConnection::NewConnection(const std::string &connName,int sockfd):
	name_(connName),
	request_(new NewRequest(sockfd))
	{}

NewConnection::~NewConnection(){}


