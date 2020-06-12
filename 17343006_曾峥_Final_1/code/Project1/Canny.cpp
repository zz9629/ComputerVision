#include "Canny.h"
#include <iostream>
#include "CImg.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <algorithm>
#include <iomanip>
#define pi 3.1415926
#define M_PI 3.14159265358979323846
#define BOOSTBLURFACTOR 90.0
#define NOEDGE 255	//��ɫ
#define POSSIBLE_EDGE 128
#define EDGE 0		//��ɫ
using namespace std;
using namespace cimg_library;

Canny::Canny() {
	rows = 0;
	cols = 0;
	smoothedim = NULL;
	delta_x = NULL;  
	delta_y = NULL; 
	dirim = NULL;  
	magnitude = NULL; 
	nms = NULL; 
	edge = NULL;
}
Canny::Canny(CImg<float> img)
{
	this->img = img;
	rows = img.width();
	cols = img.height();
	delta_x = new int[rows * cols]; memset(delta_x, 0x0, rows * cols * sizeof(int));
	delta_y = new int[rows * cols]; memset(delta_y, 0x0, rows * cols * sizeof(int));
	dirim = new float[rows * cols]; memset(dirim, 0x0, rows * cols * sizeof(float));
	magnitude = new int[rows * cols]; memset(magnitude, 0x0, rows * cols * sizeof(int));
	nms = new int[rows * cols]; memset(nms, 0x0, rows * cols * sizeof(int));
	edge = new int[rows * cols]; memset(edge, 0x0, rows * cols * sizeof(int));
	smoothedim = new int[rows * cols];  memset(smoothedim, 0x0, rows * cols * sizeof(int));
}
Canny::Canny(char const* file) {
	CImg<unsigned char> pic(file);
	this->img = pic;
	rows = img.width();
	cols = img.height();
	delta_x = new int[rows*cols]; memset(delta_x, 0x0, rows*cols * sizeof(int));
	delta_y = new int[rows*cols]; memset(delta_y, 0x0, rows*cols * sizeof(int));
	dirim = new float[rows*cols]; memset(dirim, 0x0, rows*cols * sizeof(float));
	magnitude = new int[rows*cols]; memset(magnitude, 0x0, rows*cols * sizeof(int));
	nms = new int[rows*cols]; memset(nms, 0x0, rows*cols * sizeof(int));
	edge = new int[rows*cols]; memset(edge, 0x0, rows*cols * sizeof(int));
	smoothedim = new int[rows*cols];  memset(smoothedim, 0x0, rows*cols * sizeof(int));
}
Canny::~Canny() {
	delete[] delta_x;
	delete[] delta_y;
	delete[] dirim;
	delete[] magnitude;
	delete[] nms;
	delete[] edge;
	delete[] smoothedim;
}
//��ʾͼ��
void Canny::display()
{
	img.display();
}

CImg<unsigned char> Canny::canny_image()
{
	CImg<unsigned char> ret = canny_image(2.0, 0.25, 0.75);
	return ret;
}
CImg<unsigned char> Canny::canny_image(float sigma, float tlow, float thigh) {
	//RGBtoGray();
	gaussian_smooth(sigma);
	derrivative_x_y();
	radian_direction(-1, -1);
	magnitude_x_y();
	non_max_supp();
	apply_hysteresis(tlow, thigh);
//	txt_generate();
//	img_generate();

	CImg<unsigned char> pic2(rows, cols);
	pic2.fill(0);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (edge[i*cols + j] < 0)
				pic2(i, j) = 0;
			else if (edge[i*cols + j] > 255)
				pic2(i, j) = 255;
			else
				pic2(i, j) = edge[i*cols + j];
		}
	}
//canny���ս��ͼ
	return pic2;
}
//ѡ��������Ե��Ͻ��ľ�������
CImg<unsigned char> Canny::canny_line(CImg<int> picture, int distance) {
	CImg<int> pic = picture;
	//���ڼ���ĳһ�����ص��Ƿ�Ϊ��Ե��
	//�жϷ���Ϊ�鿴�������Ϊ���ĵİ������������ֻ��1�����ص�Ϊ0, ����7��Ϊ255���Ǳ�Ե��
	bool isEdge[1000][1000];
	cimg_forXY(pic, x, y) {
		isEdge[x][y] = false;
		if (x != rows - 1 && x != 0 && y != cols - 1 && y != 0 && pic(x, y) == 0) {
			int linyu[8];
			int m = 0;
			for (int i = x - 1; i <= x + 1; i++) {
				for (int j = y - 1; j <= y + 1; j++) {
					if (!(i == x && j == y)) {
						linyu[m] = pic(i, j);
						m++;
					}
				}
			}
			sort(linyu, linyu + 8);
			if (linyu[0] == 0 && linyu[1] == 255)
				isEdge[x][y] = true;
		}
	}
	cimg_forXY(pic, x, y) {
		if (x >= distance && x <= rows - 1 - distance && y >= distance && y <= cols - 1 - distance && isEdge[x][y] == true) {
			for (int i = x - distance; i <= x + distance; i++) {
				for (int j = y - distance; j <= y + distance; j++) {
					if (isEdge[i][j] == true) {
						pic = Draw_line(pic, x, y, i, j);
						isEdge[i][j] = false;
						isEdge[x][y] = false;
					}
				}
			}
		}
	}
	return pic;
}
//ɾ������С��20�ı�Ե��
CImg<unsigned char> Canny::delete_line(CImg<int> picture, int distance) {
	//���ڼ���ĳһ�����ص��Ƿ�Ϊ��Ե��
	//�жϷ���Ϊ�鿴�������Ϊ���ĵİ������������ֻ��1�����ص�Ϊ0, ����7��Ϊ255���Ǳ�Ե��
	CImg<int> pic = picture;
	bool isEdge[1000][1000];
	cimg_forXY(pic, x, y) {
		isEdge[x][y] = false;
		if (x != rows - 1 && x != 0 && y != cols - 1 && y != 0 && pic(x, y) == 0) {
			int linyu[8];
			int m = 0;
			for (int i = x - 1; i <= x + 1; i++) {
				for (int j = y - 1; j <= y + 1; j++) {
					if (!(i == x && j == y)) {
						linyu[m] = pic(i, j);
						m++;
					}
				}
			}
			sort(linyu, linyu + 8);
			if (linyu[0] == 0 && linyu[1] == 255)
				isEdge[x][y] = true;
			//ɾ�����������ĵ�
			if (linyu[0] == 255)
				pic(x, y) = 255;
		}
	}
	//ɾ����������20������
	//�ж���������߽��ľ���С��20����ɾ���������߽����ɵľ��������кڵ㣬�����Ļ���ʹ�����߽��ֱ�������ֱ�ߵĻ�Ҳ����ν
	//�����������Ļ������߽��֮�䶼�ǰ�ɫ����ɾ��Ҳ����ν
	cimg_forXY(pic, x, y) {
		if (isEdge[x][y] == true) {
			int begin_x = x - distance > 0 ? x - distance : 0;
			int begin_y = y - distance > 0 ? y - distance : 0;
			int end_x = x + distance < rows - 1 ? x + distance : rows - 1;
			int end_y = y + distance < cols - 1 ? y + distance : cols - 1;
			for (int i = begin_x; i <= end_x; i++) {
				for (int j = begin_y; j <= end_y; j++) {
					if (isEdge[i][j] == true) {
						int max_x = x >= i ? x : i;
						int max_y = y >= j ? y : j;
						int min_x = max_x == x ? i : x;
						int min_y = max_y == y ? j : y;
						for (int ii = min_x; ii <= max_x; ii++) {
							for (int jj = min_y; jj <= max_y; jj++) {
								pic(ii, jj) = 255;
							}
						}
						isEdge[i][j] = false;
						isEdge[x][y] = false;
					}
				}
			}
		}
	}
	//ɾ��������һ����������ܴ��ڵĵ���������������
	cimg_forXY(pic, x, y) {
		if (x != rows - 1 && x != 0 && y != cols - 1 && y != 0 && pic(x, y) == 0) {
			int linyu[8];
			int m = 0;
			for (int i = x - 1; i <= x + 1; i++) {
				for (int j = y - 1; j <= y + 1; j++) {
					if (!(i == x && j == y)) {
						linyu[m] = pic(i, j);
						m++;
					}
				}
			}
			sort(linyu, linyu + 8);
			if (linyu[0] == 255)
				pic(x, y) = 255;
		}
	}
	return pic;
}

//�����еĺ���
//��ÿ����תΪ��ɫ
void Canny::RGBtoGray() {
	//����һ��width *height = rows * cols��С��ͼ��depthΪ1, 1��spectrum = channel
	//���ù�ʽת��
	CImg<unsigned char> grayed = CImg<int>(this->rows, this->cols, 1, 1);
	cimg_forXY(this->img, x, y)
	{
		int b = img(x, y, 0);	
		int g = img(x, y, 1);	
		int r = img(x, y, 2);	
		double newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);
		grayed(x, y) = newValue;
	}
//	grayed.display();
	img = grayed;
}
//��˹����
/*	
 *	using 1-dimensional gaussian smoothing kernel.
 *  @parameter sigma:     ��˹��������
 *  @parameter Kernel:    ��˹�˺���ģ��
 *  @parameter WidowSize:  ��˹ģ���С
 *  @parameter rows��cols��		���˽�б���imgͼ�������
 *  
 */
void Canny::gaussian_smooth(float sigma)
{
	float *tempim = new float[rows*cols];	//��˹ģ��,һά
	int r, c, rr, cc,
		windowsize,  //��˹�˲�������
		center;      //�����ĵ� = windowsize/2
	float *kernel,
		dot,		//��˹ϵ����ͼ�����ݵĵ��
		sum;		//�˲�ϵ���ܺͣ�������һ��
	//����һά��˹���� Create a 1-dimensional gaussian smoothing kernel.
	make_gaussian_kernel(sigma, &kernel, &windowsize);//��ַΪ����
	center = windowsize / 2;
	//���캯������Ϊkernel���ռ�
	//Blur in the x - direction.
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			dot = 0.0;
			sum = 0.0;
			for (cc = (-center); cc <= center; cc++) {
				if (((c + cc) >= 0) && ((c + cc) < cols)) {
					dot += (float)img(r, c + cc) * kernel[center + cc];
					sum += kernel[center + cc];
				}
			}
			tempim[r*cols + c] = (dot / sum);
		}
	}	
	//Blur in the y - direction.
	for (c = 0; c < cols; c++) {
		for (r = 0; r < rows; r++) {
			sum = 0.0;
			dot = 0.0;
			for (rr = (-center); rr <= center; rr++) {
				//�ж��Ƿ���ͼ���ڲ�
				if (((r + rr) >= 0) && ((r + rr) < rows)) {
					dot += tempim[(r + rr)*cols + c] * kernel[center + rr];
					sum += kernel[center + rr];
				}
			}
			smoothedim[r*cols + c] = (short int)(dot*BOOSTBLURFACTOR / sum + 0.5);
		}
	}
}
//����x,y�����һ�׵���
void Canny::derrivative_x_y() {
	int r = 0, c = 0, pos = 0;
	//����x�����һ�׵������жϱ߽������ʧ�߽����ص�
	for (r = 0; r < rows; r++) {
		pos = r * cols;
		delta_x[pos] = smoothedim[pos + 1] - smoothedim[pos];
		pos++;
		for (c = 1; c < (cols - 1); c++, pos++) {
			delta_x[pos] = smoothedim[pos + 1] - smoothedim[pos - 1];
		}
		delta_x[pos] = smoothedim[pos] - smoothedim[pos - 1];
	}
	//����y�����һ�׵������жϱ߽������ʧ�߽����ص�
	for (c = 0; c < cols; c++) {
		pos = c;
		delta_y[pos] = smoothedim[pos + cols] - smoothedim[pos];
		pos += cols;
		for (r = 1; r < (rows - 1); r++, pos += cols) {
			delta_y[pos] = smoothedim[pos + cols] - smoothedim[pos - cols];
		}
		delta_y[pos] = smoothedim[pos] - smoothedim[pos - cols];
	}
}
//�����ݶ����ϵķ�������x��Ϊ��ʱ�뷽��ָ���Ļ���
/*******************************************************************************
* Procedure: radian_direction
* Purpose: To compute a direction of the gradient image from component dx and
* dy images. Because not all derriviatives are computed in the same way, this
* code allows for dx or dy to have been calculated in different ways.
*
* FOR X:  xdirtag = -1  for  [-1 0  1]
*         xdirtag =  1  for  [ 1 0 -1]
*
* FOR Y:  ydirtag = -1  for  [-1 0  1]'
*         ydirtag =  1  for  [ 1 0 -1]'
*
* The resulting angle is in radians measured counterclockwise from the
* xdirection. The angle points "up the gradient".
*******************************************************************************/
void Canny::radian_direction(int xdirtag, int ydirtag)
{
	double dx = 0.0, dy = 0.0;
	int r = 0, c = 0, pos = 0;
	for (r = 0, pos = 0; r < rows; r++) {
		for (c = 0; c < cols; c++, pos++) {
			dx = (double)delta_x[pos];
			dy = (double)delta_y[pos];

			if (xdirtag == 1) dx = -dx;
			if (ydirtag == -1) dy = -dy;

			dirim[pos] = (float)angle_radians(dx, dy);
		}
	}
}
//�����ݶȵķ�ֵ
void Canny::magnitude_x_y()
{
	int r = 0, c = 0, pos = 0, sq1 = 0, sq2 = 0;
	for (r = 0, pos = 0; r < rows; r++) {
		for (c = 0; c < cols; c++, pos++) {
			sq1 = (int)delta_x[pos] * (int)delta_x[pos];
			sq2 = (int)delta_y[pos] * (int)delta_y[pos];
			magnitude[pos] = (short)(0.5 + sqrt((float)sq1 + (float)sq2));	
		}
	}
}
//���зǼ���ֵ����
void Canny::non_max_supp() {
	int rowcount = 0, colcount = 0, count = 0;
	int *magrowptr, *magptr;
	int *gxrowptr, *gxptr;
	int *gyrowptr, *gyptr, z1 = 0, z2 = 0;
	int m00, gx = 0, gy = 0;
	float mag1 = 0.0, mag2 = 0.0, xperp = 0.0, yperp = 0.0;
	int *resultrowptr, *resultptr;
	//Zero the edges of the result image.
	for (count = 0, resultrowptr = nms, resultptr = nms + cols * (rows - 1);
		count < cols; resultptr++, resultrowptr++, count++) {
		*resultrowptr = *resultptr = 0;
	}

	for (count = 0, resultptr = nms, resultrowptr = nms + cols - 1;
		count < rows; count++, resultptr += cols, resultrowptr += cols) {
		*resultptr = *resultrowptr = 0;
	}
	//Suppress non-maximum points.
	for (rowcount = 1, magrowptr = magnitude + cols + 1, gxrowptr = delta_x + cols + 1,
		gyrowptr = delta_y + cols + 1, resultrowptr = nms + cols + 1;
		rowcount < rows - 2;
		rowcount++, magrowptr += cols, gyrowptr += cols, gxrowptr += cols,
		resultrowptr += cols) {
		for (colcount = 1, magptr = magrowptr, gxptr = gxrowptr, gyptr = gyrowptr,
			resultptr = resultrowptr; colcount < cols - 2;
			colcount++, magptr++, gxptr++, gyptr++, resultptr++) {
			m00 = *magptr;
			if (m00 == 0) {
				*resultptr = NOEDGE;
			}
			else {
				xperp = -(gx = *gxptr) / ((float)m00);
				yperp = (gy = *gyptr) / ((float)m00);
			}

			if (gx >= 0) {
				if (gy >= 0) {
					if (gx >= gy)
					{
						/* 111 */
						/* Left point */
						z1 = *(magptr - 1);
						z2 = *(magptr - cols - 1);

						mag1 = (m00 - z1)*xperp + (z2 - z1)*yperp;

						/* Right point */
						z1 = *(magptr + 1);
						z2 = *(magptr + cols + 1);

						mag2 = (m00 - z1)*xperp + (z2 - z1)*yperp;
					}
					else
					{
						/* 110 */
						/* Left point */
						z1 = *(magptr - cols);
						z2 = *(magptr - cols - 1);

						mag1 = (z1 - z2)*xperp + (z1 - m00)*yperp;

						/* Right point */
						z1 = *(magptr + cols);
						z2 = *(magptr + cols + 1);

						mag2 = (z1 - z2)*xperp + (z1 - m00)*yperp;
					}
				}
				else
				{
					if (gx >= -gy)
					{
						/* 101 */
						/* Left point */
						z1 = *(magptr - 1);
						z2 = *(magptr + cols - 1);

						mag1 = (m00 - z1)*xperp + (z1 - z2)*yperp;

						/* Right point */
						z1 = *(magptr + 1);
						z2 = *(magptr - cols + 1);

						mag2 = (m00 - z1)*xperp + (z1 - z2)*yperp;
					}
					else
					{
						/* 100 */
						/* Left point */
						z1 = *(magptr + cols);
						z2 = *(magptr + cols - 1);

						mag1 = (z1 - z2)*xperp + (m00 - z1)*yperp;

						/* Right point */
						z1 = *(magptr - cols);
						z2 = *(magptr - cols + 1);

						mag2 = (z1 - z2)*xperp + (m00 - z1)*yperp;
					}
				}
			}
			else
			{
				if ((gy = *gyptr) >= 0)
				{
					if (-gx >= gy)
					{
						/* 011 */
						/* Left point */
						z1 = *(magptr + 1);
						z2 = *(magptr - cols + 1);

						mag1 = (z1 - m00)*xperp + (z2 - z1)*yperp;

						/* Right point */
						z1 = *(magptr - 1);
						z2 = *(magptr + cols - 1);

						mag2 = (z1 - m00)*xperp + (z2 - z1)*yperp;
					}
					else
					{
						/* 010 */
						/* Left point */
						z1 = *(magptr - cols);
						z2 = *(magptr - cols + 1);

						mag1 = (z2 - z1)*xperp + (z1 - m00)*yperp;

						/* Right point */
						z1 = *(magptr + cols);
						z2 = *(magptr + cols - 1);

						mag2 = (z2 - z1)*xperp + (z1 - m00)*yperp;
					}
				}
				else
				{
					if (-gx > -gy)
					{
						/* 001 */
						/* Left point */
						z1 = *(magptr + 1);
						z2 = *(magptr + cols + 1);

						mag1 = (z1 - m00)*xperp + (z1 - z2)*yperp;

						/* Right point */
						z1 = *(magptr - 1);
						z2 = *(magptr - cols - 1);

						mag2 = (z1 - m00)*xperp + (z1 - z2)*yperp;
					}
					else
					{
						/* 000 */
						/* Left point */
						z1 = *(magptr + cols);
						z2 = *(magptr + cols + 1);

						mag1 = (z2 - z1)*xperp + (m00 - z1)*yperp;

						/* Right point */
						z1 = *(magptr - cols);
						z2 = *(magptr - cols - 1);

						mag2 = (z2 - z1)*xperp + (m00 - z1)*yperp;
					}
				}
			}

			/* Now determine if the current point is a maximum point */

			if ((mag1 > 0.0) || (mag2 > 0.0))
			{
				*resultptr = NOEDGE;
			}
			else
			{
				if (mag2 == 0.0)
					*resultptr = NOEDGE;
				else
					*resultptr = POSSIBLE_EDGE;
			}
		}
	}
}
//˫��ֵ���
void Canny::apply_hysteresis(float tlow, float thigh) {
	int r = 0, c = 0, pos = 0, numedges = 0, lowcount = 0, highcount = 0, lowthreshold = 0, highthreshold = 0,
		i = 0, *hist, rr = 0, cc = 0;
	hist = new int[32768];
	int maximum_mag = 0, sumpix = 0;
	for (r = 0, pos = 0; r < rows; r++) {
		for (c = 0; c < cols; c++, pos++) {
			if (nms[pos] == POSSIBLE_EDGE) edge[pos] = POSSIBLE_EDGE;
			else edge[pos] = NOEDGE;
		}
	}
	/****************************************************************************
   * Initialize the edge map to possible edges everywhere the non-maximal
   * suppression suggested there could be an edge except for the border. At
   * the border we say there can not be an edge because it makes the
   * follow_edges algorithm more efficient to not worry about tracking an
   * edge off the side of the image.
   ****************************************************************************/
	for (r = 0, pos = 0; r < rows; r++, pos += cols) {
		edge[pos] = NOEDGE;
		edge[pos + cols - 1] = NOEDGE;
	}
	pos = (rows - 1) * cols;
	for (c = 0; c < cols; c++, pos++) {
		edge[c] = NOEDGE;
		edge[pos] = NOEDGE;
	}
	/****************************************************************************
	* Compute the histogram of the magnitude image. Then use the histogram to
	* compute hysteresis thresholds.
	****************************************************************************/
	for (r = 0; r < 32768; r++) hist[r] = 0;
	for (r = 0, pos = 0; r < rows; r++) {
		for (c = 0; c < cols; c++, pos++) {
			if (edge[pos] == POSSIBLE_EDGE) hist[magnitude[pos]]++;
		}
	}
	/****************************************************************************
   * Compute the number of pixels that passed the nonmaximal suppression.
   ****************************************************************************/
	for (r = 1, numedges = 0; r < 32768; r++) {
		if (hist[r] != 0) maximum_mag = r;
		numedges += hist[r];
	}

	highcount = (int)(numedges * thigh + 0.5);

	/****************************************************************************
   * Compute the high threshold value as the (100 * thigh) percentage point
   * in the magnitude of the gradient histogram of all the pixels that passes
   * non-maximal suppression. Then calculate the low threshold as a fraction
   * of the computed high threshold value. John Canny said in his paper
   * "A Computational Approach to Edge Detection" that "The ratio of the
   * high to low threshold in the implementation is in the range two or three
   * to one." That means that in terms of this implementation, we should
   * choose tlow ~= 0.5 or 0.33333.
   ****************************************************************************/
	r = 1;
	numedges = hist[1];
	while ((r < (maximum_mag - 1)) && (numedges < highcount)) {
		r++;
		numedges += hist[r];
	}
	highthreshold = r;
	lowthreshold = (int)(highthreshold * tlow + 0.5);

	/****************************************************************************
  * This loop looks for pixels above the highthreshold to locate edges and
  * then calls follow_edges to continue the edge.
  ****************************************************************************/
	for (r = 0, pos = 0; r < rows; r++) {
		for (c = 0; c < cols; c++, pos++) {
			if ((edge[pos] == POSSIBLE_EDGE) && (magnitude[pos] >= highthreshold)) {
				edge[pos] = EDGE;
				follow_edges((edge + pos), (magnitude + pos), lowthreshold, cols);
			}
		}
	}
	/****************************************************************************
  * Set all the remaining possible edges to non-edges.
  ****************************************************************************/
	for (r = 0, pos = 0; r < rows; r++) {
		for (c = 0; c < cols; c++, pos++) if (edge[pos] != EDGE) edge[pos] = NOEDGE;
	}
	delete[] hist;
}

//��������
/*
 *	creating 1-dimensional gaussian smoothing kernel.
 *  @parameter center:     �������ĵ�
 *  @parameter Kernel:    ��˹�˺���ģ��
 *  @parameter WidowSize:  ��˹ģ���С
 
*/
void Canny::make_gaussian_kernel(float sigma, float **kernel, int *windowsize) {
	int i = 0, center = 0;
	float x, fx, sum = 0.0;
	//���ݸ�˹�˲��˵ķ�������˹�˵Ŀ��//�õ�ģ�崰�ڴ�С
	*windowsize = 1 + 2 * ceil(2.5 * sigma);//sigma�õ����ڴ�С��// [-3*sigma,3*sigma] �������ݣ��Ḳ�Ǿ��󲿷��˲�ϵ��
											   //ԭ������Ϊ��˹�ֲ�����ֵ�ڣ��̡�3��,��+3��)�ĸ���Ϊ0.9974����ѡ��3*3

	//*kernel = (float*)calloc((*windowsize), sizeof(float));
	center = (*windowsize) / 2;
	*kernel = new float[*windowsize];
	for (i = 0; i < (*windowsize); i++) {
		x = (float)(i - center);			//������һ�㵽���ĵ����
		fx = pow(2.71828, -0.5*x*x / (sigma*sigma)) / (sigma * sqrt(6.2831853));//��ʽ��6.28��2pai
		(*kernel)[i] = fx;
		sum += fx;
	}
	//��һ��
	for (i = 0; i < (*windowsize); i++) {
		(*kernel)[i] /= sum;
	}
	//test
	{
		printf("The filter coefficients are:\n");
		for (i = 0; i < (*windowsize); i++)
			printf("kernel[%d] = %f\n", i, (*kernel)[i]);
		cout << "windowsize: " << (*windowsize) << endl;
	}
	
}
void Canny::follow_edges(int *edgemapptr, int *edgemagptr, int lowval, int cols)
{
	int *tempmagptr;
	int *tempmapptr;
	int i;
	float thethresh;
	int x[8] = { 1,1,0,-1,-1,-1,0,1 },
		y[8] = { 0,1,1,1,0,-1,-1,-1 };

	for (i = 0; i < 8; i++) {
		tempmapptr = edgemapptr - y[i] * cols + x[i];
		tempmagptr = edgemagptr - y[i] * cols + x[i];

		if ((*tempmapptr == POSSIBLE_EDGE) && (*tempmagptr > lowval)) {
			*tempmapptr = EDGE;
			follow_edges(tempmapptr, tempmagptr, lowval, cols);
		}
	}
}
double Canny::angle_radians(double x, double y) {
	double xu = 0.0, yu = 0.0, ang = 0.0;
	xu = fabs(x);
	yu = fabs(y);
	if ((xu == 0) && (yu == 0)) return(0);
	ang = atan(yu / xu);
	if (x >= 0) {
		if (y >= 0) return (ang);
		else return(2 * M_PI - ang);
	}
	else {
		if (y >= 0) return (M_PI - ang);
		else return(M_PI + ang);
	}
}
CImg<unsigned char> Canny::Draw_line(CImg<int> tmp, int x, int y, int x1, int y1) {

	CImg <unsigned char> TempImg = tmp;
	int black[] = { 0,0,0 };
	TempImg.draw_line(x, y, x1, y1, black);
	return TempImg;
}
void Canny::txt_generate()
{
	//magnitude
	ofstream fout("magnitude.txt", ios::out);
	//�ļ����
	int r = 0, c = 0;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			fout << setiosflags(ios::fixed) << setprecision(6) << magnitude[r*cols + c] << "   ";
		}
		fout << endl;
	}
	if (!fout) {
		cout << "Close file...\n";
		fout.close();// //�������ǰ�������ǹر���ǰ�򿪹����ļ�
	}
	//nms
	ofstream fout2("nms.txt", ios::out);
	//�ļ����
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			fout2 << setiosflags(ios::fixed) << setprecision(6) << nms[r*cols + c] << "   ";
		}
		fout2 << endl;
	}
	if (!fout2) {
		cout << "Close file...\n";
		fout2.close();// //�������ǰ�������ǹر���ǰ�򿪹����ļ�
	}
	//edge
	ofstream fout3("edge.txt", ios::out);
	//�ļ����
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			fout3 << setiosflags(ios::fixed) << setprecision(6) << edge[r*cols + c] << "   ";
		}
		fout3 << endl;
	}
	if (!fout3) {
		cout << "Close file...\n";
		fout3.close();// //�������ǰ�������ǹر���ǰ�򿪹����ļ�
	}
}
void Canny::img_generate()
{
	//	CImg<unsigned char> pic2(rows, cols, 1, 1, 5);
	
	//delta_x
	CImg<unsigned char> pic(rows, cols);
	pic.fill(0);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (delta_x[i*cols + j] < 0)
				pic(i, j) = 0;
			else if (delta_x[i*cols + j] > 255)
				pic(i, j) = 255;
			else
				pic(i, j) = delta_x[i*cols + j];
		}
	}
	//pic.display("delta_x");
	pic.save("delta_x.bmp");

	//delta_y
	CImg<unsigned char> pic2(rows, cols);
	pic2.fill(0);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (delta_y[i*cols + j] < 0)
				pic2(i, j) = 0;
			else if (delta_y[i*cols + j] > 255)
				pic2(i, j) = 255;
			else
				pic2(i, j) = delta_y[i*cols + j];
		}
	}
	//pic.display("delta_y");
	pic2.save("delta_y.bmp");

	//magnitude
	CImg<unsigned char> pic3(rows, cols);
	pic3.fill(0);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (magnitude[i*cols + j] < 0)
				pic3(i, j) = 0;
			else if (magnitude[i*cols + j] > 255)
				pic3(i, j) = 255;
			else
				pic3(i, j) = magnitude[i*cols + j];
		}
	}
	//pic.display("magnitude");
	pic3.save("magnitude.bmp");

	//nms
	CImg<unsigned char> pic4(rows, cols);
	pic4.fill(0);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (nms[i*cols + j] < 0)
				pic4(i, j) = 0;
			else if (nms[i*cols + j] > 255)
				pic4(i, j) = 255;
			else
				pic4(i, j) = nms[i*cols + j];
		}
	}
//	pic2.display("nms");
	pic4.save("nms.bmp");

}