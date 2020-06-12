/*
	1.对图像进行灰度化处理：Gimg = 0.299R+0.587G+0.114B
	2.对图像进行下采样（分辨率高的图像变小），以后处理均是这个图，得到4个顶点之后画在输出图像上。
	3.Canny算法提取边缘（得到的边缘有噪点且不完整）
	4.Hough变换（输入图像得到4个顶点）
		（1）对ImageSpace做HoughSpace（极坐标）变换，并做voting投票（矩阵累加）
		（2）对累加矩阵取最大的一些值，画出边缘直线
		（3）求每两条直线之间交点，并累计该交点的直线交叉次数
		（4）取交叉次数最多的4个点，即为A4纸的四个顶点
	5.将Hough变换得到的4个顶点放到原图中，画出4个顶点及边缘
*/
#ifndef _EDGEDETECT_H
#define _EDGEDETECT_H
#include <iostream>
#include <string>
#include <vector>
#include "CImg.h"
#include "Tool.h"
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>	//使用stringstream需要引入这个头文件
#include <iomanip>
#include <io.h>
#include <direct.h>

using namespace std;
using namespace cimg_library;
struct Plot {
	int x, y;
	Plot(int x_, int y_) :x(x_), y(y_) {}
};
struct Line {
	int rho, theta;
	int weight;
	double k, b;
	Line(int x, int y, int w) :rho(x), theta(y), weight(w) {}
	void setRhoTheta(int x, int y, int w) {
		rho = x;
		theta = y;
		weight = w;
	}
	void setKB(double kk, double bb) {
		k = kk, b = bb;
	}
};
struct Vertex {
	int x, y;
	int values = 0;
	Vertex(int posX, int posY) : x(posX), y(posY) {}
	void setXY(int _x, int _y) {
		x = _x;
		y = _y;
	}
	void addCrossTimes() {
		values++;
	}
};
struct Center {
	int a, b;
	int weight;
	Center(int x, int y, int w) : a(x), b(y), weight(w) {}
};
struct Circle {
	int a, b;	//圆心
	int r;		//半径
	Circle(int x, int y, int rr) : a(x), b(y), r(rr) {}
	void setR(int a) {
		r = a;
	}
	void setCenter(int x, int y) {
		a = x;
		b = y;
	}
};
class EdgeDetect {
public:
	EdgeDetect();
	~EdgeDetect();
	void edgeDetect(string, string, string, int);// 输入图像，及参数
private:
	//直线相关函数
	void lineDetect(string, string, int);
	CImg<float> Binarization(CImg<float> srcImg, float alpha);
	CImg<float> RGBtoGray(CImg<float> img);				//灰度处理
	CImg<float> downSampling(CImg<float> img, int num);	//下采样处理，num为缩小的倍数
	CImg<float> canny(CImg<float> img);					//canny提取边缘
	CImg<float> houghTransform(CImg<float>);		//Hough变换
	void houghReCompute(CImg<float>&);				//Hough高峰提取
	int getMaxHough(CImg<float>& img, int& size, int& y, int& x); //辅助函数
	void findEdge(CImg<float>&);						// 检测边缘
	void findVertex();									// 检测角点
	void drawVertexLines(CImg<float> &, int);			//画出顶点到边缘


	CImg<float> img; // 原图像
	CImg<float> imgGrayed; // 灰度图像
	CImg<float> imgDowned;	//下采样，这个图像用于canny函数画出边缘
	CImg<float> imgCannyed; // 经过阈值处理后的图像
	CImg<float> houghImage; // 霍夫空间图像
	CImg<float> outputImage; // 霍夫变换检测出来的图像
	vector<float> setSin; // sin集合
	vector<float> setCos; // cos集合

	int pointNumber;		// 角点数
	CImg<float> edge;		// 边缘直线
	vector<Line*> myLines;	// 检测出的直线
	vector<Vertex*> myVertexs;	//检测出来的最终4个顶点，再由顶点画出直线
	
	void test();			//测试函数，输出各阶段的图像
	map<int, vector<Plot>> blocks;			//连通块（label，点集）
	string name;

};
#endif