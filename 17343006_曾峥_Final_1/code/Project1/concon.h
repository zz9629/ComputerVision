#ifndef CONCON_H
#define CONCON_H
#include "CImg.h"
#include <vector>

using namespace cimg_library;
using namespace std;

class Concon {
private:
	unsigned char *data; /* input image */
	int width;
	int height;
	int *idata;          /* output for edges */
	int *magnitude;      /* edge magnitude as detected by Gaussians */
	float *xConv;        /* temporary for convolution in x direction */
	float *yConv;        /* temporary for convolution in y direction */
	float *xGradient;    /* gradients in x direction, as detected by Gaussians */
	float *yGradient;    /* gradients in x direction,a s detected by Gaussians */

	void cannyparam(unsigned char *grey, int width, int height,
		float lowThreshold, float highthreshold,
		float gaussiankernelradius, int gaussiankernelwidth,
		int contrastnormalised);
	Concon *allocatebuffers(unsigned char *grey, int width, int height);
	void killbuffers();
	int computeGradients(float kernelRadius, int kernelWidth);
	void performHysteresis(int low, int high);
	void follow(int x1, int y1, int i1, int threshold);

	void normalizeContrast(unsigned char *data, int width, int height);
	float hypotenuse(float x, float y);
	float gaussian(float x, float sigma);

	//canny函数，用于直线检测
	vector<vector<float> > createFilter(int, int, float);		// 产生高斯滤波器
	CImg<float> useFilter(CImg<float>&, vector<vector<float> > & ); // 进行高斯滤波
	CImg<float> sobel(CImg<float>&, CImg<float>&); // 产生sobel算子
	CImg<float> nonMaxSupp(CImg<float>&, CImg<float>&); // 进行非最大化抑制
	CImg<float> threshold(CImg<float>&, int, int); // 双阈值处理

public:
	unsigned char* result;
	unsigned char* middleChar;
	int* middleInt;
	float* middleFloat;

	CImg<unsigned char> toGrayScale(CImg<unsigned char> src);
	unsigned char *canny(unsigned char *grey, int width, int height, float lowthreshold, float highthreshold, float gaussiankernelradius, int gaussiankernelwidth);
	CImg<float> canny(CImg<float> img, int gausFilterX, int  gausFilterY, int ssigma, int tlow, int thigh);
};

#endif