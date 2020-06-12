#ifndef _TOOL_H
#define _TOOL_H

#include <iostream>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>	//ʹ��stringstream��Ҫ�������ͷ�ļ�
#include <iomanip>
#include <io.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

using namespace std;

//�ļ�����
void GetAllFiles(string path, vector<string>& files);
int getIndex(string file);
bool cmp_sortFileByIndex(string a, string b);

//�����ļ���
bool CreateFolder(string strSrcFilePath);
string& replace_all(string& str, const string& old_value, const string& new_value);


#endif