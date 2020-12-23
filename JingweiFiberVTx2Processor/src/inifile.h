#pragma once


#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <map>

using namespace std;


class CIniFile
{
public:
	CIniFile(void);
	CIniFile(const char * chFileName);
	CIniFile(const string &strFileName);
	~CIniFile(void);


	int Init();
	int Init(const char* chFileName);
	int Init(const string& strFileName);
	int Dump();


	int ReadItem(const string& strSection, const string& strKey, const string& strDefault, string& strValue);
	int WriteItem(const string& strSection, const string& strKey, const string& strValue);

private:
	int LoadFile();
	int WriteFile();

	int ReadLine(FILE* pFile, string& strLine);
	int TrimString(string& strToken);

private:
	static const int BUFFER_LEN = 1024;
	string m_strFileName;
	typedef map<string, string> ConfigType;
	map<string, ConfigType> m_mSec2Config;


};


#ifndef UINT
typedef unsigned int UINT;
#endif
#ifndef LPCSTR
typedef const char* LPCSTR;
#endif
#ifndef LPSTR
typedef char* LPSTR;
#endif
#ifndef INT
typedef int INT;
#endif
//#ifndef UINT32
//typedef unsigned int UINT32;
//#endif
#ifndef DWORD
//typedef unsigned long DWORD;
typedef unsigned int DWORD;
#endif
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

//UINT	GetPrivateProfileInt(
//	LPCSTR lpAppName,
//	LPCSTR lpKeyName,
//	INT nDefault,
//	LPCSTR lpFileName
//);
BOOL	WritePrivateProfileString(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpString,
    LPCSTR lpFileName
    );
DWORD	GetPrivateProfileString(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
    );

DWORD	GetPrivateProfileSection(
    LPCSTR lpAppName,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
    );

//DWORD	GetPrivateProfileSectionNames(
//    LPSTR lpszReturnBuffer,
//    DWORD nSize,
//    LPCSTR lpFileName
//    );

#endif /* __CONFIG_H__ */
