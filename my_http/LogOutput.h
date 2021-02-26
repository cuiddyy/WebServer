#ifndef LOGOUTPUT__H__
#define LOGOUTPUT__H__

#include <assert.h>
#include <string.h>

#include "Types.h"
#include "StringPiece.h"

using std::string;

const int smallBuffSize = 4000;
const int largeBuffSize = 4000*1000;

template <int SIZE>
class LogBuffer{
public:
	LogBuffer():cur_(data_)
		{
			setCookie(cookieStart);
		}

	~LogBuffer()
		{
			setCookie(cookieEnd);
		}

	void append(const char* buf,size_t len){
		if (implicit_cast<size_t>(avail()) > len){
			memcpy(cur_,buf,len);
			cur_ += len;
		}
	}
	
	const char* data() const {return data_;}
	int length() const {return static_cast<int>(cur_ - data_);}
	void extend(size_t len){cur_ += len;}
	int avail() const {return static_cast<int>(end() - cur_);}
	char* current() {return cur_;}
	void reset() {cur_ = data_;}
	void bzero() {memZero(data_,sizeof data_);}

	// for GDB
	const char* debugString();
	void setCookie(void (*cookie)()) { cookie_ = cookie; }

	//for unit test
	string toString() const { return string(data_,length());}
	StringPiece toStringPiece() const {return StringPiece(data_,length());}

private:
	const char* end() const { return data_ + sizeof(data_); }

	static void cookieStart();
	static void cookieEnd();
	void (*cookie_)();
	char data_[SIZE];//注意这个SIZE是通过模板参数传进来的,这种方法可以参考.
	char* cur_;
};

class LogOutput{

	using self = LogOutput;
public:
	using Buffer = LogBuffer<smallBuffSize>;
	self& operator<<(bool b){
		buffer_.append(b?"1":"0",1);
		return *this;
	}

	self& operator<<(char c){
		buffer_.append(&c,1);
		return *this;
	}

	self& operator<<(float v){
		*this << static_cast<double>(v);
		return *this;
	}
	
	self& operator<<(const char* str){
		if(str){
			buffer_.append(str,strlen(str));
		}else{
			buffer_.append("[null]",6);
		}
		return *this;
	}

	self& operator<<(const unsigned char* str){
		return operator<<(reinterpret_cast<const char*>(str));
	}

	self& operator<<(const string& v){
		buffer_.append(v.c_str(),v.size());
		return *this;
	}

	self& operator<<(const StringPiece& v){
		buffer_.append(v.data(),v.size());
		return *this;
	}
	
	self& operator<<(const Buffer& v){
		*this <<v.toStringPiece();
		return *this;
	}
	void append(const char* data,int len){buffer_.append(data,len);}
	const Buffer& buffer(){return buffer_;}
	void resetBuff(){buffer_.reset();}
	self& operator<<(const void*);
	self& operator<<(short);
	self& operator<<(int);
	self& operator<<(long);
	self& operator<<(long long);
	self& operator<<(unsigned short);
	self& operator<<(unsigned int);
	self& operator<<(unsigned long);
	self& operator<<(unsigned long long);
	self& operator<<(double);
	


private:
	Buffer buffer_;
	static const int maxNumericSize = 32;
	template<typename T>
	void intos(T);
};

string formatSI(int64_t n);

string formatIEC(int64_t n);

class FmtHel{
public:
	template<typename T>
	FmtHel(const char* fmt, T val);

	const char* data() const {return buf_;}
	int length() const {return length_;}

private:
	char buf_[32];
	int length_;
};


inline LogOutput& operator<<(LogOutput& s,const FmtHel& fmt){
	s.append(fmt.data(),fmt.length());
	return s;
}






#endif
