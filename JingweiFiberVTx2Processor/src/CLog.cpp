//
//////////////////////////////////////////////////////////////////////

#include "CLog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLog::CLog()
{
    m_nLevel = LOG_LEVEL_ERROR;
  //  ::InitializeCriticalSection(&m_crit);   //初始化临界区
}

CLog::~CLog()
{
  //  ::DeleteCriticalSection(&m_crit);    //释放里临界区
}

std::string CLog::getCurrentSystemTime()
{
	auto tt = std::chrono::system_clock::to_time_t
	(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	char date[60] = { 0 };
	sprintf(date, "%d-%02d-%02d-%02d.%02d.%02d",
		(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
		(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
	return std::string(date);
}



void CLog::Add(const char *fmt, ...)
{
    char chPath[512], chFile[512];
  //  GetCurrentPath(chPath);
	strcpy(m_tBuf, fmt);

    strcat(chPath, "./Log");
   // mkdir(chPath);
	//mkdir(chPath, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH);

    struct tm *now;
    time_t ltime;

    time(&ltime);
    now = localtime(&ltime);

    sprintf(chFile, "./Log.txt", chPath);

    /*-----------------------进入临界区(写文件)------------------------------*/
   // ::EnterCriticalSection(&m_crit);

    FILE *fp = fopen(chFile, "a"); //以添加的方式输出到文件
    if (fp)
    {
		//fprintf(fp, "[%s %s:%d]\t", szDate, szTime,systime.wMilliseconds);
		std::string str_time = getCurrentSystemTime();
		fprintf(fp, "[%s:]\t", str_time.c_str());
        fprintf(fp, "%s\n", m_tBuf);
        fclose(fp);
    }
   // ::LeaveCriticalSection(&m_crit);
    /*----------------------------退出临界区---------------------------------*/

}
