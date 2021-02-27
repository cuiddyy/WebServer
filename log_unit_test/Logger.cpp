#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>  //localtime_r

#include "Logger.h"
#include "LogOutput.h"
__thread char t_errnobuf[512];
__thread char t_time[128];
__thread time_t t_lastSecond;

//*****************处理错误**********************************
const char* strerror_tl(int savedErrno){
	return strerror_r(savedErrno,t_errnobuf,sizeof t_errnobuf);
}

LogEvent initLogEvent(){
	if(::getenv("START_LOG_TRACE"))
		return TRACE;
	else if(::getenv("START_LOG_DEBUG"))
		return DEBUG;
	else
		return INFO;
}

LogEvent glo_logEvent = initLogEvent();

const char* LogEventName[NUM_LOG_EVENTS] = 
{
	"TRACE ",
	"DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL "
};

class T{
public:
	T(const char* str,unsigned len):str_(str),len_(len){
		assert(strlen(str) == len_);
	}
	const char* str_;
	const unsigned len_;
};

//********************输出流全局重载********************************
inline LogOutput& operator<<(LogOutput& output,T t){
	output.append(t.str_,t.len_);
	return output;
}

inline LogOutput& operator<<(LogOutput& output,const TarFile& t){
	output.append(t.path_,t.size_);
	return output;
}

//最后将数据输出到终端上的函数
void defaultOutput(const char* msg,int len){
	size_t n = fwrite(msg,1,len,stdout);
	(void)n;
}
//刷新输出数据流
void defaultFlush(){
	fflush(stdout);
}

//设置Logger内的功能函数
Logger::OutputFunc glo_output = std::bind(&defaultOutput,std::placeholders::_1,std::placeholders::_2);
Logger::FlushFunc glo_flush = std::bind(&defaultFlush);

//**********************LogConta部分*********************************
LogConta::LogConta(LogEvent event,int savedErrno,const TarFile& file,int line)
	:time_(Clock::now()),
	output_(),
	event_(event),
	line_(line),
	sourFile_(file)
{
	formatTime();
	//printf("LogConta::Logconta::s=%s\n",output_.buffer().data());
	output_ << T(LogEventName[event],6);
	//printf("LogConta::Logconta::s=%s\n",output_.buffer().data());
	if(savedErrno !=0 ){
		output_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
	}	
}

void LogConta::formatTime(){
	Duration duration_since_eppoch = time_.time_since_epoch(); //从1970-01-01 00:00:00到当前时间点的时长
	time_t microSeconds_since_epoch = std::chrono::duration_cast<MCS>(duration_since_eppoch).count(); //将时长转换为微秒数
	time_t kMicroSecondsPersecond = 1000*1000;
	time_t seconds_since_epoch = microSeconds_since_epoch/kMicroSecondsPersecond;       //将时长转换成秒数
	int microseconds = microSeconds_since_epoch % kMicroSecondsPersecond;
	if(seconds_since_epoch != t_lastSecond){
		t_lastSecond = seconds_since_epoch;
		struct tm tm_time;
		::localtime_r(&seconds_since_epoch,&tm_time);
		int len = snprintf(t_time,sizeof(t_time),"%4d-%02d-%02d %02d:%02d:%02d",
						tm_time.tm_year+1900,tm_time.tm_mon+1,tm_time.tm_mday,tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec);
	//	printf("len=%d__time=%s__secons=%lld\n",len,t_time,seconds_since_epoch);
		assert(len == 19);
		(void)len;
		FmtHel us(".%06dZ ",microseconds);
		//printf("us.length()=%d__buf=%s\n",us.length(),us.data());
		assert(us.length() == 9);
		output_ << T(t_time,19) << T(us.data(),9);	
		
	}

		
}

void LogConta::finish(){
	output_ << " - " << sourFile_ << ":" <<line_ << '\n';
}

//**************************Logger相关***************************************
Logger::Logger(TarFile file,int line):logconta_(INFO,0,file,line){
	
}

Logger::Logger(TarFile file,int line,LogEvent event,const char* func):logconta_(event,0,file,line){
	
	logconta_.output_ << func << ' ';
}

Logger::Logger(TarFile file,int line,LogEvent event):logconta_(event,0,file,line){
	
}
Logger::Logger(TarFile file,int line,bool toAbort):logconta_(toAbort?FATAL:ERROR,errno,file,line){
	
}

Logger::~Logger(){
	logconta_.finish();
	
	const LogOutput::Buffer& buf(output().buffer());
	glo_output(buf.data(),buf.length());
	if(logconta_.event_ == FATAL){
		glo_flush();
		abort();
	}
}
void Logger::setLogEvent(LogEvent event){
	glo_logEvent = event;
}

void Logger::setOutputFunc(OutputFunc outputf){
	glo_output = outputf;
}

void Logger::setFlushFunc(FlushFunc flushf){
	glo_flush = flushf;
}


