#pragma once
#include <QDialog>
#include <QProgressDialog>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"  
using namespace cv;
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
using namespace std;
class tools
{
public:
	tools();
	~tools();
	int extract_video(string file_all_name, string file_path, string base_name);
};

