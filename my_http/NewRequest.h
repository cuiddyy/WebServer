#ifndef NEWREQUEST__H__
#define NEWREQUEST__H__

#include "Buffer.h"
//#include "Timer.h"
#include <string>
#include <map>
#include <iostream>
#include <memory> //shared_ptr

#define STATIC_ROOT "../www"

class Timer;
using TimerPtr = std::shared_ptr<Timer>;

class NewRequest {
public:
	NewRequest(int fd);
	~NewRequest();
	int fd() const {return fd_;}
	int read(int* savedErrno);
	int write(int* savedErrno);
	void appendToOutBuffer(const Buffer& buf){outBuffer_.append(buf);}	
	int writableBytes() const {return outBuffer_.readableBytes();}
	void setTimer(TimerPtr timer) {wtptr_ = timer;}
	std::weak_ptr<Timer> getTimer() const {return wtptr_;}
	void setWorking(){working_ = true;}
	void setNoWorking(){working_ = false;}
	bool isWorking() const {return working_;}
	
	bool parseRequest();//解析请求报文
	bool parseFinish(){return state_ == GotAll;} //是否解析完一个报文
	void resetParse(); //重置解析状态
	std::string getPath() const {return path_;}
	std::string getQuery() const {return query_;}
	std::string getHeader(const std::string& field)const;
	std::string getMethod()const;
	bool keepAlive() const; //是否为长连接

	enum NewRequestParseState{//报文解析状态
		ExceptRequestLine,
		ExpectHeaders,
		ExpectBody,
		GotAll
	};

	enum Method{//HTTP方法
		Invalid,Get,Post,Head,Put,Delete
	};

	enum Version {//HTTP版本
		Unknown,HTTP10,HTTP11
	};


private:
	//解析请求行
	bool parseRequestLine(const char* begin,const char* end);
	//设置HTTP方法
	bool setMethod(const char* begin,const char* end);

	//设置URL路径
	void setPath(const char* begin,const char* end){
		std::string addPath;
		addPath.assign(begin,end);
		if(addPath == "/")
			addPath = "index.html";
		path_ = STATIC_ROOT + addPath;
	}
	
	//设置URL参数
	void setQuery(const char* begin,const char* end){
		query_.assign(begin,end);
	}

	//设置HTTP版本
	void setVersion(Version version){version_ = version;}

	//增加报文头
	void addHeader(const char* start,const char* colon,const char* end);

	int fd_;  //该连接的文件描述符
	Buffer inBuffer_; //读缓冲区
	Buffer outBuffer_; //写缓冲区
	bool working_; //若正在工作,就不能将其移除.

	std::weak_ptr<Timer> wtptr_; //该连接的时间计时器
	
	//报文解析相关变量
	NewRequestParseState state_;  //报文解析状态
	Method method_; //HTTP方法
	Version version_; //HTTP版本
	std::string path_; //URL路径
	std::string query_; //URL参数
	std::map<std::string,std::string> headers_; //报文头部

};

using  NewRequestPtr = std::shared_ptr<NewRequest>;

#endif
