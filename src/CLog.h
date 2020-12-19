//
//////////////////////////////////////////////////////////////////////

#if !defined(__TEST_LOG__INCLUDED)
#define __TEST_LOG__INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <direct.h>
#include <time.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h> 
#include <string>
#include <chrono>

/************************************************************************/
/* 负责记录程序日志的日志类*/
/************************************************************************/

#define LOG_LEVEL_ALL			0
#define LOG_LEVEL_IMPORT		1
#define LOG_LEVEL_ERROR			2
#define LOG_LEVEL_NONE			3

class CLog
{
public:
    CLog();
    virtual ~CLog();

    //分为0、1、2、3四个级别，默认为级别2
    //0--级别最低，记录所有日志
    //1--级别一般，记录错误日志和重要信息
    //2--级别较高，只记录错误及异常日志
    //3--级别最高，不记录日志
    void SetLogLevel(int nLevel)
    {
        m_nLevel = nLevel;
    };
    bool  GetWriteLog()
    {
        return m_nLevel;
    };

    void Add(const char *fmt, ...);
	std::string getCurrentSystemTime();

private:
    enum {BUFSIZE = 3000};  //工作缓冲区
    char	m_tBuf[BUFSIZE];

    int 	m_nLevel;			//是否记录日志
  //  CRITICAL_SECTION  m_crit;  	//设置一个临界区
};

#endif // !defined(__TEST_LOG__INCLUDED_)
