/*----------------------------------------------------------------
* 项目名称 ：$rootnamespace$
* 项目描述 ：
* 类 名 称 ：$safeitemname$
* 类 描 述 ：
* 命名空间 ：$rootnamespace$
* 作    者 ：hantianyou
* 创建时间 ：$time$
* 更新时间 ：$time$
* 版 本 号 ：v1.0.0.0
*******************************************************************
* Copyright @ hantianyou 2020. All rights reserved.
*******************************************************************
//----------------------------------------------------------------*/
#include "cut_code.h"

std::string cut_code(std::string source_code, std::string head, std::string end)
{
    int pos1 = 0;
    int pos2 = 0;
    pos1 = source_code.find(head);
    if (-1 == pos1)
    {
        return "false";
    }
    pos1 += head.length();
    std::string str2 = source_code.substr(pos1);
    pos2 = str2.find_first_of(end);
    pos2 += end.length();
    std::string str3 = str2.substr(0, pos2);
    return str3;
}

int cut_result(std::string &source_code, std::string head, std::string end)
{
    int pos1 = 0;
    int pos2 = 0;
    pos1 = source_code.find(head);
    if (-1 == pos1)
    {
        return 0;
    }
    pos1 += head.length();
    std::string str2 = source_code.substr(pos1);
    pos2 = str2.find_first_of(end);
    pos2 += end.length();
    std::string str3 = str2.substr(pos2);
    source_code= str3;
    //std::cout << "my_cut" << str3 << std::endl;
    return 1;
}

bool get_m_name(std::string source_code, std::string head, std::string end, std::string& name)
{
    int pos1 = 0;
    int pos2 = 0;
    pos1 = source_code.find(head) + head.length();
    if (-1 == pos1)
    {
        return 0;
    }
    std::string str2 = source_code.substr(pos1);
    pos2 = str2.find_first_of(end);
    std::string str3 = str2.substr(0, pos2);
    name = str3;
    return true;
}

bool get_m_score(std::string source_code, std::string head, std::string end, float& a)
{
    int pos1 = 0;
    int pos2 = 0;
    pos1 = source_code.find(head) + head.length();
    if (-1 == pos1)
    {
        return 0;
    }
    std::string str2 = source_code.substr(pos1);
    pos2 = str2.find_first_of(end);
    std::string str3 = str2.substr(0, pos2);
    a = std::atof(str3.c_str());
    return true;
}
float print_x(std::string checkout,int number) 
{
    static int i = 0;
    std::string head_x = "x: "; std::string end_x = "\n";
    std::string my_result_x = cut_code(checkout, head_x, end_x);
    //std::cout << checkout << std::endl;
    if (my_result_x == "false") 
	{
        return -1;
    }
    else
    {
        float floatStr_x = atof(my_result_x.c_str());
       // std::cout << "picture_" << number << " " << "x" << i << "=" << floatStr_x << std::endl;
        i++;
        //if (i == 4)i = 0;
        return floatStr_x;
    }

}
float print_y(std::string checkout,int number) {
    static int i = 0;
    char c; char c_check = 'y'; c = checkout.at(8);
    //std::cout << "c:" << c << std::endl;
    if (c != c_check) 
    {
        float floatStr_y = 0;
       // std::cout << "picture_" << number << " " << "y" << i << "=" << floatStr_y << std::endl;
        i++;
        //if (i == 4)i = 0;
        return floatStr_y;
    }
    else
    {
        std::string head_y_true = "y: "; std::string end_y_true = "\n";
        std::string my_result_y = cut_code(checkout, head_y_true, end_y_true);
        if (my_result_y == "false") 
		{
            return -1;
        }
        else 
		{
            float floatStr_y = atof(my_result_y.c_str());
           // std::cout << "picture_" << number << " " << "y" << i << "=" << floatStr_y << std::endl;
            i++;
            //if (i == 4)i = 0;
            return floatStr_y;
        }
    }
}

char* change_char(float number) 
{
    char* buffer = new char[30];
    sprintf(buffer, "%.3f", number);
    return buffer;
}

char* change_char(int number) 
{
    char* buffer = new char[30];
    sprintf(buffer, "%d", number);
    return buffer;
}