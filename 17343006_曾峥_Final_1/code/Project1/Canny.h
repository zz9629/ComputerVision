/*
原理：用离散化的梯度逼近函数根据二维灰度矩阵梯度向量来寻找图像灰度矩阵
	的灰度跃变位置，然后再图像中将这些点连起来就形成了图像的边缘
	一般对图像进行canny边缘检测要经过以下几步：
	1.对图像进行灰度化处理：Gimg = 0.299R+0.587G+0.114B
	2.对图像进行高斯滤波(出去图片中噪声对边缘检测的影响)
		首先调用make_gaussian_kernel生成卷积核，再对图像进行卷积
	3.用一阶偏导的有限差分来计算梯度的幅值和方向
	4.对梯度幅值进行非极大值抑制
	5.双阈值检测和连接边缘
*/
#ifndef CANNY_H_	//如果没有这个宏
#define CANNY_H_	//定义这个宏
#include <iostream>
#include "CImg.h"
#include <cmath>
#include <vector>

using namespace std;
using namespace cimg_library;

class Canny {
public:
	Canny();
	Canny(CImg<float> img);
	Canny(char const* file);
	~Canny();
	void display();
	//默认参数设置,高斯滤波标准差为2.0，低阈值为0.25，高阈值为0.75
	CImg<unsigned char> canny_image();
	//整合所有获取最后的边缘图,sigma表示高斯滤波的参数，tlow和thigh为两个阈值 //会用到private的辅助函数
	CImg<unsigned char> canny_image(float sigma, float tlow, float thigh);
	//选出两个边缘点较近的距离连线
	CImg<unsigned char> canny_line(CImg<int> picture, int distance);
	//删掉长度小于20的边缘线
	CImg<unsigned char> delete_line(CImg<int> picture, int);

private:
//数据
	CImg<unsigned char> img;
	int rows;
	int cols;
	int *smoothedim;	//* The image after gaussian smoothing.      */
	int *delta_x;		//x方向的一阶导数	The first devivative image, x-direction
	int *delta_y;		//y方向的一阶导数	The first derivative image, y-direction.
	float *dirim;		//梯度的方向
	int *magnitude;		//梯度的幅值		The magnitude of the gadient image.
	int *nms;			//非极大值抑制后得到矩阵Points that are local maximal magnitude.
	int *edge;			//边缘数组
	//

//canny过程中主要函数
	// 1.对图像进行灰度化处理，把每个点转为灰色,			phase1
	void RGBtoGray();
	// 2.对图像进行高斯滤波(出去图片中噪声对边缘检测的影响)	phase2
	void gaussian_smooth(float sigma);
	// 3.计算x,y方向的一阶导数								phase3
	void derrivative_x_y();
	//	计算梯度向上的方向，以正x轴为逆时针方向指定的弧度
	void radian_direction(int xdirtag, int ydirtag);
	// 4.计算梯度的幅值										
	void magnitude_x_y();
	// 5.进行非极大值抑制									phase4
	//This routine applies non-maximal suppression to the magnitude of the gradient image.
	void non_max_supp();
	// 6.双阈值检测	This routine finds edges that are above some high threshhold or
		//are connected to a high pixel by a path of pixels greater than a low
	void apply_hysteresis(float tlow, float thigh);

//辅助函数
	//Create a one dimensional gaussian kernel.
	void make_gaussian_kernel(float sigma, float **kernel, int *windowsize);	//2的辅助函数
	//This procedure edges is a recursive routine that traces edgs along
		//all paths whose magnitude values remain above some specifyable lower
	void follow_edges(int *edgemapptr, int *edgemagptr, int lowval, int cols);	//4 非极大值抑制 辅助
	double angle_radians(double x, double y);									//3 辅助
	CImg<unsigned char> Draw_line(CImg<int> tmp, int x, int y, int x1, int y1);	//删掉小于20线 辅助
	//写TXT文件，magnitude、nms、edge
	void txt_generate();
	//生成中间文件，用于测试
	void img_generate();
};

#endif
