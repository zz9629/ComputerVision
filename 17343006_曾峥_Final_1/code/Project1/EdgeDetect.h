/*
	1.��ͼ����лҶȻ�����Gimg = 0.299R+0.587G+0.114B
	2.��ͼ������²������ֱ��ʸߵ�ͼ���С�����Ժ���������ͼ���õ�4������֮�������ͼ���ϡ�
	3.Canny�㷨��ȡ��Ե���õ��ı�Ե������Ҳ�������
	4.Hough�任������ͼ��õ�4�����㣩
		��1����ImageSpace��HoughSpace�������꣩�任������votingͶƱ�������ۼӣ�
		��2�����ۼӾ���ȡ����һЩֵ��������Եֱ��
		��3����ÿ����ֱ��֮�佻�㣬���ۼƸý����ֱ�߽������
		��4��ȡ�����������4���㣬��ΪA4ֽ���ĸ�����
	5.��Hough�任�õ���4������ŵ�ԭͼ�У�����4�����㼰��Ե
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
#include <sstream>	//ʹ��stringstream��Ҫ�������ͷ�ļ�
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
	int a, b;	//Բ��
	int r;		//�뾶
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
	void edgeDetect(string, string, string, int);// ����ͼ�񣬼�����
private:
	//ֱ����غ���
	void lineDetect(string, string, int);
	CImg<float> Binarization(CImg<float> srcImg, float alpha);
	CImg<float> RGBtoGray(CImg<float> img);				//�Ҷȴ���
	CImg<float> downSampling(CImg<float> img, int num);	//�²�������numΪ��С�ı���
	CImg<float> canny(CImg<float> img);					//canny��ȡ��Ե
	CImg<float> houghTransform(CImg<float>);		//Hough�任
	void houghReCompute(CImg<float>&);				//Hough�߷���ȡ
	int getMaxHough(CImg<float>& img, int& size, int& y, int& x); //��������
	void findEdge(CImg<float>&);						// ����Ե
	void findVertex();									// ���ǵ�
	void drawVertexLines(CImg<float> &, int);			//�������㵽��Ե


	CImg<float> img; // ԭͼ��
	CImg<float> imgGrayed; // �Ҷ�ͼ��
	CImg<float> imgDowned;	//�²��������ͼ������canny����������Ե
	CImg<float> imgCannyed; // ������ֵ������ͼ��
	CImg<float> houghImage; // ����ռ�ͼ��
	CImg<float> outputImage; // ����任��������ͼ��
	vector<float> setSin; // sin����
	vector<float> setCos; // cos����

	int pointNumber;		// �ǵ���
	CImg<float> edge;		// ��Եֱ��
	vector<Line*> myLines;	// ������ֱ��
	vector<Vertex*> myVertexs;	//������������4�����㣬���ɶ��㻭��ֱ��
	
	void test();			//���Ժ�����������׶ε�ͼ��
	map<int, vector<Plot>> blocks;			//��ͨ�飨label���㼯��
	string name;

};
#endif