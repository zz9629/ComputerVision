#include "EdgeDetect.h"
#include "Tool.h"
#include "concon.h"
#include <cstdlib>
#include <time.h>

void getVertext() {
	/*获得所有图片*/
	vector<string> files;
	string fileFolderName = "C:\\Users\\inplus\\Desktop\\Project1\\Project1\\Dataset\\";
	GetAllFiles(fileFolderName.c_str(), files);	//得到所有图片	
	string tempImgPath = "C:\\Users\\inplus\\Desktop\\Project1\\Project1\\temp\\";
	CreateFolder(tempImgPath);					//创建输出文件夹
	for (auto i : files) {
		//IMG_20191204_000806.bmp
		string path = fileFolderName + i;
		string pathTo = tempImgPath + i;
		EdgeDetect ed;
		cout << path << endl;
		cout << "i:" << i << endl;
		//if (i == "IMG_20191204_001325.bmp")
			ed.edgeDetect(i, path, pathTo, 4);
	}
}

int main() {
	clock_t start, end;
	start = clock();
	// 1.获得顶点
	getVertext();
	EdgeDetect ed;
	//ed.edgeDetect("name", "C:\\Users\\inplus\\Desktop\\test.bmp", "C:\\Users\\inplus\\Desktop\\ret.bmp", 4);
	////ed3.edgeDetect("Dataset/IMG_20191204_001419.bmp", "result/IMG_20191204_001419.bmp", 4);		//需要腐蚀4
	// 2.python根据顶点在原图中切割出名片
	// 3.拿到规则的图像后，寻找主要部分
	end = clock();
	cout << "Total time : " << (end - start)/ CLOCKS_PER_SEC << "s" << endl;
	system("pause");
	return 0;
}

	
		