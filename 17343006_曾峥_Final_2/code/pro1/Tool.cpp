#include "Tool.h"
using namespace cv;

void GetAllFiles(string path, vector<string>& files)
{
	/*
	long   hFile = 0;	//debug x64���С�
	_findfirst()��������Ϊintptr_t����long�ͣ��ӡ�intptr_t��ת������long����ʧ������
	long hFile;  ��Ϊ intptr_t hFile; ���ɡ�
	*/
	intptr_t hFile;
	//�ļ���Ϣ    
	struct _finddata_t fileinfo;//�����洢�ļ���Ϣ�Ľṹ��    
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)  //��һ�β���  
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))  //������ҵ������ļ���  
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)  //�����ļ��в���  
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					GetAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else //������ҵ��Ĳ������ļ���   
			{
				files.push_back(p.assign(fileinfo.name));  //���ļ�·�����棬Ҳ����ֻ�����ļ���:    p.assign(path).append("\\").append(fileinfo.name)  
			}

		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile); //��������  
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
	//ol9b(1).bmp��ʽ
	int index_a, index_b;
	if (isIn(a, '(')) {
		//cout << "long name" << "  ";
		index_a = getIndex(a);
		index_b = getIndex(b);
	}
	//1.bmp��ʽ
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
	if (strFilePath.size() != (rFirstPos + 1))   /* ���ת�����·��������\\����ʱ����ĩβ���\\������ĸ�ʽͳһΪD:\\1\\2\\3\\ */
	{
		//������һ���Ƿ�Ϊ�ļ���
		string strTemp = strFilePath.substr(rFirstPos, strFilePath.size());
		if (string::npos != strTemp.find("."))
		{
			//���һ�������ļ�����
			strFilePath = strFilePath.substr(0, rFirstPos + 1);
		}
		else
		{
			//���һ�����ļ�������
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
				//����ֻ��ϵͳ�̵�·����������磺D:
				continue;
			}

			struct _stat fileStat = { 0 };
			if (_stat(strFolderPath.c_str(), &fileStat) == 0)
			{
				//�ļ����ڣ��ж���ΪĿ¼�����ļ�
				if (!(fileStat.st_mode & _S_IFDIR))
				{
					//�����ļ��У��򴴽�ʧ���˳�����
					return false;
				}
			}
			else
			{
				//�ļ��в����ڣ�����д���
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


Point2f srcTri[4], dstTri[4];
int clickTimes = 0;  //��ͼ���ϵ�������
Mat image;
Mat imageWarp;
string name;

void getCard(string input) {
	string path = "C:\\Users\\inplus\\Desktop\\zengzheng\\";
	path += input;
	image = imread(path);
	cv::resize(image, image, cv::Size(), 0.3, 0.3);

	imshow("Source Image", image);
	setMouseCallback("Source Image", onMouse);
	waitKey();
}


void onMouse(int event, int x, int y, int flags, void* utsc)
{
	if (event == EVENT_LBUTTONUP)   //��Ӧ������̧���¼�
	{
		circle(image, Point(x, y), 0.5, Scalar(0, 0, 0), 2.5);  //���ѡ�е�
		imshow("Source Image", image);
		srcTri[clickTimes].x = x;
		srcTri[clickTimes].y = y;
		clickTimes++;
	}
	if (clickTimes == 4)
	{
		dstTri[0].x = 0;
		dstTri[0].y = 0;
		dstTri[1].x = image.rows - 1;
		dstTri[1].y = 0;
		dstTri[2].x = 0;
		dstTri[2].y = image.cols - 161;
		dstTri[3].x = image.rows - 1;
		dstTri[3].y = image.cols - 161;
		Mat transform = Mat::zeros(3, 3, CV_32FC1); //͸�ӱ任����
		transform = getPerspectiveTransform(srcTri, dstTri);  //��ȡ͸�ӱ任����		
		warpPerspective(image, imageWarp, transform, Size(image.rows, image.cols - 160));  //͸�ӱ任
		imshow("After WarpPerspecttive", imageWarp);
		string folder = "C:\\Users\\inplus\\Desktop\\result\\";
		CreateFolder(folder);
		cout << folder + name << endl;
		cv::imwrite(folder + name, imageWarp);
	}
}