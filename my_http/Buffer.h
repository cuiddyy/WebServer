#ifndef BUFFER__H__
#define BUFFER__H__

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

#include <cassert>



class Buffer {
public:
	Buffer():buffer_(initSize_),readerIndex_(0),writerIndex_(0){
		assert(readableBytes() == 0);
		assert(writableBytes() == initSize_);
	}
	~Buffer(){}
	//可读取的缓冲区大小
	size_t readableBytes() const {return writerIndex_ - readerIndex_;}
	//可写的缓冲区大小
	size_t writableBytes() const {return buffer_.size() - writerIndex_;}
	//存储区域前面空余空间的大小
	size_t prependableBytes() const {return readerIndex_;}
	//存储区域的开头指针
	const char* peek() const {return __begin()+readerIndex_;}
	//从套接字向缓冲区中读入数据
	ssize_t readFd(int fd,int* savedErrno);
	//从缓冲区向套接字中输出数据
	ssize_t writeFd(int fd,int* savedErrno);
	void retrieve(size_t len){
		//取出len个字节
		assert(len <= readableBytes());
		readerIndex_ += len;
	}
	//取出直到指定位置的数据
	void retrieveUntil(const char* end){
		assert(peek() <= end);
		assert(end <= beginWrite());
		retrieve(end-peek());
	}
	//取出缓冲区的所有数据
	void retrieveAll(){
		readerIndex_ = 0;
		writerIndex_ = 0;
	}
	//以string形式返回缓冲区内的内容
	std::string retrieveAsString(){
		std::string str(peek(),readableBytes());
		retrieveAll();
		return str;
	}
	//确保有足够的空间
	void ensureWriteableBytes(size_t len){
		if(writableBytes() < len){
			__makeSpace(len);
		}
		assert(writableBytes() >= len);
	}
	//可以开始写的位置
	char* beginWrite(){return __begin() + writerIndex_;}
	const char* beginWrite() const {return __begin() + writerIndex_;}
	//写入数据之后更新指针的位置
	void hasWriten(size_t len){writerIndex_ += len;}

	//插入数据
	void append(const void* data,size_t len){
		append(static_cast<const char*>(data),len);
	}
	//最终都要这个版本的append完成输入操作.
	void append(const char* data,size_t len){
		ensureWriteableBytes(len);
		std::copy(data,data+len,beginWrite());
		hasWriten(len);
	}
	void append(const std::string& str){
		append(str.data(),str.length());
	}
	//把其他Buffer中的数据添加到本缓冲区内
	void append(const Buffer& anothBuff){
		append(anothBuff.peek(),anothBuff.readableBytes());
	}
	//在整个缓冲区内查找\r\n
	const char* findCRLF() const {
		const char CRLF[] = "\r\n";
		const char* crlf = std::search(peek(),beginWrite(),CRLF,CRLF+2);
		return crlf == beginWrite() ? nullptr : crlf;
	}
	//在指定开始位置到beginWrite()开始查找\r\n
	const char* findCRLF(const char* start) const {
		assert(peek() <= start);
		assert(start <= beginWrite());
		const char *CRLF = "\r\n";
		const char* crlf = std::search(start,beginWrite(),CRLF,CRLF+2);
		return crlf == beginWrite()? nullptr:crlf;
	}



private:
	//返回缓冲区头部指针
	char* __begin(){return &*buffer_ .begin();}
	const char* __begin() const {return &*buffer_.begin();}
	//确保缓冲区有足够的空间
	void __makeSpace(size_t len){
		if(writableBytes()+prependableBytes() < len){
			buffer_.resize(writerIndex_+len);
		}else{
			size_t readable = readableBytes();
			std::copy(__begin()+readerIndex_,__begin()+writerIndex_,__begin());
			readerIndex_ = 0;
			writerIndex_ = readerIndex_ + readable;
			assert(readable == readableBytes());
		}
	}
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;
	static const int initSize_ = 1024;
};



#endif
