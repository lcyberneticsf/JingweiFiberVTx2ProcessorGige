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
/* �����¼������־����־��*/
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

    //��Ϊ0��1��2��3�ĸ�����Ĭ��Ϊ����2
    //0--������ͣ���¼������־
    //1--����һ�㣬��¼������־����Ҫ��Ϣ
    //2--����ϸߣ�ֻ��¼�����쳣��־
    //3--������ߣ�����¼��־
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
    enum {BUFSIZE = 3000};  //����������
    char	m_tBuf[BUFSIZE];

    int 	m_nLevel;			//�Ƿ��¼��־
  //  CRITICAL_SECTION  m_crit;  	//����һ���ٽ���
};

#endif // !defined(__TEST_LOG__INCLUDED_)
