#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <io.h>
#include <map>
using namespace std;
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/objdetect/objdetect.hpp"
using namespace cv;

vector<string> getFiles(string folder, string firstname, string lastname, vector<string> &folders)
{
	folders.clear();
	vector<string> files,temp_files1,temp_files2;
	//文件句柄  
	long long hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;   //大家可以去查看一下_finddata结构组成                            
								   //以及_findfirst和_findnext的用法，了解后妈妈就再也不用担心我以后不会编了  
	string p(folder);
	if (folder == "")
		p.append(".");
	p.append("\\");
	p.append(firstname);
	p.append("*");
	p.append(lastname);
	if ((hFile = _findfirst(p.c_str(), &fileinfo)) != -1) {
		do {
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib & _A_SUBDIR)) {
				//continue;

				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					temp_files1 = getFiles(p.assign(folder).append("\\").append(fileinfo.name), firstname, lastname, temp_files2);
				files.insert(files.end(), temp_files1.begin(), temp_files1.end());
				folders.insert(folders.end(), temp_files2.begin(), temp_files2.end());
			}
			else {
				files.push_back(fileinfo.name);
				folders.push_back(folder);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return files;
}


int main(int argc, char**argv) {
	if (argc != 4) {
		cout << "error";
		return -1;
	}
	string folder1(argv[1]);
	string folder2(argv[2]);
	string ignore_txt(argv[3]);
	vector<string> ignore_files;
	fstream file1(ignore_txt);
	if (file1) {
		string temp_name;
		while (getline(file1, temp_name)) {
			if (temp_name!="")
				ignore_files.push_back(temp_name);
		}

	}
	file1.close();
	vector<string> folders;
	vector<string> files = getFiles(folder1, "", "", folders);

	for (int i = 0; i < ignore_files.size(); ++i) {
		for (int j = 0; j < files.size(); ++j) {
			if (ignore_files[i] == files[j]) {
				files.erase(files.begin() + j, files.begin() + j + 1);
				folders.erase(folders.begin() + j, folders.begin() + j + 1);
			}
		}
	}
	ofstream file2(ignore_txt, ios::app);
	for (int i = 0; i < files.size(); ++i) {
		string temp = files[i];
		if (temp.size() < 5)
			continue;
		if (temp.size() > 6) {
			if (string(temp.begin(), temp.begin() + 6) == "result")
				continue;
		}
		if (temp[temp.size() - 4] != 46)
			continue;
		Mat im = imread(folders[i] + "/" + temp);
		if (!im.data)
			continue;
		imwrite(folder2 + "/" + temp, im);
		file2 << temp << endl;
	}
	file2.close();
	return 0;

}