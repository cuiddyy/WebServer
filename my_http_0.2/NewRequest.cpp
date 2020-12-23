#include "NewRequest.h"


#include <iostream>
#include <cassert>

#include <unistd.h>


NewRequest::NewRequest(int fd):fd_(fd),
	working_(false),
	state_(ExceptRequestLine),
	method_(Invalid),
	version_(Unknown){
	
	assert(fd_ >= 0);

}

NewRequest::~NewRequest(){
	close(fd_);
}

int NewRequest::read(int* savedErrno){
	printf("Newquest_read_fd=%d\n",fd_);
	int ret = inBuffer_.readFd(fd_,savedErrno);
	return ret;
}

int NewRequest::write(int* savedErrno){
	int ret = outBuffer_.writeFd(fd_,savedErrno);
	return ret;
}

std::string NewRequest::getHeader(const std::string& field)const{
	std::string ret;
	auto it = headers_.find(field); 
	if(it != headers_.end()){
		ret = it->second;
	}
	return  ret; 
}

std::string NewRequest::getMethod()const{
	if(method_ == Get) return "GET";
	else if(method_ == Post) return "POST";
	else if(method_ == Head) return "HEAD";
	else if(method_ == Put) return "PUT";
	else if(method_ == Delete) return "DELETE";
	return "";
}

bool NewRequest::setMethod(const char* start,const char* end){
	std::string ret(start,end);
	if(ret == "GET")
		method_ = Get;
	else if(ret == "POST")
		method_ = Post;
	else if(ret == "HEAD")
		method_ = Head;
	else if(ret == "PUT")
		method_ = Put;
	else if(ret == "DELETE")
		method_ = Delete;
	else
		method_ = Invalid;
	return method_ != Invalid;
}


void NewRequest::addHeader(const char* start,const char* separ,const char* end){
	std::string field(start,separ);
	separ++;
	while(separ < end && *separ==' ')
			separ++;
	std::string val(separ,end);
	while(!val.empty() && val[val.size()-1] == ' ')
		val.resize(val.size()-1);
	
	headers_[field] = val;
}

bool NewRequest::parseRequestLine(const char* begin,const char* end){
	bool succeed = false;
	const char* start = begin;
	const char* separ = std::find(start,end,' ');
	if(separ != end && setMethod(start,separ)){
		start = separ+1;
		separ = std::find(start,end,' ');
		if(separ != end){
			const char* requ = std::find(start,separ,'?');
			if(requ != separ){
				setPath(start,requ);
				setQuery(requ,separ);
			}else{
				setPath(start,separ);
			}
			start = separ + 1;
			succeed = end - start == 8 && std::equal(start,end-1,"HTTP/1.");
			if(succeed){
				if(*(end - 1) == '1')
					setVersion(HTTP11);
				else if(*(end-1) == '0')
					setVersion(HTTP10);
				else
					succeed = false;
			}
		}
	}
	return succeed;
}
//处理请求信息
bool NewRequest::parseRequest(){
	bool succeed = true;
	bool hasMore = true;

	while(hasMore){
		if(state_ == ExceptRequestLine){
			//处理请求行
			const char* crlf = inBuffer_.findCRLF();
			if(crlf){
				succeed = parseRequestLine(inBuffer_.peek(),crlf);
				if(succeed){
					inBuffer_.retrieveUntil(crlf+2);
					state_ = ExpectHeaders;
				}else{
					hasMore = false;
				}
			}else{
				hasMore = false;
			}
		}else if(state_ == ExpectHeaders){
			//处理报文头
			const char* crlf = inBuffer_.findCRLF();
			if(crlf){
				const char* separ = std::find(inBuffer_.peek(),crlf,':');
				if(separ!=crlf){
					addHeader(inBuffer_.peek(),separ,crlf);
				}else{
					state_ = GotAll;
					hasMore = false;
				}

				inBuffer_.retrieveUntil(crlf+2);
			}else{
				hasMore = false;
			}
		}else if(state_ == ExpectBody){
			//处理报文实体.
		}
	}

	return succeed;
}

bool NewRequest::keepAlive() const {
	std::string conn = getHeader("Connection");
	bool ret = (conn == "Keep-Alive" || (version_ == HTTP11 && conn != "close"));
	
	return ret;
}

void NewRequest::resetParse(){
	state_ = ExceptRequestLine; //报文解析状态
	method_ = Invalid; //HTTP方法
	version_ = Unknown; //HTTP版本
	path_ = ""; //URL路径
	query_ = "";//URL路径
	headers_.clear();
}
