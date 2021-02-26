#ifndef NEWRESPONSE__H__
#define NEWRESPONSE__H__

#include <map>
#include <string>

#define CONNECT_TIMEOUT 500 //单个连接持续时长为500ms

class Buffer;

class NewResponse{

public:
	static const std::map<int,std::string> statCode_message_map;
	static const std::map<std::string,std::string> suffix_type_map;
	
	NewResponse(int statusCode,std::string source,bool keepAlive)
		:statusCode_(statusCode),source_(source),keepAlive_(keepAlive)
		{}
	~NewResponse() {}
	
	Buffer makeResponse();
	void doErrorResponse(Buffer& output,std::string message);    //构造错误报文
	void doStaticRequest(Buffer& output,long fileSize);          //正常响应处理


private:
	std::string getFileType();
	
	std::map<std::string,std::string> headers_;   //保存的报文头部	
	int statusCode_;   //响应状态码
	std::string source_;  //请求的资源路径
	bool keepAlive_;   //长连接标志

};






#endif
