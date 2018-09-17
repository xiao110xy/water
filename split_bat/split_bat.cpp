// split_bat.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;
int main()
{
	string path = "E:/water_line/bat/3";
	fstream in_name(path+".bat");
	if (!in_name)
		return false;
	string temp_name;
	int n = 0;
	vector<string> names;
	while (!in_name.eof()) {
		getline(in_name, temp_name);
		names.push_back(temp_name);
		if (names.size() == 100) {
			ofstream out_name(path + "_temp_" + to_string(n) + ".bat");
			for (auto i : names) {
				out_name << i << endl;
			}
			out_name.close();
			++n;
			names.clear();

		}
	}
	if (names.size() >0) {
		ofstream out_name(path + "_temp_" + to_string(n) + ".bat");
		for (auto i : names) {
			out_name << i << endl;
		}
		out_name.close();
		++n;
	}

}
