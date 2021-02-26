#include "LogOutput.h"

#include <algorithm>
#include <type_traits> //template<int SIZE>
#include <assert.h>
#include <string.h>
#include <stdint.h> //uintptr_t
#include <stdio.h> //snprintf

const char digits[] = "9876543210123456789";
const char* digitsMid = digits+9;

const char num2Hex[] = "0123456789ABCDEF";

//*********************全局函数*********************
template<typename T>
size_t intos_hel(char buf[],T value){
	T t = value;
	char* start = buf;
	do{
		int num = static_cast<char>(t%10);
		t/=10;
		*start++ = digitsMid[num];
	}while(t!=0);
	if(value<0){
		*start++ = '-';
	}
	*start = '\0';
	std::reverse(buf,start);
	return start-buf;

}

//这里的uintptr_t是ISOC99定义的    typedef unsigned long int uintptr_t
size_t hextos_hel(char buf[],uintptr_t val){
	uintptr_t t = val;
	char* start = buf;
	do{
		int num = static_cast<int>(t%16);
		t/=16;
		*start++ = num2Hex[num];
	}while(t!=0);

	*start = '\0';
	std::reverse(buf,start);
	return start-buf;
}

//*********************LogBuffer部分*********************
template class LogBuffer<smallBuffSize>;
template class LogBuffer<largeBuffSize>;

template<int SIZE>
const char* LogBuffer<SIZE>::debugString()
{
	*cur_ = '\0';
	return data_;
}

template<int SIZE>
void LogBuffer<SIZE>::cookieStart(){

}

template<int SIZE>
void LogBuffer<SIZE>::cookieEnd(){

}

//*********************LogOutput部分*********************
template<typename T>
void LogOutput::intos(T t){
	if(buffer_.avail() >= maxNumericSize){
		size_t len = intos_hel(buffer_.current(),t);
		buffer_.extend(len);
	}
}

LogOutput& LogOutput::operator<<(short n){
	*this << static_cast<int>(n);
	return *this;
}

LogOutput& LogOutput::operator<<(int n){
	intos(n);
	return *this;
}

LogOutput& LogOutput::operator<<(long n){
	intos(n);
	return *this;
}

LogOutput& LogOutput::operator<<(long long n){
	intos(n);
	return *this;
}

LogOutput& LogOutput::operator<<(unsigned int n){
	intos(n);
	return *this;
}

LogOutput& LogOutput::operator<<(unsigned long n){
	intos(n);
	return *this;
}

LogOutput& LogOutput::operator<<(unsigned long long n){
	intos(n);
	return *this;
}

LogOutput& LogOutput::operator<<(const void* p){
	uintptr_t tar = reinterpret_cast<uintptr_t>(p);//转换为地址
	//auto x = (buffer_.avail() >= maxNumericSize);
	if(buffer_.avail() >= maxNumericSize){
		char* cur = buffer_.current();
		*cur++ = '0';
		*cur++ = 'x';
		size_t len = hextos_hel(cur,tar);
		buffer_.extend(len+2);
	}

	return *this;
}


LogOutput& LogOutput::operator<<(double num){
	if(buffer_.avail() >= maxNumericSize){
		int len = snprintf(buffer_.current(),maxNumericSize,"%.12g",num);
		buffer_.extend(len);
	}

	return *this;
}

template<typename T>
FmtHel::FmtHel(const char* fmt,T val){
	length_ = snprintf(buf_,sizeof buf_,fmt,val);
	assert(static_cast<size_t>(length_) < sizeof buf_);

}

template FmtHel::FmtHel(const char* fmt,char);
template FmtHel::FmtHel(const char* fmt,short);
template FmtHel::FmtHel(const char* fmt,int);
template FmtHel::FmtHel(const char* fmt,long);
template FmtHel::FmtHel(const char* fmt,long long);
template FmtHel::FmtHel(const char* fmt,unsigned short);
template FmtHel::FmtHel(const char* fmt,unsigned int);
template FmtHel::FmtHel(const char* fmt,unsigned long);
template FmtHel::FmtHel(const char* fmt,unsigned long long);
template FmtHel::FmtHel(const char* fmt,float);
template FmtHel::FmtHel(const char* fmt,double);
