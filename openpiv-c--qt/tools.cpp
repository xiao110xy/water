#include "tools.h"



tools::tools()
{
}


tools::~tools()
{
}

int tools::extract_video(string file_all_name, string file_path, string base_name)
{	
	//打开视频文件：其实就是建立一个VideoCapture结构  
	VideoCapture capture;
	//检测是否正常打开:成功打开时，isOpened返回ture  
	if (!capture.open(file_all_name)) {
		cout << "video name error!" << endl;
		return -1;
	}
	//获取整个帧数  
	long totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
	//设置开始帧()  
	long frameToStart = 0;
	capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
	//获取帧率  
	double rate = capture.get(CV_CAP_PROP_FPS);
	//承载每一帧的图像  
	Mat frame;
	QProgressDialog process;
	process.setLabelText("processing...");
	process.setRange(0, totalFrameNumber);
	process.setModal(true);
	process.setCancelButtonText("cancel");

	for (int i = 0; i < totalFrameNumber; ++i) {
		if (!capture.read(frame))
			break;
		Mat image;
		transpose(frame, image);
		ostringstream temp_string;
		temp_string.fill('0');
		temp_string.width(5);
		temp_string << i;
		string temp = file_path + "/" + base_name + "_" + temp_string.str() + ".jpg";
		imwrite(temp, image);
		process.setValue(i);
		if (process.wasCanceled())
			break;
	}
	//关闭视频文件
	capture.release();
	return 0;
}
