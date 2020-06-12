#ifndef _IMGSEG_H
#define _IMGSEG_H

#include "CImg.h"

#include <iostream>
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

using namespace cimg_library;
using namespace std;

#define min(a,b) (((a)<(b))?(a):(b))

struct Plot {
	int x, y;
	Plot(int x_, int y_) :x(x_), y(y_) {}
};
struct Block {
	int label;
	vector<Plot> Plots;
	int x_low, x_high, y_low, y_high;//��߽�

	Block(int _label) :label(_label) {
		Plots.clear();
	}
};


class ImgSeg {
public:
	ImgSeg(){}
	ImgSeg(CImg<float>);
	~ImgSeg();
	CImg<float> deleteTool();

	void Binarization(string);					//�Ҷȴ�����ֵ����
	void Binarization(string, float);
	void cptConBlocks();
	void deleteBlocks();

	CImg<float> RGBtoGray(CImg<float>);
	CImg<float> BinSimple(CImg<float>, int);	//����ֵ������ֵ��
	int otsu(CImg<float>& image, float);
	CImg<float> converse(CImg<float>);
	void normalizedHistogram(CImg<int> img, double* hist);
	void fillRunVectors(CImg<int>& bwImage, int& NumberOfRuns,
		vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun);
	void firstPass(vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun, int NumberOfRuns,
		vector<int>& runLabels, vector<pair<int, int>>& equivalences, int offset);
	void replaceSameLabel(vector<int>& runLabels, vector<pair<int, int>>& equivalence);

	bool isValid(map<int, vector<Plot> >::iterator iter);


	CImg<float> imgOriginal;//ԭͼ
	CImg<float> img;		//��ֵͼ
	CImg<float> imgForCut;	//���ڼ����м���������ͼ��

	//key��label,values�ǵ㼯��
	map<int, vector<Plot>> blocks;			//��ͨ�飨label���㼯��

};

#endif  

