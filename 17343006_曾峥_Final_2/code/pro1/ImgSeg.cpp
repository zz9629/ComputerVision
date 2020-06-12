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
	blocks.clear();				//��ͨ�飨label���㼯��
	myGroups.clear();
}

void ImgSeg::findTarget(string name, string input, string output)
{
	reset();
	myName = name;
	MyInput = input;
	myOutput = output;

	//1.ͼƬԤ����
	Binarization(input, 1.0);//ǰ�󾰷ָ�
	dilate_erode();

	//2.����ͨ�飬ɸѡ����Ҫ��Ԫ��
	cptConBlocks();
	deleteBlocks();	
	deleteGroups();

	// �и�Ⱥ
	getGroupImg(imgForCut);
	//ÿһ��Ⱥ���ַ��ָ�
	getCharactors(imgForCut);

	cout << output << endl;
	imgOriginal.save(output.c_str());

}



void ImgSeg::reset()
{
	blocks.clear();				//��ͨ�飨label���㼯��
}

vector<int> ImgSeg::getBox(vector<vector<int>> & vcs){
	//��topDigitBox
	vector<int> box;
	int x1, x2, y1, y2;
	x1 = y1 = INT_MAX;	//������
	x2 = y2 = INT_MIN;	//������
	//�ҳ�topDigit�ı߽�
	for (vector<int> vc : vcs) {
		// ÿ�����ֶ��� x_low, x_high, y_low, y_high;
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
	�ҵ���Ⱥ�����
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
		int minDist = 999999;	//��ÿһȺ����С����
		for (auto ele : blks) {
			Point b(ele[0], ele[2]);
			int tempDistance = getDistance(a, b);
			if (tempDistance < minDist)
				minDist = tempDistance;
		}
		//cout << "minDist��" << minDist << endl;
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
		cout << "MIN��" << MIN << " pos: " << pos << endl;
		return pos;
	}
	cout << "MIN��" << MIN << " pos: " << pos << endl;

	return -1;
}
/*ɸѡ�ϸ����ͨ�飬����true*/
bool ImgSeg::isValid(map<int, vector<Plot>>::iterator iter)
{
	//10����С����, 800���� H0.bmp ����
	int x_low, x_high, y_low, y_high;
	x_low = y_low = INT_MAX;	//������
	x_high = y_high = INT_MIN;	//������
	//�ҳ��߽�
	for (Plot plot : iter->second) {
		int x = plot.x;
		int y = plot.y;
		x_low = x <= x_low ? x : x_low;
		x_high = x >= x_high ? x : x_high;
		y_low = y <= y_low ? y : y_low;
		y_high = y >= y_high ? y : y_high;
	}
	//��ͨ��Ŀ��ߡ���߱�
	int width = abs(x_high - x_low);
	int height = abs(y_high - y_low);
	float size = (float)width / (float)height;
	//��Ƭ�ĸ߿� WIDTH  HEIGHT

	cout << endl << MyInput << endl;
	/*if (MyInput != "C:\\Users\\inplus\\Desktop\\data\\cutImg\\IMG_20191204_013949.bmp") {
		if ((width > 90 && height > 90)) return false;
	}*/
	if (iter->second.size() <= 70 * double(HEIGHT)/ 600) return false;
	if (height > HEIGHT / 5) return false;
	if (width < WIDTH / 60 && height < HEIGHT/60) return false;
	if (size > 20 || size < 0.05) return false;

	int index = findGroup(x_low, x_high, y_low, y_high);
	// ����myGroup���鿴���������Ⱥ��y�ϣ����Ա�ȺС�����ǲ��ܱ�Ⱥ��1/2��1/3
	// x�ϣ������Ⱥ�ľ��벻�ܴ���1.5��
	// ����ҵ�һ������ô�ͼ������Ⱥ������Ҳ��������½�һ��Ȼ�����
	if (-1 != index)
		pushToGroup(myGroups[index], x_low, x_high, y_low, y_high);
	else {
		addNewGroup(x_low, x_high, y_low, y_high);
	}
	cout << "found  ";
	return true;
}

/**
 * @brief �ж����������ľ����Ƿ��ص�
 * @param rc1 ��һ������
 * @param rc2 �ڶ�������
 * @return ���������Ƿ��ص��������ص���Ҳ��Ϊ���ص���
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
 * @brief �ж����������ľ����Ƿ�����������ֵΪdistance
 * @param a ��һ������
 * @param b �ڶ�������
 * @return ���������Ƿ����������<=distance��
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

	double theshold = double(HEIGHT) / 30;
	cout << "�ϲ�Ⱥ�߶���ֵ��" << theshold << endl;
	if (derta_Y < theshold &&  derta_y < theshold) {
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
	// �ϲ��ཻ��Ⱥ
	int count = 5;
	while (count--)
	{
		for (pos = 0; pos < len; pos++) {
			for (int i = pos + 1; i < len; i++) {
				if (myGroups[i].isValid) {
					if (isOverLap(myGroups[pos], myGroups[i])) {
						//�ڶ�����bloc�����һ��pos
						//����
						for (auto ele : myGroups[i].blocks) {
							//ele. x_high, y_low, y_high
							pushToGroup(myGroups[pos], ele[0], ele[1], ele[2], ele[3]);
						}
						myGroups[i].boundry = getBox(myGroups[i].blocks);
						myGroups[i].isValid = false;
					}
				}
			}
			// ɾ��false��Ⱥ
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

		//�ϲ����������Ⱥ,�ϲ�3��
		int distance = myDistance;
		for (pos = 0; pos < len; pos++) {
			for (int i = pos + 1; i < len; i++) {
				if (myGroups[i].isValid) {
					if (isClose(myGroups[pos], myGroups[i], distance)) {
						//�ڶ�����bloc�����һ��pos
						//����
						for (auto ele : myGroups[i].blocks) {
							//ele. x_high, y_low, y_high
							pushToGroup(myGroups[pos], ele[0], ele[1], ele[2], ele[3]);
						}
						myGroups[i].boundry = getBox(myGroups[i].blocks);
						myGroups[i].isValid = false;
					}
				}
			}
			// ɾ��false��Ⱥ
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
}

void ImgSeg::deleteBlocks() {
	//img.display("before delete");
	for (auto iter = blocks.begin(); iter != blocks.end(); iter++) {
		//��߽�
		if (!isValid(iter)) {	//��֤��ͨ����Ч�ԣ�����Ч��ı߽�
			for (Plot plot : iter->second) {
				img(plot.x, plot.y) = 0;//���ص�ɾ��
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

		double threhold_Low = 20 * HEIGHT /600;
		double threhold_High = 100 * HEIGHT / 600;

		if ((*iter).blocks.size() < 2 && 
			(width < threhold_Low || height < threhold_Low || (width > threhold_High &&  height > threhold_High))) {
			cout << "erase a group!!  ";
			iter = myGroups.erase(iter);
		}
		else if (height < threhold_Low) {
			cout << "erase a group!!  ";
			iter = myGroups.erase(iter);
		}
			//ȥ�������ֻ�����
		else iter++;
	}

	cout << "\nmyGroups.size: " << myGroups.size() << endl;

	vector<string> colors;
	colors.push_back("blue"); 
	colors.push_back("red");
	colors.push_back("green");
	colors.push_back("orange");

	for (int i = 0; i < myGroups.size(); i++) {
		drawABox(myGroups[i].boundry, 0, colors[i % 4], 0.2);
		//for (auto j : myGroups[i].blocks) drawABox(j, 0, "blue", 0.3);
	}

	/*img.display("after delete");
	imgOriginal.display();*/
}

void ImgSeg::getGroupImg(CImg<float> & img) {
	string file = "C:\\Users\\inplus\\Desktop\\data\\result\\groups\\";
	string folder_name = file + myName + "\\";
	CreateFolder(folder_name);
	cout << folder_name;

	for (int i = 0; i < myGroups.size(); i++) {
		string file_name = folder_name  + to_string(i) + ".bmp";
		vector<int> vc = myGroups[i].boundry;

		//int x1 = vc[0], x2 = vc[1], y1 = vc[2], y2 = vc[3];
		int paddingSize = (vc[3] - vc[2]) / 4;
		int x1 = max(vc[0] - paddingSize, 0);
		int x2 = min(vc[1] + paddingSize, img._width - 1);
		int y1 = max(vc[2] - paddingSize, 0);
		int y2 = min(vc[3] + paddingSize, img._height - 1);
		CImg<float> tempImg = CImg<float>(x2 - x1 + 1, y2 - y1 + 1, 1, 3);
		cimg_forXY(tempImg, x, y) tempImg(x, y) = 0;
		for (int i = 0; i + y1 <= y2; i++) {
			for (int j = 0; j + x1 <= x2; j++) {
				//3��channel �ֱ�ֵ
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

/*
	@ vc: ���Ⱥ�ı߽�
	@ subfile�����Ⱥ���ַ��ı���Ŀ¼
	@ isHaveDigit �����Ⱥ���Ƿ������֣���Ҫ�Ѽ������Сһ��
	1. ��ͼ��imgFor2�����и�����Ⱥ
	2. �������Ⱥ�Ĵ�ֱͶӰ
	3. �ҵ��ַ�
	4. �и��ַ�

*/
void ImgSeg::getGroupSubImg(vector<int> & vc, string subfile, bool isHaveDigit = false) {

	/*img.display();
	imgFor2.display();*/

	// 1. ��ͼ���и�����Ⱥ
	int X1 = vc[0], X2 = vc[1], Y1 = vc[2], Y2 = vc[3];
	CImg<float> tempImg = CImg<float>(X2 - X1 + 1, Y2 - Y1 + 1, 1, 1);
	cimg_forXY(tempImg, x, y) tempImg(x, y) = 0;
	for (int i = 0; i + Y1 <= Y2; i++) {
		for (int j = 0; j + X1 <= X2; j++) {
			tempImg(j, i) = imgFor2(j + X1, i + Y1, 0);
		}
	}
	//tempImg.display();				//���Ⱥ�Ķ�ֵͼ��

	// 2. �������Ⱥ�Ĵ�ֱͶӰ
	int height = tempImg.height();
	int width = tempImg.width();

	int* hist = new int[width];			//yͶӰ
	for (int i = 0; i < width; i++)
		hist[i] = 0;

	cimg_forXY(tempImg, x, y)
	{
		if (tempImg(x, y) == 255)
			hist[x] += 1;
	}
	for (int i = 0; i < width; i++)
		cout << hist[i] << " ";

	// 2. ��ʾ��ֱͶӰ
	//srcImage.rows: height, cols: width
	{
		Mat histogramImage(height, width, CV_8UC1);
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++)
			{
				int value = 0;  //����Ϊ��ɫ��  
				histogramImage.at<uchar>(i, j) = value;
			}
		}
		for (int i = 0; i < width; i++)
			for (int j = 0; j < hist[i]; j++)
			{
				int value = 255;  //����Ϊ��ɫ  
				histogramImage.at<uchar>(j, i) = value;
			}
		//imshow("��ֱͶӰ", histogramImage);
		//waitKey(0);
	}

	// 3. ����hist���ҵ��ַ�
	int count = 0;		//�ҵ����ַ�����
	int min_thresh = 3;	//��ֵ
	int min_range;
	min_range = isHaveDigit == true? height / 4 : double(height) / 1.5;

	int begin = 0;
	int end = 0;
	for (int i = 0; i < width; i++) {
		if (hist[i] > min_thresh&& begin == 0)
		{
			begin = i;
		}
		else if (hist[i] > min_thresh&& i != width - 1 && begin != 0)
		{
			continue;
		}
		else if (i == width - 1 || (hist[i] < min_thresh && begin != 0))
		{

			end = i;
			int range = end - begin;
			//cout << "3" << " " << range << " " << min_range;

			if (range >= min_range)
			{
				// �и��
				{
					// ������ԭͼ�еľ���
					//int x1 = vc[0], x2 = vc[1], y1 = vc[2], y2 = vc[3];
					cout << "\n�ַ���ΧΪ��" << begin << " " << end << endl;
					vector<int> tempPic;
					tempPic.push_back(X1 + begin);
					tempPic.push_back(X1 + end);
					tempPic.push_back(Y1);
					tempPic.push_back(Y2);

					// ��ʾ���������и
					//drawABox(tempPic, 0, "blue", 0.3);

					string path = subfile + to_string(count) + ".bmp";
					//cout << "·����" << path << endl;
					int paddingSize = 2;// (tempPic[1] - tempPic[0]) / 8;
					int x1 = max(tempPic[0] - 2, 0);
					int x2 = min(tempPic[1] + 1, imgForCut._width-1);
					int y1 = max(tempPic[2] - paddingSize, 0);
					int y2 = min(tempPic[3] + paddingSize, imgForCut._height-1);
					
					CImg<float> tempImg = CImg<float>(x2 - x1 + 1, y2 - y1 + 1, 1, 3);
					cimg_forXY(tempImg, x, y) tempImg(x, y) = 0;
					for (int i = 0; i + y1 <= y2; i++) {
						for (int j = 0; j + x1 <= x2; j++) {
							//3��channel �ֱ�ֵ
							tempImg(j, i, 0) = imgForCut(j + x1, i + y1, 0);
							tempImg(j, i, 1) = imgForCut(j + x1, i + y1, 1);
							tempImg(j, i, 2) = imgForCut(j + x1, i + y1, 2);
						}
					}
					//tempImg.resize(100, 100);
					tempImg.save(path.c_str());
				}
				begin = 0;
				end = 0;
				count++;
			}

		}
		else if (hist[i] < min_thresh || begin == 0)
		{
			continue;
		}
		else
		{
			//printf("raise error!\n");
		}
	}
	cout << "�ҵ��ַ���Ϊ��" << count << endl;
	//imgOriginal.display(" ");
}

void ImgSeg::getCharactors(CImg<float>& imgForCut) {
	imgFor2.dilate(2);//3
	imgFor2.erode(2);//3

	string file = "C:\\Users\\inplus\\Desktop\\data\\result\\groups\\";
	string folder_name = file + myName + "\\";	//���ͼƬ���ļ���

	for (int i = 0; i < myGroups.size(); i++) {
		//MGI_32234324343rr.bmp�ļ��У�������Ⱥ��ͼ��
		string subfile = folder_name + "group_"+ to_string(i) + "\\";
		cout << "subfile" << subfile << endl;	//���Ⱥ���ļ���
		CreateFolder(subfile);

		vector<int> vc = myGroups[i].boundry;
		getGroupSubImg(vc, subfile);
	}
	//imgOriginal.display();
}

CImg<float> ImgSeg::RGBtoGray(CImg<float> img)
{
	//����һ��ͼ��depthΪ1, 1��spectrum = channel
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
	Binarization(input, 1.0);	//Ĭ��alpha=1.0
}
void ImgSeg::Binarization(string input,  float alpha)
{
	//���ò����ȣ�
	CImg<float> tempImg;
	tempImg.load(input.c_str());

	WIDTH = tempImg._width;
	HEIGHT = tempImg._height;

	myDistance = double(60) * WIDTH / 1100;

	//tempImg.resize(840*2, 600*2);
	imgOriginal = imgForCut = tempImg;
	//imgForCut.display();
	img = RGBtoGray(imgOriginal);		//�Ҷȴ���

	int otsusValue = otsu(img, alpha);//ǰ�󾰷ָ���ֵ otsus�㷨
	cout << "otsusValue = " << otsusValue << endl;
	cimg_forXY(img, x, y)
	{
		img(x, y) = img(x, y) >= otsusValue ? 255 : 0;
	}

	img = converse(img);
	imgFor2 = img;
}

void ImgSeg::dilate_erode() {
	double param = 8;

	param = (param * HEIGHT / 600);
	cout << "���͵Ĳ���Ϊ��" << param << endl;

	img.dilate(param);
	img.erode(param);
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

	//���ں�ŵ����֣��м�С��������ʽ���Ը��ݺ��ӵĸ߶����������
	if (!vc.empty()) {
		vector<int> front = vc[0]; 
		vector<int> box = getBox(vc);
		//drawABox(box, 5, "black", 0.5);
		//drawABox(front, 5, "black", 0.7);
		int height1 = box[3] - box[2];
		int height2 = front[3] - front[2];
		cout << "height1 = "<< height1 << " height2 = " << height2 << endl;
		if (height1 > 2 * height2 ){
			cout << "���ݻ���Ҫ����y���򣡣�\n";
			/*��ǰ���x_high���derta < 5 ˵����һ��Ⱥ*/
								
			vector<vector<int> > vcTemp;	//��ʱ������һ��Ⱥ
			int count = 0;					//��ʱ������Ⱥ�ĳ�Ա����
			int pos_temp = 0;				//һ��Ⱥ�����

			for (int pos = 0; pos < vc.size(); pos++) {
				cout << "pos: " << pos << "   ";
				if(pos != 0){
					int derta = vc[pos][1] - vc[pos - 1][1];	//��ǰ��Ĳ�
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
						pos_temp += count;	//ע����pos_temp
						vcTemp.clear();
						vcTemp.push_back(vc[pos]);
						count = 1;	
					}
				}
				//��һ��Ԫ��ֱ�Ӽ���һ��vector,���ú�ǰ��ıȽ�
				else if(pos == 0){
					vcTemp.push_back(vc[pos]);
					count++;
				}
			}
			//���һ��group
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

	vector<Plot> plots_4 = boundToPlots(pic, setoff);	//1�ļ��

	imgOriginal.draw_rectangle(plots_4[0].x, plots_4[0].y, plots_4[3].x, plots_4[3].y, color, opacity);
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
	normalizedHistogram(image, hist);	//ֱ��ͼ��һ��

	double omega[256];
	double mu[256];

	omega[0] = hist[0];
	mu[0] = 0;
	for (int i = 1; i < 256; i++)
	{
		omega[i] = omega[i - 1] + hist[i]; //�ۻ��ֲ�����
		mu[i] = mu[i - 1] + i * hist[i];
	}
	double mean = mu[255];// �Ҷ�ƽ��ֵ
	double max = 0;
	int k_max = 0;
	for (int k = 1; k < 255; k++)
	{
		double PA = omega[k]; // A����ռ�ı���
		double PB = 1 - omega[k]; //B����ռ�ı���
		double value = 0;
		if (fabs(PA) > 0.001 && fabs(PB) > 0.001)
		{
			double MA = mu[k] / PA; //A ��ĻҶȾ�ֵ
			double MB = (mean - mu[k]) / PB;//B��ҶȾ�ֵ
			value = pow(PA, alpha) * (MA - mean) * (MA - mean) + pow(PB, alpha) * (MB - mean) * (MB - mean);//��䷽��

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
	//�м�����
	int NumberOfRuns = 0;				//�ŵĸ���
	vector<int> stRun, enRun, rowRun;	//ÿ���ŵĲ���
	vector<int> runLabels;				//�ŵ�label
	vector<pair<int, int> > equivalences;//�ȼ۶�

	CImg<int> img = this->img;

	fillRunVectors(img, NumberOfRuns, stRun, enRun, rowRun);//�ҵ�ÿ�е���
	firstPass(stRun, enRun, rowRun, NumberOfRuns, runLabels, equivalences, 1);//�ŵ���ͨ��0��4�ڽӣ�1��8�ڽ�
	replaceSameLabel(runLabels, equivalences);	//�ȼ۶ԵĴ���,����runLabels

	cout << "\nrums��" << NumberOfRuns << endl;
	cout << "runLabels:" << runLabels.size() << endl;
	cout << "equivalences :" << equivalences.size() << endl;//���������ϵĵȼۿ�

	//set��ȥ�أ��ҵ����в�ͬlabels
	set<int> SetLabel(runLabels.begin(), runLabels.end());
	cout << "set��" << SetLabel.size() << endl;

	//�½���ͬ�飬ȥ��label�ĵ㼯��
	for (int i = 0; i < NumberOfRuns; i++) {
		for (int value : SetLabel) {
			//labelΪvalue��run��,//Ѱ��label��ͬ��run
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
	}//���ڰ�ɫ��Ŀ����ɫ����ɫ�Ǳ���ɫ
	return img;
}

void ImgSeg::normalizedHistogram(CImg<int> img, double* hist)
{
	int height = img.height();
	int width = img.width();
	int N = height * width;
	CImg<double> histImg = img.histogram(256, 0, 255);
	//histImg.display_graph("Histogram", 3);
	//ֱ��ͼ��һ��
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
// ��run���һ��ͼ���ϵ�һ�������� ��ɫ������
// �ŵı����ȼ۶��б������
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
		// ����Ǹ��еĵ�һ��run���������һ�е�һ��run�����һ��run�����
		if (rowRun[i] != curRowIdx)
		{
			curRowIdx = rowRun[i]; // �����е����
			firstRunOnPre = firstRunOnCur;
			lastRunOnPre = i - 1;
			firstRunOnCur = i;
		}
		// ������һ�е�����run���ж��Ƿ��ڵ�ǰrun���غϵ�����
		for (int j = firstRunOnPre; j <= lastRunOnPre; j++)
		{
			// �����غ� �� �������ڵ�����
			if (stRun[i] <= enRun[j] + offset && enRun[i] >= stRun[j] - offset && rowRun[i] == rowRun[j] + 1)
			{
				if (runLabels[i] == 0) // û�б���Ź�
					runLabels[i] = runLabels[j];
				else if (runLabels[i] != runLabels[j])// �Ѿ������       
				{
					equivalences.push_back(make_pair(runLabels[i], runLabels[j])); // ����ȼ۶�
					//cout << "runLabels[i]: " << runLabels[i] << " runLabels[j]: " << runLabels[j]<< endl;
				}

			}
		}
		if (runLabels[i] == 0) // û����ǰһ�е��κ�run�غ�
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

