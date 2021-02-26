#include "NewResponse.h"
#include "Buffer.h"

#include <string>
#include <iostream>
#include <cassert>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>	  //open
#include <unistd.h>   //close


const std::map<int,std::string> NewResponse::statCode_message_map = {
	{200,"OK"},
	{400,"Bad Request"},
	{403,"Forbidden"},
	{404,"Not Found"}

};

const std::map<std::string,std::string> NewResponse::suffix_type_map = {
	{".html", "text/html"},
	{".xml", "text/xml"},
	{".xhtml", "application/xhtml+xml"},
	{".txt", "text/plain"},
	{".rtf", "application/rtf"},
	{".pdf", "application/pdf"},
	{".word", "application/nsword"},
	{".png", "image/png"},
	{".gif", "image/gif"},
	{".jpg", "image/jpeg"},
	{".au", "audio/basic"},
	{".mpeg", "video/mpeg"},
	{".mpg", "video/mpeg"},
	{".avi", "video/x-msvideo"},
	{".gz", "application/x-gzip"},
	{".tar", "application/x-tar"},
	{".css", "text/css"}
};

std::string NewResponse::getFileType(){
	int index = source_.find_last_of('.');
	if(index== std::string::npos){
		return "text/plain";
	}
	std::string suffix = source_.substr(index);
	auto ret = suffix_type_map.find(suffix);
	if(ret == suffix_type_map.end()){
		return "text/plain";
	}

	return ret -> second;
}

void NewResponse::doErrorResponse(Buffer& outputs,std::string message){
	auto it = statCode_message_map.find(statusCode_);
	if(it == statCode_message_map.end()){
		return;
	}
	std::string body;
	body += "<html><title>Durings Error</title>";
	body += "<body bgcolor=\"ffffff\">";
	body += std::to_string(statusCode_) + " : " + it -> second + "\n";
	body += "<p>" + message + "</p>";
	body += "<hr><em>Durings web server</em></body></html>";
	//响应行
	outputs.append("HTTP/1.1" + std::to_string(statusCode_) + " " + it->second + "\r\n");
	//报文头
	outputs.append("Server:Durings\r\n");
	outputs.append("Content-type:text/html\r\n");
	outputs.append("Connection:close\r\n");
	outputs.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");

	//报文体
	outputs.append(body);
}

//处理静态文件请求
void NewResponse::doStaticRequest(Buffer& outputs,long fileSize){
	assert(fileSize >=0 );

	auto it = statCode_message_map.find(statusCode_);
	if(it == statCode_message_map.end()){
		statusCode_ = 400;
		doErrorResponse(outputs,"Unknown status code");
		return;
	}

	//响应行
	outputs.append("HTTP/1.1 " + std::to_string(statusCode_) + " " + it->second + "\r\n");
	//报文头
	if(keepAlive_){
		outputs.append("Connection: Keep-Alive\r\n");
		outputs.append("Keep-Alive: timeout=" + std::to_string(CONNECT_TIMEOUT) + "\r\n");
	}else{
		outputs.append("Connection: close\r\n");
	}

	outputs.append("Content-type: " + getFileType() + "\r\n");
	outputs.append("Content-length: " + std::to_string(fileSize) + "\r\n");

	outputs.append("Server:Durings\r\n");
	outputs.append("\r\n");

	//报文体
	int sourceFd = ::open(source_.data(),O_RDONLY,0);
    //内存映射IO
    void* mmapRet = ::mmap(NULL,fileSize,PROT_READ,MAP_PRIVATE,sourceFd,0);
	::close(sourceFd);
	if(mmapRet == (void*)-1){
		munmap(mmapRet,fileSize);
		outputs.retrieveAll();
		statusCode_ = 404;
		doErrorResponse(outputs,"Durings can't find the file");
		return;
	}	

	char *sourceAddr = static_cast<char*>(mmapRet);
	outputs.append(sourceAddr,fileSize);
	munmap(sourceAddr,fileSize);

}

Buffer NewResponse::makeResponse(){
	Buffer sendBuff;

	if(statusCode_ == 400){
		doErrorResponse(sendBuff,"Server can't parse the message");
		return sendBuff;
	}

	struct stat sbuf;
	//服务器找不到请求的文件
	if(::stat(source_.data(),&sbuf) < 0){
		statusCode_ = 404;
		doErrorResponse(sendBuff,"Server cant't find the file");
		return sendBuff;
	}

	//没有权限(不是常规文件或者用户没有读的权限)
	if((!S_ISREG(sbuf.st_mode))|(!(S_IRUSR & sbuf.st_mode))){
		statusCode_ = 403;
		doErrorResponse(sendBuff,"Server cant't read the file");
		return sendBuff;
	}
	

	//开始处理静态文件请求
	doStaticRequest(sendBuff,sbuf.st_size);
	return sendBuff;


}
