#ifndef LOGGER__H__
#define LOGGER__H__

#include <functional>

#include "TimeTools.h"
#include "LogOutput.h"

class TimeZone;

enum LogEvent{
	TRACE,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL,
	NUM_LOG_EVENTS,
};

class TarFile{
public:
	template<int N>
	TarFile(const char (&path)[N])
	:path_(path),size_(N-1){
		const char* splitc = strrchr(path_,'/');
			if(splitc){
			path_ = splitc + 1;
			size_ -= static_cast<int>(path_ - path);  
		}
		

	}
	explicit TarFile(const char* path)
	:path_(path),size_(strlen(path)){
		const char* splitc = strrchr(path,'/');
		path_ = splitc + 1;
		size_ = static_cast<int>(strlen(path_));
	}
	
	const char* path_;
	int size_;
};
class LogConta{
public:
	LogConta(LogEvent event,int old_errno,const TarFile& file,int line);
	void formatTime();
	void finish();
	Timestamp time_;
	LogOutput output_;
	LogEvent event_;
	int line_;
	TarFile sourFile_;
};
class Logger{
public:
	Logger(TarFile file,int line);
	Logger(TarFile file,int line,bool w2abort);
	Logger(TarFile file,int line,LogEvent event);
	Logger(TarFile file,int line,LogEvent event,const char* func);

	~Logger();

	LogOutput& output() {return logconta_.output_;}
	static LogEvent logEvent();
	static void setLogEvent(LogEvent event);

	using OutputFunc = std::function<void(const char* msg,int len)>;
	using FlushFunc = std::function<void()>;
	
	static void setOutputFunc(OutputFunc);
	static void setFlushFunc(FlushFunc);
	static void setTimeZone(const TimeZone& tz);
private:
	LogConta logconta_;
};

extern LogEvent glo_logEvent;
inline LogEvent Logger::logEvent(){
	return glo_logEvent;
} 

#define LOG_TRACE if (Logger::logEvent() <= TRACE) \
	Logger(__FILE__,__LINE__,TRACE,__func__).output()
#define LOG_DEBUG if (Logger::logEvent() <= DEBUG) \
	Logger(__FILE__,__LINE__,DEBUG,__func__).output()
#define LOG_INFO if (Logger::logEvent() <= INFO) \
	Logger(__FILE__,__LINE__).output()
#define LOG_WARN Logger(__FILE__,__LINE__,WARN).output()
#define LOG_ERROR Logger(__FILE__,__LINE__,ERROR).output()
#define LOG_FATAL Logger(__FILE__,__LINE__,FATAL).output()
#define LOG_SYSERR Logger(__FILE__,__LINE__,false).output()
#define LOG_SYSFATAL Logger(__FILE__,__LINE__,true).output()

const char* errno2str(int savedErrno);



#endif
