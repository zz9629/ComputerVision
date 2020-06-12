#include "CImg.h"
#include "ImgSeg.h"
#include <time.h>
#include "core/core.hpp"
#include "highgui/highgui.hpp"
#include "imgproc/imgproc.hpp"

#define alpha 1.0

void findGroups(string);
void test();
void getCard(string input);
void onMouse(int event, int x, int y, int flags, void* utsc);
Point2f srcTri[4], dstTri[4];
int clickTimes = 0;  //在图像上单击次数
Mat image;
Mat imageWarp;

string name;

int main() {
	clock_t start, end;
	start = clock();
	string fileFolderName = "C:\\Users\\inplus\\Desktop\\Project1\\Project1\\cutImg\\";
	//test();
	findGroups(fileFolderName);
	end = clock();
	cout << "\nTotal time: " << (end - start) / CLOCKS_PER_SEC << "s" << endl;
	//cout << "\nAverage time: " << (end - start) / CLOCKS_PER_SEC / 8 << "s" << endl;
}

void findGroups(string fileFolderName) {
	clock_t start, end;
	start = clock();
	vector<string> files;
	GetAllFiles(fileFolderName.c_str(), files);

	string resultPath = "result\\";
	CreateFolder(resultPath);
	for (auto i : files) {
		//if (i == "IMG_20191204_001722.bmp")	
		ImgSeg object;
		int distance = 150;
		if (i == "IMG_20191204_000806.bmp" || i == "IMG_20191204_012728.bmp") {
			distance = 50;
		}
		else if (i == ".bmp") distance = 300;
		//else if()
		//if(i == "IMG_20191204_013949.bmp")
		object.findTarget(i, fileFolderName + i, "test", resultPath + i, 0, distance);	//测试模式
	}
	end = clock();
	cout << "\nTotal time for finding Groups: " << (end - start) / CLOCKS_PER_SEC << "s" << endl;
}
void getCard(string input) {
	image = imread(input);
	cv::resize(image, image, cv::Size(), 0.2, 0.2);

	imshow("Source Image", image);
	setMouseCallback("Source Image", onMouse);
	waitKey();
}

void test() {
	string fileFolder = "C:\\Users\\inplus\\Desktop\\Project1\\Project1\\Dataset\\";
	vector<string> files;
	GetAllFiles(fileFolder.c_str(), files);

	string resultPath = "result\\";
	CreateFolder(resultPath);
	for (auto i : files) {
		cout << i << endl;
		name = i;
		getCard(fileFolder + i);
	}
}
void onMouse(int event, int x, int y, int flags, void* utsc)
{
	if (event == EVENT_LBUTTONUP)   //响应鼠标左键抬起事件
	{
		circle(image, Point(x, y), 2.5, Scalar(0, 0, 255), 2.5);  //标记选中点
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
		Mat transform = Mat::zeros(3, 3, CV_32FC1); //透视变换矩阵
		transform = getPerspectiveTransform(srcTri, dstTri);  //获取透视变换矩阵		
		warpPerspective(image, imageWarp, transform, Size(image.rows, image.cols - 160));  //透视变换
		imshow("After WarpPerspecttive", imageWarp);
		string folder = "C:\\Users\\inplus\\Desktop\\result\\";
		CreateFolder(folder);
		cout << folder + name << endl;
		cv::imwrite(folder + name, imageWarp);
	}
}