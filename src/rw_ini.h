#ifndef RW_INI_H
#define RW_INI_H
#include <map>
#include <string>

using namespace std;
//
//struct txt_line_flds
//{
//	int  fldsp;        /* fld start pointer */
//	int  fldlen;       /* fld len */
//};
//
//#define MAX_TXT_LINE_FLD                         128 
//extern struct txt_line_flds txt_line_fld[];
//
//#define MAX_CFG_BUF                              512 
//
//#define CFG_OK                                   0 
//#define CFG_SECTION_NOT_FOUND                    -1 
//#define CFG_KEY_NOT_FOUND                        -2 
//#define CFG_ERR                                  -10 
//#define CFG_ERR_FILE                             -10 
//#define CFG_ERR_OPEN_FILE                        -10 
//#define CFG_ERR_CREATE_FILE                      -11 
//#define CFG_ERR_READ_FILE                        -12 
//#define CFG_ERR_WRITE_FILE                       -13 
//#define CFG_ERR_FILE_FORMAT                      -14 
//#define CFG_ERR_SYSTEM                           -20 
//#define CFG_ERR_SYSTEM_CALL                      -20 
//#define CFG_ERR_INTERNAL                         -21 
//#define CFG_ERR_EXCEED_BUF_SIZE                  -22 
//
//#define COPYF_OK                                 0 
//#define COPYF_ERR_OPEN_FILE                      -10 
//#define COPYF_ERR_CREATE_FILE                    -11 
//#define COPYF_ERR_READ_FILE                      -12 
//#define COPYF_ERR_WRITE_FILE                     -13 
//
//#define TXTF_OK                                  0 
//#define TXTF_ERR_OPEN_FILE                       -1 
//#define TXTF_ERR_READ_FILE                       -2 
//#define TXTF_ERR_WRITE_FILE                      -3 
//#define TXTF_ERR_DELETE_FILE                     -4 
//#define TXTF_ERR_NOT_FOUND                       -5 
//
#define CONFIGLEN           256

enum INI_RES
{
    INI_SUCCESS,            //成功
    INI_ERROR,              //普通错误
    INI_OPENFILE_ERROR,     //打开文件失败
    INI_NO_ATTR            //无对应的键值
};

//              子键索引    子键值
typedef map<std::string,std::string> KEYMAP;
//              主键索引 主键值
typedef map<std::string,KEYMAP> MAINKEYMAP;
// config 文件的基本操作类

class CIni
{
public:
    // 构造函数
    CIni();

    // 析够函数
    virtual ~CIni();
public:
    //获取整形的键值
    int  GetInt(const char* mAttr, const char* cAttr );
    //获取键值的字符串
    char *GetStr(const char* mAttr, const char* cAttr );
    // 打开config 文件
    INI_RES OpenFile(const char* pathName, const char* type);
    // 关闭config 文件
    INI_RES CloseFile();
	//int  CFG_set_key(char* CFG_file, char* section, char* key, char* buf);
	//int WritePrivateProfileString_(char* section, char* key, char* valStr, char* fName);
protected:
    // 读取config文件
    INI_RES GetKey(const char* mAttr, const char* cAttr, char* value);
protected:
    // 被打开的文件局柄
    FILE* m_fp;
    char  m_szKey[ CONFIGLEN ];
    MAINKEYMAP m_Map;
};

#endif
 
