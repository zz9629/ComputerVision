#include "EdgeDetect.h"
#include "CONCON.h"
#include <cmath>
#include <algorithm>
#include <functional>

#define Pi 3.141592653
#define DownSampledSize 8
#define gFilterX 5
#define gFilterY 5
#define sigma 1
#define thresholdLow 120
#define thresholdHigh 140
#define thetaSize 360
#define windowSize 40

EdgeDetect::EdgeDetect() {
	for (int i = 0; i < thetaSize; i++) {
		setSin.push_back(sin(2 * Pi * i / thetaSize));
		setCos.push_back(cos(2 * Pi * i / thetaSize));
	}
}
EdgeDetect::~EdgeDetect() {
	setSin.clear();
	setCos.clear();
	myLines.clear();
	myVertexs.clear();
}
void EdgeDetect::edgeDetect(string name, string input, string output, int number) {
	img.load(input.c_str());
	int width = img._width, height = img._height;
	if (width == 0 || height == 0) {
		cout << "Cannot open or find the image" << endl;
	}
	this->name = name;
	this->pointNumber = number;
	outputImage = img;
	imgGrayed = Binarization(img, 1);	
	imgDowned = downSampling(imgGrayed, DownSampledSize);	
	imgCannyed = canny(imgDowned);				
	houghImage = houghTransform(imgCannyed);	//hough变换
	//houghImage.display();

	houghReCompute(houghImage);		//求局部峰值
	//houghImage.display();
	findEdge(imgDowned);			//在图中找到这个4条边
	findVertex();					//根据4条边找到4个顶点
	drawVertexLines(outputImage, DownSampledSize);	//写如顶点信息，并在原图中标出顶点
	outputImage.save(output.c_str());//保存
	//outputImage.display();
	//test();
	//outputImage.resize(width, height);
}

void EdgeDetect::test() {
	//img.display("original");
	imgGrayed.display("grayed");
	imgDowned.display("downed");
	imgCannyed.display("cannyed");
	//houghImage.display("hough");
	//edge.display("edge");
	outputImage.display("output");

	int count = 1;
	for (int i = 1; i < 4; i++) {
		int Diagonal = myVertexs[0]->values;
		int A, B, C;
		int x1, y1, x2, y2;
		if (i != Diagonal)	
		{
			x2 = myVertexs[i]->x, y2 = myVertexs[i]->y;
			{	
				x1 = myVertexs[0]->x, y1 = myVertexs[0]->y;
				A = y2 - y1;
				B = x1 - x2;
				C = x2 * y1 - x1 * y2;

				cout << "line" << count << ": ";
				cout << A << " x + " << B << "y + " << C << " = 0 " << endl;
				count++;
			}
			{	
				x1 = myVertexs[Diagonal]->x, y1 = myVertexs[Diagonal]->y;
				A = y2 - y1;
				B = x1 - x2;
				C = x2 * y1 - x1 * y2;
				cout << "line" << count << ": ";
				cout << A << " x + " << B << "y + " << C << " = 0 " << endl;
				count++;
			}
		}
	}
}
CImg<float> EdgeDetect::RGBtoGray(CImg<float> img) {

	CImg<float> grayed = CImg<float>(img._width, img._height, 1, 1);
	cimg_forXY(img, x, y)
	{
		int b = img(x, y, 0);
		int g = img(x, y, 1);
		int r = img(x, y, 2);
		double newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);
		grayed(x, y) = newValue;
	}
	return grayed;
}

void normalizedHistogram(CImg<int> img, double* hist)
{
	int height = img.height();
	int width = img.width();
	int N = height * width;
	CImg<double> histImg = img.histogram(256, 0, 255);
	//histImg.display_graph("Histogram", 3);
	int i = 0;
	cimg_forXY(histImg, x, y)
	{
		hist[i] = histImg(x, y) = double(histImg(x, y) / N);
		//cout << histImg(x, y) << " " << hist[i] << endl;
		i++;
	}
	//histImg.display_graph("Histogram", 3);
}

int otsu(CImg<float>& image, float alpha)
{
	double hist[256];
	normalizedHistogram(image, hist);	//ֱ

	double omega[256];
	double mu[256];

	omega[0] = hist[0];
	mu[0] = 0;
	for (int i = 1; i < 256; i++)
	{
		omega[i] = omega[i - 1] + hist[i]; //
		mu[i] = mu[i - 1] + i * hist[i];
	}
	double mean = mu[255];// 
	double max = 0;
	int k_max = 0;
	for (int k = 1; k < 255; k++)
	{
		double PA = omega[k]; // A
		double PB = 1 - omega[k]; //B
		double value = 0;
		if (fabs(PA) > 0.001 && fabs(PB) > 0.001)
		{
			double MA = mu[k] / PA; //A
			double MB = (mean - mu[k]) / PB;
			value = pow(PA, alpha) * (MA - mean) * (MA - mean) + pow(PB, alpha) * (MB - mean) * (MB - mean);//

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

CImg<float> EdgeDetect::Binarization(CImg<float> srcImg, float alpha)
{
	CImg<float> img = RGBtoGray(srcImg);		
	int otsusValue = otsu(img, alpha);
	cout << "otsusValue = " << otsusValue << endl;
	cimg_forXY(img, x, y)
	{
		img(x, y) = img(x, y) >= otsusValue ? 255 : 0;
	}

	//img = converse(img);
	return img;
}
CImg<float> EdgeDetect::downSampling(CImg<float> img, int num) {
	CImg<float> downed = CImg<float>(img._width / num, img._height / num, 1, 1);
	cimg_forXY(downed, x, y)
	{
		int sum = 0;
		for (int xx = num * x; xx < num * (x + 1); xx++) {
			for (int yy = num * y; yy < num * (y + 1); yy++)
				sum += img(xx, yy);
		}
		downed(x, y) = sum / pow(num, 2);
	}
	return downed;
}

void fillRunVectors(CImg<int>& bwImage, int& NumberOfRuns,
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
void firstPass(vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun, int NumberOfRuns,
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
		if (rowRun[i] != curRowIdx)
		{
			curRowIdx = rowRun[i]; 
			firstRunOnPre = firstRunOnCur;
			lastRunOnPre = i - 1;
			firstRunOnCur = i;
		}
		for (int j = firstRunOnPre; j <= lastRunOnPre; j++)
		{
			if (stRun[i] <= enRun[j] + offset && enRun[i] >= stRun[j] - offset && rowRun[i] == rowRun[j] + 1)
			{
				if (runLabels[i] == 0) 
					runLabels[i] = runLabels[j];
				else if (runLabels[i] != runLabels[j])    
				{
					equivalences.push_back(make_pair(runLabels[i], runLabels[j])); 
					//cout << "runLabels[i]: " << runLabels[i] << " runLabels[j]: " << runLabels[j]<< endl;
				}

			}
		}
		if (runLabels[i] == 0)
		{
			runLabels[i] = idxLabel++;
		}

	}
}
void replaceSameLabel(vector<int>& runLabels, vector<pair<int, int>>& equivalence)
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

void cptConBlocks(CImg<float> result, map<int, vector<Plot>>& blocks)
{
	int NumberOfRuns = 0;				
	vector<int> stRun, enRun, rowRun;	
	vector<int> runLabels;				
	vector<pair<int, int> > equivalences;

	CImg<int> img = result;

	fillRunVectors(img, NumberOfRuns, stRun, enRun, rowRun);
	firstPass(stRun, enRun, rowRun, NumberOfRuns, runLabels, equivalences, 1);
	replaceSameLabel(runLabels, equivalences);	

	cout << "\nrums��" << NumberOfRuns << endl;
	cout << "runLabels:" << runLabels.size() << endl;
	cout << "equivalences :" << equivalences.size() << endl;

	set<int> SetLabel(runLabels.begin(), runLabels.end());
	cout << "set��" << SetLabel.size() << endl;

	for (int i = 0; i < NumberOfRuns; i++) {
		for (int value : SetLabel) {
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
bool isValid(map<int, vector<Plot>>::iterator iter) {
	cout << iter->second.size() << " ";

	if (iter->second.size() <= 1000) return false;
	return true;
}

void deleteBlocks(CImg<float>& img, map<int, vector<Plot>>& blocks) {
	for (auto iter = blocks.begin(); iter != blocks.end(); iter++) {
		if (!isValid(iter)) {
			for (Plot plot : iter->second) {
				img(plot.x, plot.y) = 0;
			}
		}
	}
}
CImg<float> EdgeDetect::canny(CImg<float> img)
{
	//Canny canny(img);
	//img = canny.canny_image();
	//img = canny.canny_line(img, 10);
	//img = canny.delete_line(img,20);
	//return img;
	Concon canny;
	CImg<float> result = canny.canny(img, gFilterX, gFilterY, sigma, thresholdLow, thresholdHigh);

	//Canny canny(img);
	//result = c.canny_line(result, 10);
	//result = c.delete_line(result,200);
	map<int, vector<Plot>> blocks;
	cptConBlocks(result, blocks);
	cout << "hhh\n";
	deleteBlocks(result, blocks);
	return result;

	// gFilterX, gFilterY, sigma, thresholdLow, thresholdHigh);
}

CImg<float> EdgeDetect::houghTransform(CImg<float> img) {
	int width = img._width, height = img._height, maxLength, row, col;
	maxLength = sqrt(pow(width / 2, 2) + pow(height / 2, 2)); 
	int w = maxLength;
	int h = thetaSize;
	CImg<float> houghImage = CImg<float>(w, h);
	houghImage.fill(0);

	cimg_forXY(img, x, y) {
		if (img(x, y) == 255) {
			//int x0 = x / 2, y0 = y / 2 ;
			int x0 = x - width / 2, y0 = height / 2 - y;
			for (int i = 0; i < thetaSize; i++) {
				int rho = x0 * setCos[i] + y0 * setSin[i];
				if (rho >= 0 && rho < maxLength) {
					houghImage(rho, i)++;
				}
			}
		}
	}
	//houghImage.display("HOUGH");
	return houghImage;
}

void EdgeDetect::houghReCompute(CImg<float>& houghImage) {
	int width = houghImage._width, height = houghImage._height, size = windowSize, max;
	for (int i = 0; i < height; i += size / 2) {
		for (int j = 0; j < width; j += size / 2) {
			max = getMaxHough(houghImage, size, i, j);
			for (int y = i; y < i + size; ++y) {
				for (int x = j; x < j + size; ++x) {
					if (houghImage._atXY(x, y) < max) {
						houghImage._atXY(x, y) = 0; 
					}
				}
			}
		}
	}
	int count = 0;
	cimg_forXY(houghImage, x, y) {
		if (houghImage(x, y) != 0) {
			cout << "x:" << x << "  y:" << y << "vote: " << houghImage(x, y) << endl;
			count++;
			if (myLines.empty()) 					
				myLines.push_back(new Line(x, y, houghImage(x, y)));
			else {
				int i;
				for (i = 0; i < myLines.size(); i++) {
					int dotrho = myLines[i]->rho;
					int dottheta = myLines[i]->theta;
					if (abs(x - dotrho) <= 5 && abs(y - dottheta) <= 5) {
						
						myLines[i]->rho = x;
						myLines[i]->theta = y;
						break;	
					}
				}
				if (i == myLines.size()) {
					myLines.push_back(new Line(x, y, houghImage(x, y)));
					cout << "(" << x << "," << y << ")" << endl;
				}
			}
		}
	}
	cout << count << endl;
	//houghImage.display("NEW HOUGH");
}

int EdgeDetect::getMaxHough(CImg<float>& img, int& size, int& y, int& x) {
	int width = (x + size > img._width) ? img._width : x + size;
	int height = (y + size > img._height) ? img._height : y + size;
	int max = 0;
	for (int j = x; j < width; j++) {
		for (int i = y; i < height; i++) {
			max = (img(j, i) >= max) ? img(j, i) : max;
		}
	}
	return max;
}

bool cpm(Line* a, Line* b) {
	return a->weight > b->weight;
}

void EdgeDetect::findEdge(CImg<float>& img) {
	sort(myLines.begin(), myLines.end(), cpm);
	int width = img._width, height = img._height, maxLength;
	edge = CImg<float>(width, height, 1, 1, 0);


	for (int i = 0; i < pointNumber; i++) {
		int theta = myLines[i]->theta, rho = myLines[i]->rho;
		cimg_forXY(edge, x, y) {
			int x0 = x - width / 2, y0 = height / 2 - y;
			if (rho == (int)(x0 * setCos[theta] + y0 * setSin[theta])) {
				edge(x, y) += 255.0 / 2;			
				//imgtemp(x, y, 0, 2) = 255;			
			}
		}
	}
	//edge.display(" edges");	

	//imgtemp.display("imgtemp");
	//outputImage.display("OUTPUTIMAGE");
}
void EdgeDetect::findVertex() {
	unsigned char red[3] = { 255, 0, 0 };
	//img(0,0) - img(width-1,height-1)
	for (int y = 1; y < imgDowned._height - 1; y++) {
		for (int x = 1; x < imgDowned._width - 1; x++) {
			int arr[9];							
			arr[0] = edge(x, y);
			arr[1] = edge(x + 1, y);
			arr[2] = edge(x, y + 1);
			arr[3] = edge(x + 1, y + 1);

			if (arr[0] + arr[1] + arr[2] + arr[3] >= 255.0 * 3 / 2) {


				if (myVertexs.empty())
					myVertexs.push_back(new Vertex(x, y));
				else {
					int i;
					for (i = 0; i < myVertexs.size(); i++) {
						int dotx = myVertexs[i]->x;
						int doty = myVertexs[i]->y;
						if (abs(x - dotx) <= 15 && abs(y - doty) <= 15) {
							myVertexs[i]->x = x;
							myVertexs[i]->y = y;
							break;	
						}
					}
					if (i == myVertexs.size())
						myVertexs.push_back(new Vertex(x, y));
				}
			}
		}
	}
}
bool compare1(Vertex* a, Vertex* b) {
	return a->y < b->y;
}
void sort(vector<Vertex*>& Vertexs) {
	if (Vertexs.size() > 3) {
		if (Vertexs[0]->x > Vertexs[1]->x) {
			Vertex* temp = new Vertex(Vertexs[0]->x, Vertexs[0]->y);
			Vertexs[0] = Vertexs[1];
			Vertexs[1] = temp;
		}
		//2,3
		if (Vertexs[2]->x > Vertexs[3]->x) {
			Vertex* temp = new Vertex(Vertexs[2]->x, Vertexs[2]->y);
			Vertexs[2] = Vertexs[3];
			Vertexs[3] = temp;
		}
	}
}

void EdgeDetect::drawVertexLines(CImg<float>& img, int downSampledSize)
{
	sort(myVertexs.begin(), myVertexs.end(), compare1);
	sort(myVertexs);

	string txtPath = "Vertex4_txt\\";
	CreateFolder(txtPath);
	txtPath = txtPath + name + ".txt";
	cout << "name:" << name << endl;
	cout << "txtPath:" << txtPath << endl;
	ofstream file(txtPath.c_str(), fstream::out);
	if (file) cout << " new file created" << endl;
	else cout << "failed!!\n";

	for (int i = 0; i < myVertexs.size(); i++) {
		cout << i << " " << (myVertexs[i]->x) * DownSampledSize << " " << (myVertexs[i]->y) * DownSampledSize << endl;
		file << (myVertexs[i]->x) * DownSampledSize << " " << (myVertexs[i]->y) * DownSampledSize << endl;
	}
	file.close();

	const double yellow[] = { 255, 255, 0 };
	unsigned char red[3] = { 255, 0, 0 };
	for (int i = 0; i < myVertexs.size(); i++) {
		int x = myVertexs[i]->x;
		int y = myVertexs[i]->y;
		img.draw_circle(x * downSampledSize, y * downSampledSize, 50, red);
	}
	
}