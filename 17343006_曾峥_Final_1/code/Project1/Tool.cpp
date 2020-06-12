#include "Tool.h"

void GetAllFiles(string path, vector<string>& files)
{
	/*
	long   hFile = 0;	//debug x64不行。
	_findfirst()返回类型为intptr_t而非long型，从“intptr_t”转换到“long”丢失了数据
	long hFile;  改为 intptr_t hFile; 即可。
	*/
	intptr_t hFile;
	//文件信息    
	struct _finddata_t fileinfo;//用来存储文件信息的结构体    
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)  //第一次查找  
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))  //如果查找到的是文件夹  
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)  //进入文件夹查找  
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					GetAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else //如果查找到的不是是文件夹   
			{
				files.push_back(p.assign(fileinfo.name));  //将文件路径保存，也可以只保存文件名:    p.assign(path).append("\\").append(fileinfo.name)  
			}

		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile); //结束查找  
	}
}

int getIndex(string file) {
	cout << endl << "file: " << file << "    ";
	int left = 0;
	int right = 0;
	string index_str = "";
	for (int i = 0; i < file.length(); i++) {
		if (file[i] == '(')left = i;
		if (file[i] == ')')right = i;
	}
	for (int i = left + 1; i < right; i++)
		index_str += file[i];
	cout << "index_str: " << index_str << endl;
	int index = atoi(index_str.c_str());
	return index;
}
bool isIn(string str, char c) {
	for (auto i : str){
		if (i == c)	return true;
	}
	return false;
}
bool cmp_sortFileByIndex(string a, string b) {
	//ol9b(1).bmp形式
	int index_a, index_b;
	if (isIn(a, '(')) {
		//cout << "long name" << "  ";
		index_a = getIndex(a);
		index_b = getIndex(b);
	}
	//1.bmp形式
	else {
		index_a = atoi(a.c_str());
		index_b = atoi(b.c_str());
	}
	
	return index_a < index_b;
}

bool CreateFolder(string strSrcFilePath)
{
	string strFilePath = replace_all(strSrcFilePath, "/", "\\");
	string::size_type rFirstPos = strFilePath.rfind("\\");
	if (strFilePath.size() != (rFirstPos + 1))   /* 如果转换后的路径不是以\\结束时候，往末尾添加\\，处理的格式统一为D:\\1\\2\\3\\ */
	{
		//检测最后一个是否为文件名
		string strTemp = strFilePath.substr(rFirstPos, strFilePath.size());
		if (string::npos != strTemp.find("."))
		{
			//最后一个不是文件夹名
			strFilePath = strFilePath.substr(0, rFirstPos + 1);
		}
		else
		{
			//最后一个是文件夹名字
			strFilePath += "\\";
		}
	}
	else
	{
		strFilePath += "\\";
	}

	string::size_type startPos(0);
	string::size_type endPos(0);

	while (true)
	{
		if ((endPos = strFilePath.find("\\", startPos)) != string::npos)
		{
			string strFolderPath = strFilePath.substr(0, endPos);
			startPos = endPos + string::size_type(1);

			if (strFolderPath.rfind(":") == (strFolderPath.size() - 1))
			{
				//跳过只有系统盘的路径的情况，如：D:
				continue;
			}

			struct _stat fileStat = { 0 };
			if (_stat(strFolderPath.c_str(), &fileStat) == 0)
			{
				//文件存在，判断其为目录还是文件
				if (!(fileStat.st_mode & _S_IFDIR))
				{
					//不是文件夹，则创建失败退出返回
					return false;
				}
			}
			else
			{
				//文件夹不存在，则进行创建
				if (-1 == _mkdir(strFolderPath.c_str()))
				{
					return false;
				}
			}

			continue;
		}

		break;
	}
	return true;
}

string& replace_all(string& str, const string& old_value, const string& new_value)
{
	string::size_type  pos(0);
	while (true)
	{
		if ((pos = str.find(old_value, pos)) != string::npos)
		{
			str.replace(pos, old_value.length(), new_value);
		}
		else
		{
			break;
		}
	}
	return  str;
}