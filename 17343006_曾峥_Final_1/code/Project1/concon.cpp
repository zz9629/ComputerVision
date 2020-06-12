#include "concon.h"
#include "CImg.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#define Pi 3.141592653
using namespace cimg_library;

#define ffabs(x) ( (x) >= 0 ? (x) : -(x) ) 
#define GAUSSIAN_CUT_OFF 0.005f
#define MAGNITUDE_SCALE 100.0f
#define MAGNITUDE_LIMIT 1000.0f
#define MAGNITUDE_MAX ((int) (MAGNITUDE_SCALE * MAGNITUDE_LIMIT))

CImg<unsigned char> Concon::toGrayScale(CImg<unsigned char> src) {
	int width = src.width();
	int height = src.height();
	int depth = src.depth();

	//New grayscale images.
	CImg<unsigned char> gray(width, height, depth, 1);
	unsigned char r, g, b;
	unsigned char gr1 = 0;

	/* Convert RGB image to grayscale image */
	for (int i = 0; i<width; i++) {
		for (int j = 0; j<height; j++) {
			//Return a pointer to a located pixel value. 
			r = src(i, j, 0, 0); // First channel RED
			g = src(i, j, 0, 1); // Second channel GREEN
			b = src(i, j, 0, 2); // Third channel BLUE
								 //PAL: Y = 0.299*R + 0.587*G + 0.114*B
			gr1 = round(0.299*((double)r) + 0.587*((double)g) + 0.114*((double)b));
			gray(i, j, 0, 0) = gr1;
		}

	}
	return gray;
}

/*
  Canny edge detection with default parameters
    Params: grey - the greyscale image
	        width, height - image width and height
    Returns: binary image with edges as set pixels
*/
unsigned char* Concon::canny(unsigned char *grey, int width, int height, float lowthreshold, float highthreshold, float gaussiankernelradius, int gaussiankernelwidth)
{
	cannyparam(grey, width, height, lowthreshold, highthreshold, gaussiankernelradius, gaussiankernelwidth, 0);
	return result;
}
CImg<float> Concon::canny(CImg<float> img, int gausFilterX, int  gausFilterY, int ssigma, int tlow, int thigh) {
	vector<vector<float> > filter;	// 滤波器

	filter = createFilter(gausFilterX, gausFilterY, ssigma); // 产生高斯滤波器
	CImg<float> gFiltered = useFilter(img, filter); // 进行高斯滤波
	CImg<float> angles;
	CImg<float> sFiltered = sobel(gFiltered, angles); // 产生sobel算子并计算梯度幅值和角度图像
	CImg<float> nms = nonMaxSupp(sFiltered, angles); // 非最大化抑制处理
	CImg<float> ret = threshold(nms, tlow, thigh); // 双阈值处理
	return ret;
}

/*高斯滤波器*/
vector<vector<float> > Concon::createFilter(int row, int col, float tempSigma) {
	vector<vector<float>> filter; // 滤波器
	float sum = 0, temp = 2.0 * tempSigma * tempSigma;
	/*初始化*/
	for (int i = 0; i < row; i++) {
		vector<float> v(col, 0);
		filter.push_back(v);
	}

	for (int i = -row / 2; i <= row / 2; i++) {
		for (int j = -col / 2; j <= col / 2; j++) {
			filter[i + row / 2][j + col / 2] = exp(-(i * i + j * j) / temp) / sqrt(Pi * temp); // 高斯函数
			sum += filter[i + row / 2][j + col / 2];
		}
	}
	// 归一化
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			filter[i][j] /= sum;
		}
	}
	return filter;
}

/*进行高斯滤波*/
CImg<float> Concon::useFilter(CImg<float>& img, vector<vector<float> >& filt) {
	int size = filt.size() / 2;
	CImg<float> filtered(img._width - 2 * size, img._height - 2 * size, 1, 1);
	for (int i = size; i < img._width - size; i++) {
		for (int j = size; j < img._height - size; j++) {
			float sum = 0;
			for (int x = 0; x < filt.size(); x++) {
				for (int y = 0; y < filt.size(); y++) {
					sum += filt[x][y] * (float)(img(i + x - size, j + y - size)); // 高斯滤波
				}
			}
			filtered(i - size, j - size) = sum;
		}
	}
	return filtered;
}

/*利用sobel算子计算梯度幅值和角度图像*/
CImg<float> Concon::sobel(CImg<float>& gFiltered, CImg<float>& angles) {
	/*sobel算子*/
	vector<vector<float>> xFilter(3, vector<float>(3, 0)), yFilter(3, vector<float>(3, 0));
	xFilter[0][0] = xFilter[2][0] = yFilter[0][0] = yFilter[0][2] = -1;
	xFilter[0][2] = xFilter[2][2] = yFilter[2][0] = yFilter[2][2] = 1;
	xFilter[1][0] = yFilter[0][1] = -2;
	xFilter[1][2] = yFilter[2][1] = 2;

	int size = xFilter.size() / 2;
	CImg<float> filteredImage(gFiltered._width - 2 * size, gFiltered._height - 2 * size, 1, 1);
	angles = filteredImage;

	for (int i = size; i < gFiltered._width - size; i++) {
		for (int j = size; j < gFiltered._height - size; j++) {
			/*计算梯度幅度gx,gy*/
			float sumX = 0, sumY = 0;
			for (int x = 0; x < xFilter.size(); x++) {
				for (int y = 0; y < yFilter.size(); y++) {
					sumX += xFilter[y][x] * (float)(gFiltered(i + x - size, j + y - size));
					sumY += yFilter[y][x] * (float)(gFiltered(i + x - size, j + y - size));
				}
			}
			if (sqrt(sumX * sumX + sumY * sumY) > 255) {
				filteredImage(i - size, j - size) = 255;
			}
			else {
				filteredImage(i - size, j - size) = sqrt(sumX * sumX + sumY * sumY);
			}

			/*计算梯度方向*/
			if (sumX == 0) {
				angles(i - size, j - size) = 90;
			}
			else {
				angles(i - size, j - size) = atan(sumY / sumX);
			}
		}
	}
	return filteredImage;
}

/*对梯度幅值图像应用非最大化抑制*/
CImg<float> Concon::nonMaxSupp(CImg<float>& sFiltered, CImg<float>& angles) {
	CImg<float> nms(sFiltered._width - 2, sFiltered._height - 2, 1, 1);
	for (int i = 1; i < sFiltered._width - 1; i++) {
		for (int j = 1; j < sFiltered._height - 1; j++) {
			float angle = angles(i, j);
			nms(i - 1, j - 1) = sFiltered(i, j);

			/*水平边缘*/
			if ((angle > -22.5 && angle <= 22.5) || (angle > 157.5 && angle <= -157.5)) {
				if (sFiltered(i, j) < sFiltered(i, j + 1) || sFiltered(i, j) < sFiltered(i, j - 1)) {
					nms(i - 1, j - 1) = 0;
				}
			}
			/*+45度边缘*/
			if ((angle > -67.5 && angle <= -22.5) || (angle > 112.5 && angle <= 157.5)) {
				if (sFiltered(i, j) < sFiltered(i - 1, j + 1) || sFiltered(i, j) < sFiltered(i + 1, j - 1)) {
					nms(i - 1, j - 1) = 0;
				}
			}
			/*垂直边缘*/
			if ((angle > -112.5 && angle <= -67.5) || (angle > 67.5 && angle <= 112.5)) {
				if (sFiltered(i, j) < sFiltered(i + 1, j) || sFiltered(i, j) < sFiltered(i - 1, j)) {
					nms(i - 1, j - 1) = 0;
				}
			}
			/*-45度边缘*/
			if ((angle > -157.5 && angle <= -112.5) || (angle > 22.5 && angle <= 67.5)) {
				if (sFiltered(i, j) < sFiltered(i + 1, j + 1) || sFiltered(i, j) < sFiltered(i - 1, j - 1)) {
					nms(i - 1, j - 1) = 0;
				}
			}
		}
	}
	return nms;
}

/*用双阈值处理和连接分析来检测并连接边缘*/
CImg<float> Concon::threshold(CImg<float>& img, int low, int high) {
	low = (low > 255) ? 255 : low;
	high = (high > 255) ? 255 : high;

	CImg<float> edgeMatch(img._width, img._height, 1, 1);
	for (int i = 0; i < img._width; i++) {
		for (int j = 0; j < img._height; j++) {
			edgeMatch(i, j) = img(i, j);
			if (edgeMatch(i, j) > high) {
				edgeMatch(i, j) = 255; // 如果高于高阈值，赋值为255
			}
			else if (edgeMatch(i, j) < low) {
				edgeMatch(i, j) = 0; // 如果低于低阈值，赋值为0
			}
			else {
				bool ifHigh = false, ifBetween = false;
				for (int x = i - 1; x < i + 2; x++) {
					for (int y = j - 1; y < j + 2; y++) {
						if (x > 0 && x <= edgeMatch._height && y > 0 && y <= edgeMatch._width) {
							if (edgeMatch(x, y) > high) {
								edgeMatch(i, j) = 255;
								ifHigh = true;
								break;
							}
							else if (edgeMatch(x, y) <= high && edgeMatch(x, y) >= low) {
								ifBetween = true;
							}
						}
					}
					if (ifHigh) {
						break;
					}
				}
				if (!ifHigh && ifBetween) {
					for (int x = i - 2; x < i + 3; x++) {
						for (int y = j - 1; y < j + 3; y++) {
							if (x > 0 && x <= edgeMatch._height && y > 0 && y <= edgeMatch._width) {

								if (edgeMatch(x, y) > high) {
									edgeMatch(i, j) = 255;
									ifHigh = true;
									break;
								}
							}
						}
						if (ifHigh) {
							break;
						}
					}
				}
				if (!ifHigh) {
					edgeMatch(i, j) = 0;
				}
			}
		}
	}
	return edgeMatch;
}

/*
 Canny edge detection with parameters passed in by user
   Params: grey - the greyscale image
           width, height - image dimensions
		   lowthreshold - default 2.5
		   highthreshold - default 7.5
		   gaussiankernelradius - radius of edge detection Gaussian, in standard deviations
		     (default 2.0)
		   gaussiankernelwidth - width of Gaussian kernel, in pixels (default 16)
		   contrastnormalised - flag to normalise image before edge detection (defualt 0)
   Returns: binary image with set pixels as edges

*/
void Concon::cannyparam(unsigned char *grey, int width, int height,
						  float lowthreshold, float highthreshold, 
						  float gaussiankernelradius, int gaussiankernelwidth,  
						  int contrastnormalised)
{
    int low, high;
	int err;
	int i;

	allocatebuffers(grey, width, height);
	if(!this->result)
		goto error_exit;
	if (contrastnormalised) 
		normalizeContrast(data, width, height);

	err = computeGradients(gaussiankernelradius, gaussiankernelwidth);
	if(err < 0)
	  goto error_exit;
    low = (int) (lowthreshold * MAGNITUDE_SCALE + 0.5f);
	high = (int) ( highthreshold * MAGNITUDE_SCALE + 0.5f);
	performHysteresis(low, high);
	for(i=0;i<width*height;i++)
		result[i] = idata[i] > 0 ? 1 : 0;
	
	killbuffers();
	return;
error_exit:
	killbuffers();
	return;
}


/*
  buffer allocation
*/
Concon* Concon::allocatebuffers(unsigned char *grey, int width, int height)
{
	if(!this)  
		goto error_exit;
	result = (unsigned char*)malloc(width * height);
	middleChar = (unsigned char*)malloc(width * height);
	middleInt = (int *)malloc(width * height * sizeof(int));
	middleFloat = (float*)malloc(width * height* sizeof(float));

	data = (unsigned char *)malloc(width * height);
	idata = (int *)malloc(width * height * sizeof(int));
	magnitude = (int *)malloc(width * height * sizeof(int));
	xConv = (float *)malloc(width * height * sizeof(float));
	yConv = (float *)malloc(width * height * sizeof(float));
	xGradient = (float *)malloc(width * height * sizeof(float));
	yGradient = (float *)malloc(width * height * sizeof(float));
	if(!data || !idata || !magnitude 
		|| !xConv || !yConv 
		|| !xGradient || !yGradient)
		 goto error_exit;

	memcpy(data, grey, width * height);
	this->width = width;
	this->height = height;

	return this;
error_exit:
	killbuffers();
	return 0;
}

/*
  buffers destructor
*/
void Concon::killbuffers()
{
	if(this)
	{
		free(data);
		free(idata);
		free(magnitude);
		free(xConv);
		free(yConv);
		free(xGradient);
		free(yGradient);
	}
}

/* NOTE: The elements of the method below (specifically the technique for
	non-maximal suppression and the technique for gradient computation)
	are derived from an implementation posted in the following forum (with the
	clear intent of others using the code):
	  http://forum.java.sun.com/thread.jspa?threadID=546211&start=45&tstart=0
	My code effectively mimics the algorithm exhibited above.
	Since I don't know the providence of the code that was posted it is a
	possibility (though I think a very remote one) that this code violates
	someone's intellectual property rights. If this concerns you feel free to
	contact me for an alternative, though less efficient, implementation.
	*/
	
int Concon::computeGradients(float kernelRadius, int kernelWidth)
{	
		float *kernel;
		float *diffKernel;
		int kwidth;

		int initX;
		int maxX;
		int initY;
		int maxY;

		int x, y;
		int i;
		int flag;

		kernel = (float *)malloc(kernelWidth * sizeof(float));
		diffKernel = (float *)malloc(kernelWidth * sizeof(float));
		if(!kernel || !diffKernel)
			goto error_exit;

		/* initialise the Gaussian kernel */
		for (kwidth = 0; kwidth < kernelWidth; kwidth++) 
		{
			float g1, g2, g3;
			g1 = gaussian((float) kwidth, kernelRadius);
			if (g1 <= GAUSSIAN_CUT_OFF && kwidth >= 2) 
				break;
			g2 = gaussian(kwidth - 0.5f, kernelRadius);
			g3 = gaussian(kwidth + 0.5f, kernelRadius);
			kernel[kwidth] = (g1 + g2 + g3) / 3.0f / (2.0f * (float) 3.14 * kernelRadius * kernelRadius);
			diffKernel[kwidth] = g3 - g2;
		}

		initX = kwidth - 1;
		maxX = width - (kwidth - 1);
		initY = width * (kwidth - 1);
		maxY = width * (height - (kwidth - 1));
		
		/* perform convolution in x and y directions */
		for(x = initX; x < maxX; x++) 
		{
			for(y = initY; y < maxY; y += width) 
			{
				int index = x + y;
				float sumX = data[index] * kernel[0];
				float sumY = sumX;
				int xOffset = 1;
				int yOffset = width;
				while(xOffset < kwidth) 
				{
					sumY += kernel[xOffset] * (data[index - yOffset] + data[index + yOffset]);
					sumX += kernel[xOffset] * (data[index - xOffset] + data[index + xOffset]);
					yOffset += width;
					xOffset++;
				}
				
				yConv[index] = sumY;
				xConv[index] = sumX;
			}
 
		}

		for (int index = 0; index<width*height; index++) //在高斯滤波图
			middleFloat[index] = hypotenuse(xConv[index], yConv[index]);
 
		for (x = initX; x < maxX; x++) 
		{
			for (y = initY; y < maxY; y += width) 
			{
				float sum = 0.0f;
				int index = x + y;
				for (i = 1; i < kwidth; i++)
					sum += diffKernel[i] * (yConv[index - i] - yConv[index + i]);
 
				xGradient[index] = sum;
			}
 
		}

		for(x = kwidth; x < width - kwidth; x++) 
		{
			for (y = initY; y < maxY; y += width) 
			{
				float sum = 0.0f;
				int index = x + y;
				int yOffset = width;
				for (i = 1; i < kwidth; i++) 
				{
					sum += diffKernel[i] * (xConv[index - yOffset] - xConv[index + yOffset]);
					yOffset += width;
				}
 
				yGradient[index] = sum;
			}
		}
		//for (int index = 0; index<width*height; index++) //梯度模图像
		//	middleFloat[index] = xGradient[index] + yGradient[index];
		//	middleFloat[index] = yGradient[index];  
		//	middleFloat[index] = hypotenuse(can->xGradient[index], can->yGradient[index]); // 不对 
 
		initX = kwidth;
		maxX = width - kwidth;
		initY = width * kwidth;
		maxY = width * (height - kwidth);
		for(x = initX; x < maxX; x++) 
		{
			for(y = initY; y < maxY; y += width) 
			{
				int index = x + y;
				int indexN = index - width;
				int indexS = index + width;
				int indexW = index - 1;
				int indexE = index + 1;
				int indexNW = indexN - 1;
				int indexNE = indexN + 1;
				int indexSW = indexS - 1;
				int indexSE = indexS + 1;
				
				float xGrad = xGradient[index];
				float yGrad = yGradient[index];
				float gradMag = hypotenuse(xGrad, yGrad);

				/* perform non-maximal supression */
				float nMag = hypotenuse(xGradient[indexN], yGradient[indexN]);
				float sMag = hypotenuse(xGradient[indexS], yGradient[indexS]);
				float wMag = hypotenuse(xGradient[indexW], yGradient[indexW]);
				float eMag = hypotenuse(xGradient[indexE], yGradient[indexE]);
				float neMag = hypotenuse(xGradient[indexNE], yGradient[indexNE]);
				float seMag = hypotenuse(xGradient[indexSE], yGradient[indexSE]);
				float swMag = hypotenuse(xGradient[indexSW], yGradient[indexSW]);
				float nwMag = hypotenuse(xGradient[indexNW], yGradient[indexNW]);
				float tmp;
				/*
				 * An explanation of what's happening here, for those who want
				 * to understand the source: This performs the "non-maximal
				 * supression" phase of the Canny edge detection in which we
				 * need to compare the gradient magnitude to that in the
				 * direction of the gradient; only if the value is a local
				 * maximum do we consider the point as an edge candidate.
				 * 
				 * We need to break the comparison into a number of different
				 * cases depending on the gradient direction so that the
				 * appropriate values can be used. To avoid computing the
				 * gradient direction, we use two simple comparisons: first we
				 * check that the partial derivatives have the same sign (1)
				 * and then we check which is larger (2). As a consequence, we
				 * have reduced the problem to one of four identical cases that
				 * each test the central gradient magnitude against the values at
				 * two points with 'identical support'; what this means is that
				 * the geometry required to accurately interpolate the magnitude
				 * of gradient function at those points has an identical
				 * geometry (upto right-angled-rotation/reflection).
				 * 
				 * When comparing the central gradient to the two interpolated
				 * values, we avoid performing any divisions by multiplying both
				 * sides of each inequality by the greater of the two partial
				 * derivatives. The common comparand is stored in a temporary
				 * variable (3) and reused in the mirror case (4).
				 * 
				 */
				flag = ( (xGrad * yGrad <= 0.0f) /*(1)*/
					? ffabs(xGrad) >= ffabs(yGrad) /*(2)*/
						? (tmp = ffabs(xGrad * gradMag)) >= ffabs(yGrad * neMag - (xGrad + yGrad) * eMag) /*(3)*/
							&& tmp > fabs(yGrad * swMag - (xGrad + yGrad) * wMag) /*(4)*/
						: (tmp = ffabs(yGrad * gradMag)) >= ffabs(xGrad * neMag - (yGrad + xGrad) * nMag) /*(3)*/
							&& tmp > ffabs(xGrad * swMag - (yGrad + xGrad) * sMag) /*(4)*/
					: ffabs(xGrad) >= ffabs(yGrad) /*(2)*/
						? (tmp = ffabs(xGrad * gradMag)) >= ffabs(yGrad * seMag + (xGrad - yGrad) * eMag) /*(3)*/
							&& tmp > ffabs(yGrad * nwMag + (xGrad - yGrad) * wMag) /*(4)*/
						: (tmp = ffabs(yGrad * gradMag)) >= ffabs(xGrad * seMag + (yGrad - xGrad) * sMag) /*(3)*/
							&& tmp > ffabs(xGrad * nwMag + (yGrad - xGrad) * nMag) /*(4)*/
					);
                if(flag)
				{
					magnitude[index] = (gradMag >= MAGNITUDE_LIMIT) ? MAGNITUDE_MAX : (int) (MAGNITUDE_SCALE * gradMag);
					/*NOTE: The orientation of the edge is not employed by this
					 implementation. It is a simple matter to compute it at
					this point as: Math.atan2(yGrad, xGrad); */
				} 
				else 
				{
					magnitude[index] = 0;
				}
			}
		}
		for (int index = 0; index<width*height; index++) //极大值抑制
			middleInt[index] = magnitude[index];

		free(kernel);
		free(diffKernel);
		return 0;
error_exit:
		free(kernel);
		free(diffKernel);
		return -1;
}
	
	/*
	  we follow edges. high gives the parameter for starting an edge,
	  how the parameter for continuing it.
	*/
void Concon::performHysteresis(int low, int high)
{
  int offset = 0;
  int x, y;
	
  memset(idata, 0, width * height * sizeof(int));
		
  for(y = 0; y < height; y++)
  {
    for(x = 0; x < width; x++)
	{
      if(idata[offset] == 0 && magnitude[offset] >= high) //!!!!
	    follow(x, y, offset, low);
	  offset++;
    }
  }
}
 
/*
	recursive portion of edge follower 
*/
void Concon::follow(int x1, int y1, int i1, int threshold)
{
  int x, y;
  int x0 = x1 == 0 ? x1 : x1 - 1;
  int x2 = x1 == width - 1 ? x1 : x1 + 1;
  int y0 = y1 == 0 ? y1 : y1 - 1;
  int y2 = y1 == height -1 ? y1 : y1 + 1;
		
  idata[i1] = magnitude[i1];  // !!!
  for (x = x0; x <= x2; x++) 
  {
    for (y = y0; y <= y2; y++) 
	{
      int i2 = x + y * width;
	  if ((y != y1 || x != x1) && idata[i2] == 0 && magnitude[i2] >= threshold)  // !!!
	    follow(x, y, i2, threshold);
    }
  }
}

void Concon::normalizeContrast(unsigned char *data, int width, int height)
{
	int histogram[256] = {0};
    int remap[256];
	int sum = 0;
    int j = 0;
	int k;
    int target;
    int i;

	for (i = 0; i < width * height; i++) 
			histogram[data[i]]++;
		
	
    for (i = 0; i < 256; i++) 
	{
			sum += histogram[i];
			target = (sum*255)/(width * height);
			for (k = j+1; k <= target; k++) 
				remap[k] = i;
			j = target;
	 }
		
    for (i = 0; i < width * height; i++) 
			data[i] = remap[data[i]];
}

float Concon::hypotenuse(float x, float y)
{
	return (float) sqrt(x*x +y*y);
}
 
float Concon::gaussian(float x, float tempSigma)
{
	return (float) exp(-(x * x) / (2.0f * tempSigma * tempSigma));
}
