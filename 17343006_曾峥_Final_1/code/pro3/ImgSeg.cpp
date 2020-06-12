#include "ImgSeg.h"
using namespace std;
//#define alpha 1.0


template <class Type>
Type stringToNum(const string& str)
{
	istringstream iss(str);
	Type num;
	iss >> num;
	return num;
}

ImgSeg::~ImgSeg()
{
	blocks.clear();				//连通块（label，点集）
	myGroups.clear();
}

void ImgSeg::findTarget(string name, string input, string mode, string output, int index, int distance)
{
	reset();
	myName = name;
	MyInput = input;
	myOutput = output;
	myDistance = distance;

	//1.图片预处理
	Binarization(input, 1.0);//前后景分割
	//img.display("img");
	img.dilate(8);//3
	img.erode(8);
	//img.display("img");

	//2.求连通块，筛选出需要的元素
	cptConBlocks();
	deleteBlocks();	
	deleteGroups();
	imgOriginal.save(output.c_str());

	// 切割群
	getImg(imgForCut);

}


void ImgSeg::getImgFiles(vector<vector<int> >& vc, int state) {
	//排序，把digit的vector进行排序
	sortVC(vc, state);

	//剪切出测试图片
	cutImgs(imgForCut, vc, state);
}

void ImgSeg::reset()
{
	blocks.clear();				//连通块（label，点集）
}
vector<string> transfer(vector<vector<string>> &vc){
	//string -> char
	vector<string> VC;
	for (auto value : vc) {
		string temp = "";
		for (auto i : value) {
			temp += i;
		}
		VC.push_back(temp);
	}
	return VC;
}

bool writeToResultTxt(vector<string>& digits, int myIndex) {
	string txtPath = "Dataset/Ol9b_txt/ol9b (" + to_string(myIndex) + ").txt";
	ofstream file(txtPath.c_str(), fstream::out);
	if (file) cout << " new file created" << endl;
	else {
		cout << "failed!!\n";
		return  false;
	}

	for (auto i : digits)
		file << i << endl;
	file.close();
	return true;
}

vector<int> ImgSeg::getBox(vector<vector<int>> & vcs){
	//求topDigitBox
	vector<int> box;
	int x1, x2, y1, y2;
	x1 = y1 = INT_MAX;	//正无穷
	x2 = y2 = INT_MIN;	//负无穷
	//找出topDigit的边界
	for (vector<int> vc : vcs) {
		// 每个数字都有 x_low, x_high, y_low, y_high;
		x1 = vc[0] < x1 ? vc[0] : x1;
		x2 = vc[1] > x2 ? vc[1] : x2;
		y1 = vc[2] < y1 ? vc[2] : y1;
		y2 = vc[3] > y2 ? vc[3] : y2;
	}
	box.push_back(x1);
	box.push_back(x2);
	box.push_back(y1);
	box.push_back(y2);
	return box;
}

bool pushToVector(vector< vector<int>>& vec, int x_low, int x_high, int y_low, int y_high) {
	vector<int> values;
	values.push_back(x_low); values.push_back(x_high); values.push_back(y_low); values.push_back(y_high);
	vec.push_back(values);
	return true;
}

bool ImgSeg::pushToGroup(Group & group, int x_low, int x_high, int y_low, int y_high) {
	bool flag = pushToVector(group.blocks, x_low, x_high, y_low, y_high);
	if(flag) group.boundry = getBox(group.blocks);
	return true;
}

bool ImgSeg::addNewGroup(int x_low, int x_high, int y_low, int y_high) {
	vector< vector<int>> blocks;
	vector<int> boundry;
	bool flag = pushToVector(blocks, x_low, x_high, y_low, y_high);
	if (flag)  boundry = getBox(blocks);
	myGroups.push_back(Group(true, boundry, blocks));
	return true;
}

int getDistance(Point a, Point b) {
	int weight = 5; 
	return sqrt(pow(a.x - b.x, 2) + pow(weight*(a.y - b.y), 2));
}
/*
	找到合群的序号
*/
int ImgSeg::findGroup(int x_low,int x_high, int y_low, int y_high) {
	Point a(x_low, y_low);
	int width = x_high - x_low;
	int height = y_high - y_low;
	int MIN = 999999, pos = -1;
	
	for (int i = 0; i < myGroups.size(); i++) {
		//cout << "Group " << i << ":" << endl;
		vector<int> temp = myGroups[i].boundry;
		vector<vector<int>> blks = myGroups[i].blocks;
		int minDist = 999999;	//和每一群的最小距离
		for (auto ele : blks) {
			Point b(ele[0], ele[2]);
			int tempDistance = getDistance(a, b);
			if (tempDistance < minDist)
				minDist = tempDistance;
		}
		//cout << "minDist：" << minDist << endl;
		if (minDist < MIN) {
			pos = i;
			MIN = minDist;;
		}
		int box_x_low = temp[0] - width;
		int box_x_high = temp[1] + width;
		int box_y_low = temp[2] - 10;
		int box_y_high = temp[3] + 10;

		int WIDTH = box_x_high - box_x_low;
		int HEIGHT = box_y_high - box_y_low;
	}
	if (MIN < 100) {
		cout << "MIN：" << MIN << " pos: " << pos << endl;
		return pos;
	}
	cout << "MIN：" << MIN << " pos: " << pos << endl;

	return -1;
}
/*筛选合格的联通块，返回true*/
bool ImgSeg::isValidTest(map<int, vector<Plot>>::iterator iter)
{
	cout << iter->second.size() << " ";
	
	//10保留小数点, 800保留 H0.bmp 括号
	int x_low, x_high, y_low, y_high;
	x_low = y_low = INT_MAX;	//正无穷
	x_high = y_high = INT_MIN;	//负无穷
	//找出边界
	for (Plot plot : iter->second) {
		int x = plot.x;
		int y = plot.y;
		x_low = x <= x_low ? x : x_low;
		x_high = x >= x_high ? x : x_high;
		y_low = y <= y_low ? y : y_low;
		y_high = y >= y_high ? y : y_high;
	}
	//连通块的宽、高、宽高比
	int width = abs(x_high - x_low);
	int height = abs(y_high - y_low);
	float size = (float)width / (float)height;

	if (iter->second.size() <= 70) return false;
	////if (iter->second.size() >= 2000 && size > 0.8) return false;
	////80 120
	cout << endl << MyInput << endl;
	if (MyInput != "C:\\Users\\inplus\\Desktop\\Project1\\Project1\\cutImg\\IMG_20191204_013949.bmp") {
		if ((width > 90 && height > 90)) return false;
	}
	if (height > 150) return false;
	if (width < 10 && height < 10) return false;
	if (size > 20 || size < 0.05) return false;

	int index = findGroup(x_low, x_high, y_low, y_high);
	// 遍历myGroup，查看有无相近的群，y上，可以比群小，但是不能比群大1/2，1/3
	// x上，与这个群的距离不能大于1.5倍
	// 如果找到一个，那么就加入这个群，如果找不到，就新建一个然后加入
	if (-1 != index)
		pushToGroup(myGroups[index], x_low, x_high, y_low, y_high);
	else {
		addNewGroup(x_low, x_high, y_low, y_high);
	}
	cout << "found  ";
	return true;
}

/**
 * @brief 判断两个轴对齐的矩形是否重叠
 * @param rc1 第一个矩阵
 * @param rc2 第二个矩阵
 * @return 两个矩阵是否重叠（边沿重叠，也认为是重叠）
 */
bool ImgSeg::isOverLap(Group a, Group b) {
	int x1 = a.boundry[0], y1 = a.boundry[2];
	int width1 = a.boundry[1] - a.boundry[0];
	int height1 = a.boundry[3] - a.boundry[2];

	int x2 = b.boundry[0], y2 = b.boundry[2];
	int width2 = b.boundry[1] - b.boundry[0];
	int height2 = b.boundry[3] - b.boundry[2];

	if (x1 + width1 > x2&&
		x2 + width2 > x1&&
		y1 + height1 > y2&&
		y2 + height2 > y1
		)
		return true;
	else
		return false;
}

/**
 * @brief 判断两个轴对齐的矩形是否相近，相近阈值为distance
 * @param a 第一个矩阵
 * @param b 第二个矩阵
 * @return 两个矩阵是否相近（距离<=distance）
 */
bool ImgSeg::isClose(Group a, Group b, int distance) {
	int x1 = a.boundry[0], y1 = a.boundry[2];
	int width1 = a.boundry[1] - a.boundry[0];
	int height1 = a.boundry[3] - a.boundry[2];

	int x2 = b.boundry[0], y2 = b.boundry[2];
	int width2 = b.boundry[1] - b.boundry[0];
	int height2 = b.boundry[3] - b.boundry[2];

	int derta_Y = abs(a.boundry[2] - b.boundry[2]);
	int derta_y = abs(a.boundry[3] - b.boundry[3]);
	if (derta_Y < 10 ) {
		if (x1 < x2)
		{
			if (x1 + width1 + distance > x2)
				return true;
			return false;
		}
		else {
			if (x2 + width2 + distance > x1)
				return true;
			return false;
		}
	}
	return false;
}

void ImgSeg::combiGroups() {
	int len = myGroups.size();
	int pos = 0;
	// 合并相交的群
	int count = 3;
	while (count--)
	{
		for (pos = 0; pos < len; pos++) {
			for (int i = pos + 1; i < len; i++) {
				if (myGroups[i].isValid) {
					if (isOverLap(myGroups[pos], myGroups[i])) {
						//第二个的bloc加入第一个pos
						//更新
						for (auto ele : myGroups[i].blocks) {
							//ele. x_high, y_low, y_high
							pushToGroup(myGroups[pos], ele[0], ele[1], ele[2], ele[3]);
						}
						myGroups[i].boundry = getBox(myGroups[i].blocks);
						myGroups[i].isValid = false;
					}
				}
			}
			// 删除false的群
			vector<Group>::iterator iter = myGroups.begin();
			while (iter != myGroups.end()) {
				if (!(*iter).isValid) {
					cout << "erase a group!!";
					iter = myGroups.erase(iter);
				}
				else iter++;
			}
			len = myGroups.size();
		}

		//合并距离相近的群,合并3次
		int distance = myDistance;
		for (pos = 0; pos < len; pos++) {
			for (int i = pos + 1; i < len; i++) {
				if (myGroups[i].isValid) {
					if (isClose(myGroups[pos], myGroups[i], distance)) {
						//第二个的bloc加入第一个pos
						//更新
						for (auto ele : myGroups[i].blocks) {
							//ele. x_high, y_low, y_high
							pushToGroup(myGroups[pos], ele[0], ele[1], ele[2], ele[3]);
						}
						myGroups[i].boundry = getBox(myGroups[i].blocks);
						myGroups[i].isValid = false;
					}
				}
			}
			// 删除false的群
			vector<Group>::iterator iter = myGroups.begin();
			while (iter != myGroups.end()) {
				if (!(*iter).isValid) {
					cout << "erase a group!!";
					iter = myGroups.erase(iter);
				}
				else iter++;
			}
			len = myGroups.size();
		}

	}
	
	
	count = 3;
	while (count--) {
		
	}
	
}

void ImgSeg::deleteBlocks() {
	//img.display("before delete");
	for (auto iter = blocks.begin(); iter != blocks.end(); iter++) {
		//求边界
		if (!isValidTest(iter)) {	//验证连通块有效性，求有效块的边界
			for (Plot plot : iter->second) {
				img(plot.x, plot.y) = 0;//像素点删除
			}
		}
	}
	
	//for (auto i : Ruler) drawABox(i, 1, "green", 0.5);
	/*for (auto i : topDigits) drawABox(i, 1, "green", 0.5);
	for (auto i : rulerDigit) drawABox(i, 2, "red", 0.5);
	for (auto i : bottomDigits) drawABox(i, 2, "blue", 0.5);*/

	//for (auto i : brackets) drawABox(i, 2, "orange", 0.2);
	//imgOriginal.display("after delete");
	//img.display("after delete");

	/*cout << "\ntopDigits: " << topDigits.size() << endl;		
	cout << "rulerDigit: " << rulerDigit.size() << endl;		
	cout << "bottomDigits: " << bottomDigits.size() << endl;
	cout << "brackets: " << brackets.size() << endl;			
	cout << "Ruler: " << Ruler.size() << endl;		*/
	
}
void ImgSeg::deleteGroups() {
	cout << "myGroups.size: " << myGroups.size() << endl;

	combiGroups();
	
	vector<Group>::iterator iter = myGroups.begin();
	while (iter != myGroups.end()) {
		vector<int> temp = (*iter).boundry;
		int width = temp[1] - temp[0];
		int height = temp[3] - temp[2];
		if ((*iter).blocks.size() < 2 && (width < 20 || height < 20 || (width > 100 &&  height > 100))) {
			cout << "erase a group!!  ";
			iter = myGroups.erase(iter);
		}
		else if (height < 20) {
			cout << "erase a group!!  ";
			iter = myGroups.erase(iter);
		}
			//去掉比名字还大块的
		else iter++;
	}

	cout << "\nmyGroups.size: " << myGroups.size() << endl;

	vector<string> colors;
	colors.push_back("blue");
	colors.push_back("red");
	colors.push_back("green");
	colors.push_back("orange");

	for (int i = 0; i < myGroups.size(); i++) {
		drawABox(myGroups[i].boundry, 0, colors[i % 4], 0.4);
		//for (auto j : myGroups[i].blocks) drawABox(j, 0, "blue", 0.3);
	}

	//img.display("after delete");
	//imgOriginal.display();
}

void ImgSeg::getImg(CImg<float> & img) {
	string file = "C:\\Users\\inplus\\Desktop\\Project1\\Project1\\result\\groups\\";
	string folder_name = file + myName + "\\";
	CreateFolder(folder_name);
	cout << folder_name;

	for (int i = 0; i < myGroups.size(); i++) {
		string file_name = folder_name  + to_string(i) + ".bmp";
		vector<int> vc = myGroups[i].boundry;

		int x1 = vc[0], x2 = vc[1], y1 = vc[2], y2 = vc[3];
		CImg<float> tempImg = CImg<float>(x2 - x1 + 1, y2 - y1 + 1, 1, 3);
		cimg_forXY(tempImg, x, y) tempImg(x, y) = 0;
		for (int i = 0; i + y1 <= y2; i++) {
			for (int j = 0; j + x1 <= x2; j++) {
				//3个channel 分别赋值
				tempImg(j, i, 0) = img(j + x1, i + y1, 0);
				tempImg(j, i, 1) = img(j + x1, i + y1, 1);
				tempImg(j, i, 2) = img(j + x1, i + y1, 2);
			}
		}
		tempImg.save(file_name.c_str());
		//tempImg.resize(32, 48);
		/*  eg: result/.....bmp/i.bmp   */
	}
}

CImg<float> ImgSeg::RGBtoGray(CImg<float> img)
{
	//创建一个图，depth为1, 1是spectrum = channel
	CImg<float> grayed = CImg<float>(img._width, img._height, 1, 1);
	cimg_forXY(img, x, y)
	{
		int b = img(x, y, 0);
		int g = img(x, y, 1);
		int r = img(x, y, 2);
		int newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);
		grayed(x, y) = newValue;
	}
	return grayed;
}

void ImgSeg::Binarization(string input)
{
	Binarization(input, 1.0);	//默认alpha=1.0
}
void ImgSeg::Binarization(string input,  float alpha)
{
	CImg<float> tempImg;
	tempImg.load(input.c_str());
	tempImg.resize(840, 600);
	imgOriginal = imgForCut = tempImg;
	//imgForCut.display();
	img = RGBtoGray(imgOriginal);		//灰度处理

	int otsusValue = otsu(img, alpha);//前后景分割阈值 otsus算法
	cout << "otsusValue = " << otsusValue << endl;
	cimg_forXY(img, x, y)
	{
		img(x, y) = img(x, y) >= otsusValue ? 255 : 0;
	}

	img = converse(img);
}

bool cmp(vector<int> a, vector<int> b) {
	return a[0] < b[0];
}
bool cmp2(vector<int> a, vector<int> b) {
	return a[2] > b[2];
}

void ImgSeg::sortVC(vector<vector<int> > & vc, int state) {
	sort(vc.begin(), vc.end(), cmp);
	for (vector<int> i : vc) { cout << i[0] << " ";}

	//对于横放的数字，中间小数的排序方式可以根据盒子的高度与数字相比
	if (!vc.empty()) {
		vector<int> front = vc[0]; 
		vector<int> box = getBox(vc);
		//drawABox(box, 5, "black", 0.5);
		//drawABox(front, 5, "black", 0.7);
		int height1 = box[3] - box[2];
		int height2 = front[3] - front[2];
		cout << "height1 = "<< height1 << " height2 = " << height2 << endl;
		if (height1 > 2 * height2 ){
			cout << "数据还需要根据y排序！！\n";
			/*和前面的x_high做差，derta < 5 说明在一个群*/
								
			vector<vector<int> > vcTemp;	//临时变量，一个群
			int count = 0;					//临时变量，群的成员个数
			int pos_temp = 0;				//一个群的起点

			for (int pos = 0; pos < vc.size(); pos++) {
				cout << "pos: " << pos << "   ";
				if(pos != 0){
					int derta = vc[pos][1] - vc[pos - 1][1];	//和前面的差
					cout << "derta: " << derta << endl;
					if (derta < 4) {
						vcTemp.push_back(vc[pos]);
						count++;
					}
					else {
						//new group
						cout << "   New group!  group size :" << vcTemp.size() << endl
							<< "    pos_temp = " << pos_temp << "   count = " << count << endl;
						sort(vcTemp.begin(), vcTemp.end(), cmp2);
						for (int i = 0; i < count; i++)
							vc[pos_temp + i] = vcTemp[i];
						pos_temp += count;	//注意是pos_temp
						vcTemp.clear();
						vcTemp.push_back(vc[pos]);
						count = 1;	
					}
				}
				//第一个元素直接加入一个vector,不用和前面的比较
				else if(pos == 0){
					vcTemp.push_back(vc[pos]);
					count++;
				}
			}
			//最后一个group
			if (!vcTemp.empty()) {
				cout << "   New group!  group size :" << vcTemp.size() << endl;
				sort(vcTemp.begin(), vcTemp.end(), cmp2);
				for (int i = 0; i < count; i++)
					vc[pos_temp + i] = vcTemp[i];
			}
		}
	}
}

void saveImg(CImg<float>& img, int index, int state, int i) {
	string stateStr;
	if (state == 0) stateStr = "topDigit";
	else if (state == 1) stateStr = "rulerDigit";
	else stateStr = "bottomDigit";
	string prefix = "temp/" + to_string(index) + "/temp_" + stateStr + "/";
	CreateFolder(prefix);

	string path = prefix + to_string(i) + ".bmp";
	cout << "path: " << path << "    ";
	
	img.save(path.c_str());
}
vector<CImg<float>> ImgSeg::cutImgs(CImg<float> &img, vector<vector<int> >&vc,int state) {
	vector<CImg<float>> myTests;
	for (int i = 0; i < vc.size(); i++) {
		int x1 = vc[i][0], x2 = vc[i][1], y1 = vc[i][2], y2 = vc[i][3];
		CImg<float> tempImg = CImg<float>(x2 - x1 + 1, y2 - y1 + 1, 1, 3);
		cimg_forXY(tempImg, x, y) tempImg(x, y) = 0;
		for (int i = 0; i + y1 <= y2; i++) {
			for (int j = 0; j + x1 <= x2; j++) {
				//3个channel 分别赋值
				tempImg(j, i,0) = img(j + x1, i + y1,0);
				tempImg(j, i,1) = img(j + x1, i + y1,1);
				tempImg(j, i,2) = img(j + x1, i + y1,2);
			}
		}
		/*需要旋转的图像*/
		if(tempImg._width >= tempImg._height){
			CImg<float> tempPic = CImg<float>(tempImg._height, tempImg._width, 1, 1);
			int width1 = tempImg._width;
			int height1 = tempImg._height;
			cimg_forXY(tempPic, x, y) {
				tempPic(x, y) = tempImg(y, height1 - x - 1);
			}
			tempImg = tempPic;
		}
		tempImg.resize(32, 48);
		/*  eg: temp/myIndex/temp_topDigit/i.bmp   */
		saveImg(tempImg, myIndex, state, i);
		myTests.push_back(tempImg);
	}
	return myTests;
}

void setColor(unsigned char c1[3], unsigned char c2[3]) {
	for (int i = 0; i < 3; i++) {
		c1[i] = c2[i];
	}
}
void ImgSeg::drawABox(vector<int> pic, int setoff, string COLOR, float opacity) {
	unsigned char red[3] = { 255, 0, 0 };
	unsigned char blue[3] = { 0, 0 ,255 };
	unsigned char green[3] = { 0, 255 ,0 };
	unsigned char black[3] = { 0, 0 ,0 };
	unsigned char yellow[3] = { 255, 255 ,0 };
	unsigned char orange[3] = { 255, 97 ,0 };

	unsigned char color[3];
	if (COLOR == "red") setColor(color, red);
	else if (COLOR == "blue") setColor(color, blue);
	else if (COLOR == "green") setColor(color, green);
	else if (COLOR == "black") setColor(color, black);
	else if (COLOR == "yellow") setColor(color, yellow);
	else if (COLOR == "orange") setColor(color, orange);

	vector<Plot> plots_4 = boundToPlots(pic, setoff);	//1的间隔

	imgOriginal.draw_rectangle(plots_4[0].x, plots_4[0].y, plots_4[3].x, plots_4[3].y, color, opacity);
}
void ImgSeg::checkDigit(vector<vector<vector<int>>> &AllImags, vector<string>& AllDigits) {
	fstream in;
	string str;
	str = "ol9b (" + to_string(myIndex) + ").txt";
	
	//读取2/3个字符流
	string path = "Dataset/Ol9b_txt/" + str;
	in.open(path.c_str());
	vector<string> answers;
	for (int i = 0; i < AllDigits.size(); i++) {
		string line;
		while (in >> line) {
			answers.push_back(line);
		}
	}
	
	//比较vc和answer
	cout << "应该有的个数 answer.size() = " << answers.size() << endl;
	
	int count = 0;
	int sum = 0;
	int offset = 1;
	float opacity = 0.8;
	cout << "识别结果如下： \n";
	for (int i = 0; i < AllDigits.size() && i < answers.size() && i < AllImags.size(); i++) {
		for (int j = 0; j < AllDigits[i].length() && j < answers[i].length() && j < AllImags[i].size(); j++) {
			if (AllDigits[i][j] == answers[i][j]) {	//char-char比较
				count++;
				cout << AllDigits[i][j] << " ";
				//在图中画出这个vc框
				drawABox(AllImags[i][j], offset, "red", opacity);
			}
			else {
				cout << AllDigits[i][j] << " Error!" << " ";
				drawABox(AllImags[i][j], offset, "black", opacity);
			}
		}
		cout << endl;
		sum += AllDigits[i].length();
	}
	if(!sum) cout << "正确率：" << (float)count / sum << endl;
	//imgOriginal.display("task1");
}

vector<vector<int>>ImgSeg::combiVC(vector<vector<int> >& vc) {
	vector<vector<int>> Vers;
	vector<vector<int>> combinedVC;
	if (!vc.empty()) {
		vector<int> front = vc[0];
		if (front[3] - front[2] > 100) {
			//将两个括号和合并成一个，并得到两个顶点值
			for (int i = 0; i < vc.size(); i += 2) {
				//得到两个顶点
				int x_left = vc[i][1];
				int x_right = vc[i][0];
				vector<int> temp;
				temp.push_back(x_left);
				temp.push_back(x_right);
				Vers.push_back(temp);
				//合并两个括号
				for (int j = 0; j < vc[i].size(); j++) {
					if (j % 2 == 0) vc[i][j] = min(vc[i][j], vc[i + 1][j]);
					else vc[i][j] = max(vc[i][j], vc[i + 1][j]);
				}
				combinedVC.push_back(vc[i]);
			}
			vc = combinedVC;
		}
	}
	//for (auto i : vc) drawABox(i, 0, "black", 0.9);
	//imgOriginal.display();
	return Vers;
}

vector<string> ImgSeg::getDigit(int state)
{
	string path;	//这个类的图片的路径
	if (state == 0)	path = "temp/" + to_string(myIndex) + "/temp_topDigit/";
	if (state == 1)	path = "temp/" + to_string(myIndex) + "/temp_rulerDigit/";
	if (state == 2) path = "temp/" + to_string(myIndex) + "/temp_bottomDigit/";
	
	vector<string> tempStr;
	vector<int> results = svm.test(path);
	for (auto i : results) {
		string str = to_string(i);
		tempStr.push_back(str);
	}
	
	return tempStr;
}

CImg<float> ImgSeg::BinSimple(CImg<float> img, int threhold)
{
	cimg_forXY(img, x, y) {
		img(x, y) = img(x, y) > threhold ? 255 : 0;
	}
	return img;
}

int ImgSeg::otsu(CImg<float>& image, float alpha)
{
	double hist[256];
	normalizedHistogram(image, hist);	//直方图归一化

	double omega[256];
	double mu[256];

	omega[0] = hist[0];
	mu[0] = 0;
	for (int i = 1; i < 256; i++)
	{
		omega[i] = omega[i - 1] + hist[i]; //累积分布函数
		mu[i] = mu[i - 1] + i * hist[i];
	}
	double mean = mu[255];// 灰度平均值
	double max = 0;
	int k_max = 0;
	for (int k = 1; k < 255; k++)
	{
		double PA = omega[k]; // A类所占的比例
		double PB = 1 - omega[k]; //B类所占的比例
		double value = 0;
		if (fabs(PA) > 0.001 && fabs(PB) > 0.001)
		{
			double MA = mu[k] / PA; //A 类的灰度均值
			double MB = (mean - mu[k]) / PB;//B类灰度均值
			value = pow(PA, alpha) * (MA - mean) * (MA - mean) + pow(PB, alpha) * (MB - mean) * (MB - mean);//类间方差

			if (value > max)
			{
				max = value;
				k_max = k;
			}
		}
		//qDebug() <<k << " " << hist[k] << " " << value;
	}
	return k_max;
}

void ImgSeg::cptConBlocks()
{
	//中间数据
	int NumberOfRuns = 0;				//团的个数
	vector<int> stRun, enRun, rowRun;	//每个团的参数
	vector<int> runLabels;				//团的label
	vector<pair<int, int> > equivalences;//等价对

	CImg<int> img = this->img;

	fillRunVectors(img, NumberOfRuns, stRun, enRun, rowRun);//找到每行的团
	firstPass(stRun, enRun, rowRun, NumberOfRuns, runLabels, equivalences, 1);//团的连通。0是4邻接，1是8邻接
	replaceSameLabel(runLabels, equivalences);	//等价对的处理,更新runLabels

	cout << "\nrums：" << NumberOfRuns << endl;
	cout << "runLabels:" << runLabels.size() << endl;
	cout << "equivalences :" << equivalences.size() << endl;//两个及以上的等价块

	//set是去重，找到所有不同labels
	set<int> SetLabel(runLabels.begin(), runLabels.end());
	cout << "set：" << SetLabel.size() << endl;

	//新建不同块，去重label的点集合
	for (int i = 0; i < NumberOfRuns; i++) {
		for (int value : SetLabel) {
			//label为value的run集,//寻找label相同的run
			if (runLabels[i] == value) {
				//for every run
				for (int j = stRun[i]; j <= enRun[i]; j++) {
					//for every plot
					int x = j;
					int y = rowRun[i];
					blocks[value].push_back(Plot(x, y));
				}
			}
		}
	}
	//cout << "BlockSize: " << blocks.size();
}


CImg<float> ImgSeg::converse(CImg<float> img)
{
	cimg_forXY(img, x, y)
	{
		img(x, y) = img(x, y) == 0 ? 255 : 0;
	}//现在白色是目标颜色，黑色是背景色
	return img;
}

void ImgSeg::normalizedHistogram(CImg<int> img, double* hist)
{
	int height = img.height();
	int width = img.width();
	int N = height * width;
	CImg<double> histImg = img.histogram(256, 0, 255);
	//histImg.display_graph("Histogram", 3);
	//直方图归一化
	int i = 0;
	cimg_forXY(histImg, x, y)
	{
		hist[i] = histImg(x, y) = double(histImg(x, y) / N);
		//cout << histImg(x, y) << " " << hist[i] << endl;
		i++;
	}
	//histImg.display_graph("Histogram", 3);
}

void ImgSeg::fillRunVectors(CImg<int>& bwImage, int& NumberOfRuns,
	vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun)
{
	//bwImage.display("bwImage");
	cimg_forY(bwImage, y) {
		vector<int> rowData;
		cimg_forX(bwImage, x) {
			//cout << typeid(bwImage(x, y)).name();
			rowData.push_back(bwImage(x, y));
			//cout << rowData[x] << " ";
		}
		//cout << endl;
		if (rowData[0] == 255)
		{
			NumberOfRuns++;
			stRun.push_back(0);
			rowRun.push_back(y);
		}
		for (int j = 1; j < bwImage._width; j++)
		{
			if (rowData[j - 1] == 0 && rowData[j] == 255)
			{
				NumberOfRuns++;
				stRun.push_back(j);
				rowRun.push_back(y);
			}
			else if (rowData[j - 1] == 255 && rowData[j] == 0)
			{
				enRun.push_back(j - 1);
			}
		}
		if (rowData[bwImage._width - 1])
		{
			enRun.push_back(bwImage._width - 1);
		}
	}
	//cout << "NumberOfRuns:" << NumberOfRuns << endl;
}
// 把run理解一行图像上的一个连续的 白色像素条
// 团的标记与等价对列表的生成
void ImgSeg::firstPass(vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun, int NumberOfRuns,
	vector<int>& runLabels, vector<pair<int, int>>& equivalences, int offset)
{
	runLabels.assign(NumberOfRuns, 0);
	int idxLabel = 1;
	int curRowIdx = 0;
	int firstRunOnCur = 0;
	int firstRunOnPre = 0;
	int lastRunOnPre = -1;
	for (int i = 0; i < NumberOfRuns; i++)
	{
		// 如果是该行的第一个run，则更新上一行第一个run和最后一个run的序号
		if (rowRun[i] != curRowIdx)
		{
			curRowIdx = rowRun[i]; // 更新行的序号
			firstRunOnPre = firstRunOnCur;
			lastRunOnPre = i - 1;
			firstRunOnCur = i;
		}
		// 遍历上一行的所有run，判断是否于当前run有重合的区域
		for (int j = firstRunOnPre; j <= lastRunOnPre; j++)
		{
			// 区域重合 且 处于相邻的两行
			if (stRun[i] <= enRun[j] + offset && enRun[i] >= stRun[j] - offset && rowRun[i] == rowRun[j] + 1)
			{
				if (runLabels[i] == 0) // 没有被标号过
					runLabels[i] = runLabels[j];
				else if (runLabels[i] != runLabels[j])// 已经被标号       
				{
					equivalences.push_back(make_pair(runLabels[i], runLabels[j])); // 保存等价对
					//cout << "runLabels[i]: " << runLabels[i] << " runLabels[j]: " << runLabels[j]<< endl;
				}

			}
		}
		if (runLabels[i] == 0) // 没有与前一列的任何run重合
		{
			runLabels[i] = idxLabel++;
		}

	}
}
void ImgSeg::replaceSameLabel(vector<int>& runLabels, vector<pair<int, int>>& equivalence)
{
	int maxLabel = *max_element(runLabels.begin(), runLabels.end());
	vector<vector<bool>> eqTab(maxLabel, vector<bool>(maxLabel, false));
	vector<pair<int, int>>::iterator vecPairIt = equivalence.begin();
	while (vecPairIt != equivalence.end())
	{
		eqTab[vecPairIt->first - 1][vecPairIt->second - 1] = true;
		eqTab[vecPairIt->second - 1][vecPairIt->first - 1] = true;
		vecPairIt++;
	}
	vector<int> labelFlag(maxLabel, 0);
	vector<vector<int>> equaList;
	vector<int> tempList;
	cout << maxLabel << " ";
	for (int i = 1; i <= maxLabel; i++)
	{
		if (labelFlag[i - 1])
		{
			continue;
		}
		labelFlag[i - 1] = equaList.size() + 1;
		tempList.push_back(i);
		for (vector<int>::size_type j = 0; j < tempList.size(); j++)
		{
			for (vector<bool>::size_type k = 0; k != eqTab[tempList[j] - 1].size(); k++)
			{
				if (eqTab[tempList[j] - 1][k] && !labelFlag[k])
				{
					tempList.push_back(k + 1);
					labelFlag[k] = equaList.size() + 1;
				}
			}
		}
		equaList.push_back(tempList);
		tempList.clear();
	}
	cout << equaList.size() << endl;
	for (vector<int>::size_type i = 0; i != runLabels.size(); i++)
	{
		runLabels[i] = labelFlag[runLabels[i] - 1];
	}
}


vector<Plot> ImgSeg::boundToPlots(vector<int> vc, int offset) {
	/*
		x_low, x_high, y_low, y_high
		----------------------------
		p1 (x_low, y_low)
		p2 (x_high, y_low);
		p3 (x_low, y_high)
		p4 (x_high, y_high)
	*/
	int x_low, x_high, y_low, y_high;

	if (offset == -1) {
		x_low = vc[0] - 5;
		x_high = vc[1] + 5;
		y_low = vc[2] - 20;
		y_high = vc[3] + 40;
	}
	else {
		x_low = vc[0] - offset;
		x_high = vc[1] + offset;
		y_low = vc[2] - offset;
		y_high = vc[3]+ offset;
	}
	vector<Plot> plots;
	plots.push_back(Plot(x_low, y_low));
	plots.push_back(Plot(x_high, y_low));
	plots.push_back(Plot(x_low, y_high));
	plots.push_back(Plot(x_high, y_high));

	return plots;
}

