#ifndef _TOOL_H
#define _TOOL_H

#include <iostream>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>	//使用stringstream需要引入这个头文件
#include <iomanip>
#include <io.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

using namespace std;

//文件处理
void GetAllFiles(string path, vector<string>& files);
int getIndex(string file);
bool cmp_sortFileByIndex(string a, string b);

//创建文件夹
bool CreateFolder(string strSrcFilePath);
string& replace_all(string& str, const string& old_value, const string& new_value);


#endif