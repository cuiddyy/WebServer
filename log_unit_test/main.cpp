#include "Logger.h"



int main(){
	LOG_TRACE << "111" ;
	LOG_DEBUG << "222" ;
	LOG_INFO << "333" ;
	LOG_WARN << "444" ;
	LOG_ERROR << "555" ;
	LOG_FATAL << "666" ;
	//LOG_SYSERR << "777" ;
	LOG_SYSFATAL << "888" ;
	return 0;
}
