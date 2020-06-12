#ifndef _IMGSEG_H
#define _IMGSEG_H

#include "CImg.h"
#include "Tool.h"
#include "SVM.h"

#include <iostream>
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
	int x_low, x_high, y_low, y_high;//块边界

	Block(int _label) :label(_label) {
		Plots.clear();
	}
};
struct Group {
	bool isValid;
	vector<int> boundry;
	vector<vector<int>> blocks;
	Group(bool v, vector<int> _boundry, vector<vector<int>> _blocks) {
		isValid = v;
		boundry = _boundry;
		blocks = _blocks;
	}
};

class ImgSeg {
public:
	ImgSeg() {}
	~ImgSeg();
	void findTarget(string, string, string, string, int, int);//每个图像调用一次
	
private:
	void reset();
	void Binarization(string);					//灰度处理，二值处理
	void Binarization(string, float);
	void cptConBlocks();
	void deleteBlocks();
	int findGroup(int, int, int, int);
	bool pushToGroup(Group&, int x_low, int x_high, int y_low, int y_high);
	bool addNewGroup(int x_low, int x_high, int y_low, int y_high);

	void deleteGroups();
	bool isOverLap(Group, Group);
	bool isClose(Group, Group, int);
	void combiGroups();
	void getImg(CImg<float> & );


	void getImgFiles(vector<vector<int> >& vc, int state);

	CImg<float> RGBtoGray(CImg<float>);
	CImg<float> BinSimple(CImg<float>, int);	//简单阈值处理，二值化
	int otsu(CImg<float>& image, float);
	CImg<float> converse(CImg<float>);
	void normalizedHistogram(CImg<int> img, double* hist);
	void fillRunVectors(CImg<int>& bwImage, int& NumberOfRuns,
		vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun);
	void firstPass(vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun, int NumberOfRuns,
		vector<int>& runLabels, vector<pair<int, int>>& equivalences, int offset);
	void replaceSameLabel(vector<int>& runLabels, vector<pair<int, int>>& equivalence);

	void drawABox(vector<int>, int, string, float);
	vector<int> getBox(vector<vector<int>>&);
	vector<Plot> boundToPlots(vector<int> vc, int offset);
	vector<string> getDigit(int);
	
	bool isValidTest(map<int, vector<Plot>>::iterator iter);
	void checkDigit(vector<vector<vector<int>>> & imgs, vector<string>& strs);
	void sortVC(vector<vector<int> >& vc, int state);
	vector<CImg<float>> cutImgs(CImg<float>& img, vector<vector<int> >& vc, int state);
	vector<vector<int>> combiVC(vector<vector<int> >& vc);

	CImg<float> imgOriginal;//原图
	CImg<float> img;		//二值图
	CImg<float> imgForCut;	//用于剪切中间生成数字图像

	//key是label,values是点集。
	map<int, vector<Plot>> blocks;			//连通块（label，点集）
	
	vector<Group> myGroups;					//集群
	int myDistance;
	string MyInput;
	string myName;

	int myIndex;
	string myMode;
	string myOutput;
	SVM svm;
};

#endif  

