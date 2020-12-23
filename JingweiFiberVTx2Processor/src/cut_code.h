#pragma once
#ifndef CUT_CODE_H
#define CUT_CODE_H
#include <string.h>
#include <iostream>

std::string cut_code(std::string source_code, std::string head, std::string end);
int cut_result(std::string &source_code, std::string head, std::string end);
bool get_m_name(std::string source_code, std::string head, std::string end, std::string& name);
bool get_m_score(std::string source_code, std::string head, std::string end, float& a);
float print_x(std::string checkout,int number);
float print_y(std::string checkout,int number);
char* change_char(int number);
char* change_char(float number);
#endif // CUT_CODE_H
