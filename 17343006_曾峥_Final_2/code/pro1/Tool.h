#ifndef _TOOL_H
#define _TOOL_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>	//ʹ��stringstream��Ҫ�������ͷ�ļ�
#include <iomanip>
#include <io.h>
#include <direct.h>
#include <opencv2/core.hpp>
#include <time.h>
#include "core/core.hpp"
#include "highgui/highgui.hpp"
#include "imgproc/imgproc.hpp"

using namespace std;

//�ļ�����
void GetAllFiles(string path, vector<string>& files);
int getIndex(string file);
bool cmp_sortFileByIndex(string a, string b);

//�����ļ���

bool CreateFolder(string strSrcFilePath);

string& replace_all(string& str, const string& old_value, const string& new_value);

// ����
void getCard(string input);
void onMouse(int event, int x, int y, int flags, void* utsc);

#endif