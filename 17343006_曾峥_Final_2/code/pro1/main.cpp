#include "CImg.h"
#include "ImgSeg.h"

#define alpha 1.0

void findGroups(string);

int main() {
	clock_t start, end;
	start = clock();
	string fileFolderName = "C:\\Users\\inplus\\Desktop\\data\\cutImg\\";
	findGroups(fileFolderName);
	end = clock();
	cout << "\nTotal time: " << (end - start) / CLOCKS_PER_SEC << "s" << endl;
	cout << "\nAverage time: " << (end - start) / CLOCKS_PER_SEC / 80 << "s" << endl;
}

void findGroups(string fileFolderName) {
	clock_t start, end;
	start = clock();
	vector<string> files;
	GetAllFiles(fileFolderName.c_str(), files);

	string resultPath = "C:\\Users\\inplus\\Desktop\\data\\result\\";
	CreateFolder(resultPath);
	for (auto i : files) {
		//if (i == "IMG_20191204_001722.bmp")	
		ImgSeg object;
		//else if (i == ".bmp") distance = 300;
		//else if()
		//if(i == "IMG_20191204_013949.bmp")
		object.findTarget(i, fileFolderName + i, resultPath + i);	//≤‚ ‘ƒ£ Ω
	}
	end = clock();
	cout << "\nTotal time for finding Groups: " << (end - start) / CLOCKS_PER_SEC << "s" << endl;
}
