/*
ԭ������ɢ�����ݶȱƽ��������ݶ�ά�ҶȾ����ݶ�������Ѱ��ͼ��ҶȾ���
	�ĻҶ�Ծ��λ�ã�Ȼ����ͼ���н���Щ�����������γ���ͼ��ı�Ե
	һ���ͼ�����canny��Ե���Ҫ�������¼�����
	1.��ͼ����лҶȻ�����Gimg = 0.299R+0.587G+0.114B
	2.��ͼ����и�˹�˲�(��ȥͼƬ�������Ա�Ե����Ӱ��)
		���ȵ���make_gaussian_kernel���ɾ���ˣ��ٶ�ͼ����о��
	3.��һ��ƫ�������޲���������ݶȵķ�ֵ�ͷ���
	4.���ݶȷ�ֵ���зǼ���ֵ����
	5.˫��ֵ�������ӱ�Ե
*/
#ifndef CANNY_H_	//���û�������
#define CANNY_H_	//���������
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
	//Ĭ�ϲ�������,��˹�˲���׼��Ϊ2.0������ֵΪ0.25������ֵΪ0.75
	CImg<unsigned char> canny_image();
	//�������л�ȡ���ı�Եͼ,sigma��ʾ��˹�˲��Ĳ�����tlow��thighΪ������ֵ //���õ�private�ĸ�������
	CImg<unsigned char> canny_image(float sigma, float tlow, float thigh);
	//ѡ��������Ե��Ͻ��ľ�������
	CImg<unsigned char> canny_line(CImg<int> picture, int distance);
	//ɾ������С��20�ı�Ե��
	CImg<unsigned char> delete_line(CImg<int> picture, int);

private:
//����
	CImg<unsigned char> img;
	int rows;
	int cols;
	int *smoothedim;	//* The image after gaussian smoothing.      */
	int *delta_x;		//x�����һ�׵���	The first devivative image, x-direction
	int *delta_y;		//y�����һ�׵���	The first derivative image, y-direction.
	float *dirim;		//�ݶȵķ���
	int *magnitude;		//�ݶȵķ�ֵ		The magnitude of the gadient image.
	int *nms;			//�Ǽ���ֵ���ƺ�õ�����Points that are local maximal magnitude.
	int *edge;			//��Ե����
	//

//canny��������Ҫ����
	// 1.��ͼ����лҶȻ�������ÿ����תΪ��ɫ,			phase1
	void RGBtoGray();
	// 2.��ͼ����и�˹�˲�(��ȥͼƬ�������Ա�Ե����Ӱ��)	phase2
	void gaussian_smooth(float sigma);
	// 3.����x,y�����һ�׵���								phase3
	void derrivative_x_y();
	//	�����ݶ����ϵķ�������x��Ϊ��ʱ�뷽��ָ���Ļ���
	void radian_direction(int xdirtag, int ydirtag);
	// 4.�����ݶȵķ�ֵ										
	void magnitude_x_y();
	// 5.���зǼ���ֵ����									phase4
	//This routine applies non-maximal suppression to the magnitude of the gradient image.
	void non_max_supp();
	// 6.˫��ֵ���	This routine finds edges that are above some high threshhold or
		//are connected to a high pixel by a path of pixels greater than a low
	void apply_hysteresis(float tlow, float thigh);

//��������
	//Create a one dimensional gaussian kernel.
	void make_gaussian_kernel(float sigma, float **kernel, int *windowsize);	//2�ĸ�������
	//This procedure edges is a recursive routine that traces edgs along
		//all paths whose magnitude values remain above some specifyable lower
	void follow_edges(int *edgemapptr, int *edgemagptr, int lowval, int cols);	//4 �Ǽ���ֵ���� ����
	double angle_radians(double x, double y);									//3 ����
	CImg<unsigned char> Draw_line(CImg<int> tmp, int x, int y, int x1, int y1);	//ɾ��С��20�� ����
	//дTXT�ļ���magnitude��nms��edge
	void txt_generate();
	//�����м��ļ������ڲ���
	void img_generate();
};

#endif
