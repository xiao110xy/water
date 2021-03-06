// format_image.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <io.h>
vector<string> getFiles(string folder, string firstname, string lastname)
{
	vector<string> files;
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
				continue;

				/*if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				getFiles(p.assign(folder).append("\\").append(fileinfo.name), files);*/
			}
			else {
				files.push_back(fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return files;
}


int main(int argc,char *argv[])
{
	string folder;
	if (argc != 2)
		folder = "";
	else
		folder = string(argv[1]);
		 
	vector<string> name = getFiles(folder, "", "");
	for (int i = 0; i < name.size(); ++i) {
		string temp = name[i];
		if (temp.size() < 5)
			continue;
		if (temp[temp.size() - 4] != 46)
			continue;
		Mat im = imread(folder+name[i]);
		if (im.data)
			imwrite(folder + name[i], im);
	}

    return 0;
}

