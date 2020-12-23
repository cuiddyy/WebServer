#ifndef NEWCONNECTION__H__
#define NEWCONNECTION__H__

#include <memory>
#include "Buffer.h"
#include "NewRequest.h"


class NewConnection : std::enable_shared_from_this<NewConnection>{
public:
	NewConnection(const std::string &connName,int sockfd);
	~NewConnection();
	

private:
	std::unique_ptr<NewRequest>request_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;
	const std::string name_;	
};

using NewConnectionPtr = std::shared_ptr<NewConnection>;


#endif
