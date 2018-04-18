#include "stdafx.h"
#include "xy.h"
Mat xy_Canny(Mat src, double low_thresh, double high_thresh, int aperture_size, bool L2gradient)
{
	Mat dst = Mat::zeros(src.size(),CV_8UC1);		   //输出图像，为单通道黑白图  
	// low_thresh 表示低阈值， high_thresh表示高阈值  
	// aperture_size 表示算子大小，默认为3  
	// L2gradient计算梯度幅值的标识，默认为false	 

	// 如果L2gradient为false 并且 apeture_size的值为-1（-1的二进制标识为：1111 1111）	 
	// L2gradient为false 则计算sobel导数时，用G = |Gx|+|Gy|	
	// L2gradient为true	则计算sobel导数时，用G = Math.sqrt((Gx)^2 + (Gy)^2) 根号下 开平方	 

	if (!L2gradient && (aperture_size & CV_CANNY_L2_GRADIENT) == CV_CANNY_L2_GRADIENT)
	{
		// CV_CANNY_L2_GRADIENT 宏定义其值为： Value = (1<<31) 1左移31位	即2147483648	 
		//backward compatibility  
		// ~标识按位取反	
		aperture_size &= ~CV_CANNY_L2_GRADIENT;//相当于取绝对值  
		L2gradient = true;
	}


	// 判别条件1：aperture_size是奇数  
	// 判别条件2: aperture_size的范围应当是[3,7], 默认值3   
	if ((aperture_size & 1) == 0 || (aperture_size != -1 && (aperture_size < 3 || aperture_size > 7)))
		CV_Error(CV_StsBadFlag, "");  // 报错	 

	if (low_thresh > high_thresh)			// 如果低阈值 > 高阈值	
		std::swap(low_thresh, high_thresh); // 则交换低阈值和高阈值  

	const int cn = src.channels();			 // cn为输入图像的通道数	 
	Mat dx(src.rows, src.cols, CV_16SC(cn)); // 存储 x方向 方向导数的矩阵，CV_16SC(cn)：16位有符号cn通道  
	Mat dy(src.rows, src.cols, CV_16SC(cn)); // 存储 y方向 方向导数的矩阵 ......  

	/*Sobel参数说明：(参考cvSobel)
	cvSobel(
	const  CvArr* src,				  // 输入图像
	CvArr*		  dst,				  // 输入图像
	int			  xorder，			 // x方向求导的阶数
	int			  yorder，		  // y方向求导的阶数
	int			  aperture_size = 3	  // 滤波器的宽和高 必须是奇数
	);
	*/

	// BORDER_REPLICATE 表示当卷积点在图像的边界时，原始图像边缘的像素会被复制，并用复制的像素扩展原始图的尺寸	 
	// 计算x方向的sobel方向导数，计算结果存在dx中  
	Sobel(src, dx, CV_16S, 1, 0, aperture_size, 1, 0, cv::BORDER_REPLICATE);
	// 计算y方向的sobel方向导数，计算结果存在dy中  
	Sobel(src, dy, CV_16S, 0, 1, aperture_size, 1, 0, cv::BORDER_REPLICATE);

	//L2gradient为true时， 表示需要根号下开平方运算，阈值也需要平方  
	if (L2gradient)
	{
		low_thresh = std::min(32767.0, low_thresh);
		high_thresh = std::min(32767.0, high_thresh);

		if (low_thresh > 0) low_thresh *= low_thresh;	 //低阈值平方运算	
		if (high_thresh > 0) high_thresh *= high_thresh; //高阈值平方运算	
	}

	int low = cvFloor(low_thresh);	 // cvFloor返回不大于参数的最大整数值, 相当于取整	 
	int high = cvFloor(high_thresh);

	// ptrdiff_t 是C/C++标准库中定义的一个数据类型，signed类型，通常用于存储两个指针的差（距离），可以是负数  
	// mapstep 用于存放	 
	ptrdiff_t mapstep = src.cols + 2; // +2 表示左右各扩展一条边	

	// AutoBuffer<uchar> 会自动分配一定大小的内存，并且指定内存中的数据类型是uchar  
	// 列数 +2 表示图像左右各自扩展一条边 （用于复制边缘像素，扩大原始图像）  
	// 行数 +2 表示图像上下各自扩展一条边	
	AutoBuffer<uchar> buffer((src.cols + 2)*(src.rows + 2) + cn * mapstep * 3 * sizeof(int));

	int* mag_buf[3];  //定义一个大小为3的int型指针数组，	
	mag_buf[0] = (int*)(uchar*)buffer;
	mag_buf[1] = mag_buf[0] + mapstep * cn;
	mag_buf[2] = mag_buf[1] + mapstep * cn;
	memset(mag_buf[0], 0, /* cn* */mapstep * sizeof(int));

	uchar* map = (uchar*)(mag_buf[2] + mapstep * cn);
	memset(map, 1, mapstep);
	memset(map + mapstep * (src.rows + 1), 1, mapstep);

	int maxsize = std::max(1 << 10, src.cols * src.rows / 10); // 2的10次幂 1024  
	std::vector<uchar*> stack(maxsize); // 定义指针类型向量，用于存地址  
	uchar **stack_top = &stack[0];		// 栈顶指针（指向指针的指针），指向stack[0], stack[0]也是一个指针	 
	uchar **stack_bottom = &stack[0];	// 栈底指针 ，初始时 栈底指针 == 栈顶指针  


	// 梯度的方向被近似到四个角度之一 (0, 45, 90, 135 四选一)	 
	/* sector numbers
	(Top-Left Origin)

	1	2	 3
	 *	*  *
	  * * *
	0******0
	  * * *
	 *	* *  *
	3	2	 1
	*/


	// define 定义函数块	 
	// CANNY_PUSH(d) 是入栈函数， 参数d表示地址指针，让该指针指向的内容为2（int型强制转换成uchar型），并入栈，栈顶指针+1  
	// 2表示 像素属于某条边缘 可以看下方的注释  
	// CANNY_POP(d) 是出栈函数， 栈顶指针-1，然后将-1后的栈顶指针指向的值，赋给d  
#define CANNY_PUSH(d)	 *(d) = uchar(2), *stack_top++ = (d)  
#define CANNY_POP(d)	 (d) = *--stack_top	 

	// calculate magnitude and angle of gradient, perform non-maxima suppression.  
	// fill the map with one of the following values:  
	// 0 - the pixel might belong to an edge 可能属于边缘	 
	// 1 - the pixel can not belong to an edge 不属于边缘  
	// 2 - the pixel does belong to an edge 一定属于边缘	

	// for内进行非极大值抑制 + 滞后阈值处理  
	for (int i = 0; i <= src.rows; i++) // i 表示第i行	
	{

		// i == 0 时，_norm 指向 mag_buf[1]	 
		// i > 0 时， _norm 指向 mag_buf[2]	 
		// +1 表示跳过每行的第一个元素，因为是后扩展的边，不可能是边缘	
		int* _norm = mag_buf[(i > 0) + 1] + 1;

		if (i < src.rows)
		{
			short* _dx = dx.ptr<short>(i); // _dx指向dx矩阵的第i行	 
			short* _dy = dy.ptr<short>(i); // _dy指向dy矩阵的第i行	 

			if (!L2gradient) // 如果 L2gradient为false	 
			{
				for (int j = 0; j < src.cols*cn; j++) // 对第i行里的每一个值都进行计算  
					_norm[j] = std::abs(int(_dx[j])) + std::abs(int(_dy[j])); // 用||+||计算  
			}
			else
			{
				for (int j = 0; j < src.cols*cn; j++)
					//用平方计算,当 L2gradient为 true时，高低阈值都被平方了，所以此处_norm[j]无需开平方	 
					_norm[j] = int(_dx[j])*_dx[j] + int(_dy[j])*_dy[j]; //	
			}

			if (cn > 1) // 如果不是单通道	
			{
				for (int j = 0, jn = 0; j < src.cols; ++j, jn += cn)
				{
					int maxIdx = jn;
					for (int k = 1; k < cn; ++k)
						if (_norm[jn + k] > _norm[maxIdx]) maxIdx = jn + k;
					_norm[j] = _norm[maxIdx];
					_dx[j] = _dx[maxIdx];
					_dy[j] = _dy[maxIdx];
				}
			}
			_norm[-1] = _norm[src.cols] = 0; // 最后一列和第一列的梯度幅值设置为0  
		}
		// 当i == src.rows （最后一行）时，申请空间并且每个空间的值初始化为0, 存储在mag_buf[2]中	 
		else
			memset(_norm - 1, 0, /* cn* */mapstep * sizeof(int));

		// at the very beginning we do not have a complete ring	 
		// buffer of 3 magnitude rows for non-maxima suppression  
		if (i == 0)
			continue;

		uchar* _map = map + mapstep * i + 1; // _map 指向第 i+1 行，+1表示跳过该行第一个元素	
		_map[-1] = _map[src.cols] = 1; // 第一列和最后一列不是边缘，所以设置为1  

		int* _mag = mag_buf[1] + 1; // take the central row 中间那一行  
		ptrdiff_t magstep1 = mag_buf[2] - mag_buf[1];
		ptrdiff_t magstep2 = mag_buf[0] - mag_buf[1];

		const short* _x = dx.ptr<short>(i - 1);
		const short* _y = dy.ptr<short>(i - 1);

		// 如果栈的大小不够，则重新为栈分配内存（相当于扩大容量）  
		if ((stack_top - stack_bottom) + src.cols > maxsize)
		{
			int sz = (int)(stack_top - stack_bottom);
			maxsize = maxsize * 3 / 2;
			stack.resize(maxsize);
			stack_bottom = &stack[0];
			stack_top = stack_bottom + sz;
		}

		int prev_flag = 0; //前一个像素点 0：非边缘点 ；1：边缘点  
		for (int j = 0; j < src.cols; j++) // 第 j 列	 
		{
			#define CANNY_SHIFT 15	
			// tan22.5	
			const int TG22 = (int)(0.4142135623730950488016887242097*(1 << CANNY_SHIFT) + 0.5);

			int m = _mag[j];

			if (m > low) // 如果大于低阈值	 
			{
				int xs = _x[j];	   // dx中 第i-1行 第j列	 
				int ys = _y[j];	   // dy中 第i-1行 第j列	 
				int x = std::abs(xs);
				int y = std::abs(ys) << CANNY_SHIFT;

				int tg22x = x * TG22;

				if (y < tg22x) //角度小于22.5 用区间表示：[0, 22.5)  
				{
					// 与左右两点的梯度幅值比较，如果比左右都大  
					//（此时当前点是左右邻域内的极大值），则 goto __ocv_canny_push 执行入栈操作  
					if (m > _mag[j - 1] && m >= _mag[j + 1]) goto __ocv_canny_push;
				}
				else //角度大于22.5	 
				{
					int tg67x = tg22x + (x << (CANNY_SHIFT + 1));
					if (y > tg67x) //(67.5, 90)	 
					{
						//与上下两点的梯度幅值比较，如果比上下都大	
						//（此时当前点是左右邻域内的极大值），则 goto __ocv_canny_push 执行入栈操作	
						if (m > _mag[j + magstep2] && m >= _mag[j + magstep1]) goto __ocv_canny_push;
					}
					else //[22.5, 67.5]	 
					{
						// ^ 按位异或 如果xs与ys异号 则取-1 否则取1  
						int s = (xs ^ ys) < 0 ? -1 : 1;
						//比较对角线邻域  
						if (m > _mag[j + magstep2 - s] && m > _mag[j + magstep1 + s]) goto __ocv_canny_push;
					}
				}
			}

			//比当前的梯度幅值低阈值还低，直接被确定为非边缘	 
			prev_flag = 0;
			_map[j] = uchar(1); // 1 表示不属于边缘  

			continue;
		__ocv_canny_push:
			// 前一个点不是边缘点 并且 当前点的幅值大于高阈值（大于高阈值被视为边缘像素） 并且 正上方的点不是边缘点	 
			if (!prev_flag && m > high && _map[j - mapstep] != 2)
			{
				//将当前点的地址入栈，入栈前，会将该点地址指向的值设置为2（查看上面的宏定义函数块里）  
				CANNY_PUSH(_map + j);
				prev_flag = 1;
			}
			else
				_map[j] = 0;
		}

		// scroll the ring buffer  
		// 交换指针指向的位置，向上覆盖，把mag_[1]的内容覆盖到mag_buf[0]上	 
		// 把mag_[2]的内容覆盖到mag_buf[1]上  
		// 最后 让mag_buf[2]指向_mag指向的那一行  
		_mag = mag_buf[0];
		mag_buf[0] = mag_buf[1];
		mag_buf[1] = mag_buf[2];
		mag_buf[2] = _mag;
	}

	const uchar* pmap = map + mapstep + 1;
	uchar* pdst = dst.ptr();
	for (int i = 0; i < src.rows; i++, pmap += mapstep, pdst += dst.step)
	{
		for (int j = 0; j < src.cols; j++) {
			if (pmap[j] == 1)
				continue;
			else {
				pdst[j] = 125 + 50 * pmap[j];
			}
		}
	}
	// now track the edges (hysteresis thresholding)  
	// 通过上面的for循环，确定了各个邻域内的极大值点为边缘点（标记为2）  
	// 现在，在这些边缘点的8邻域内（上下左右+4个对角）,将可能的边缘点（标记为0）确定为边缘	 
	while (stack_top > stack_bottom)
	{
		uchar* m;
		if ((stack_top - stack_bottom) + 8 > maxsize)
		{
			int sz = (int)(stack_top - stack_bottom);
			maxsize = maxsize * 3 / 2;
			stack.resize(maxsize);
			stack_bottom = &stack[0];
			stack_top = stack_bottom + sz;
		}

		CANNY_POP(m); // 出栈	 

		if (!m[-1])			CANNY_PUSH(m - 1);
		if (!m[1])			CANNY_PUSH(m + 1);
		if (!m[-mapstep - 1]) CANNY_PUSH(m - mapstep - 1);
		if (!m[-mapstep])	CANNY_PUSH(m - mapstep);
		if (!m[-mapstep + 1]) CANNY_PUSH(m - mapstep + 1);
		if (!m[mapstep - 1])	CANNY_PUSH(m + mapstep - 1);
		if (!m[mapstep])	CANNY_PUSH(m + mapstep);
		if (!m[mapstep + 1])	CANNY_PUSH(m + mapstep + 1);
	}

	 //the final pass, form the final image	 
	 //生成边缘图  
	//const uchar* pmap = map + mapstep + 1;
	//uchar* pdst = dst.ptr();
	//for (int i = 0; i < src.rows; i++, pmap += mapstep, pdst += dst.step)
	//{
	//	for (int j = 0; j < src.cols; j++)
	//		pdst[j] = (uchar)-(pmap[j] >> 1);
	//}
	return dst;
}

Mat Edge_Detect(Mat im, int aperture_size)
{
	Mat test_im;
	//
	Mat data,result;
	// 灰度图像
	if (im.channels() > 1)
		cvtColor(im, data, CV_BGR2GRAY);
	else
		data = im.clone();
	// BORDER_REPLICATE 表示当卷积点在图像的边界时，原始图像边缘的像素会被复制，并用复制的像素扩展原始图的尺寸  
	// 计算x方向的sobel方向导数，计算结果存在dx中  
	// 计算y方向的sobel方向导数，计算结果存在dy中  
	Mat dx, dy;
	Sobel(im, dx, CV_32F, 1, 0, aperture_size, 1, 0, cv::BORDER_REPLICATE);
	Sobel(im, dy, CV_32F, 0, 1, aperture_size, 1, 0, cv::BORDER_REPLICATE);
	// dy方向局部极大极小值 找出极小值，并进行处理
	vector<Point3f> points1 = localmax_point_score(dx, dx.cols / 3, dx.cols / 3 * 2, 3, 0);
	vector<Point3f> points2 = localmax_point_score(-dx, dx.cols / 3, dx.cols / 3 * 2, 3, 0);
	// 筛选points
	vector<vector<vector<Point3f>>> E_area1 = select_pointline(dx, dy, points1, true);
	vector<vector<vector<Point3f>>> E_area2 = select_pointline(dx, dy, points2, false);

	Mat temp[3];
	temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
	merge(temp, 3, test_im);
	Scalar rgb[3];
	rgb[0] = Scalar(0, 255, 0);
	rgb[1] = Scalar(0, 0, 255);
	rgb[2] = Scalar(255, 0,0);
	for (auto i:E_area1) {
		for (int j = 0;j<3;++j)
			for (auto k : i[j]) {
				test_im.at<Vec3b>(k.y, k.x).val[0] = rgb[j].val[0];
				test_im.at<Vec3b>(k.y, k.x).val[1] = rgb[j].val[1];
				test_im.at<Vec3b>(k.y, k.x).val[2] = rgb[j].val[2];
			}
	}
	for (auto i:E_area2) {
		for (int j = 0;j<3;++j)
			for (auto k : i[j]) {
				test_im.at<Vec3b>(k.y, k.x).val[0] = rgb[j].val[0];
				test_im.at<Vec3b>(k.y, k.x).val[1] = rgb[j].val[1];
				test_im.at<Vec3b>(k.y, k.x).val[2] = rgb[j].val[2];
			}
	}



	// 去除非中心区域
	return Mat();
}

vector<vector<vector<Point3f>>> select_pointline(Mat dx, Mat dy, vector<Point3f> points, bool flag)
{
	Mat point_scoreimage = flag?dx.clone():-dx.clone();
	Mat line_scoreimage = dy.clone();
	vector<vector<vector<Point3f>>> result;
	int dety = line_scoreimage.cols / 6;
	dety = dety < 3 ? 3 : dety;
	int detx = line_scoreimage.cols / 4;
	Mat flag_image = Mat::zeros(point_scoreimage.size(), CV_8UC1);
	for (auto &i : points) {
		if (i.y<3 || i.y>point_scoreimage.rows - 4)
			continue;
		float score_t = i.z*0.5;
		Mat temp_flag_image = flag_image.clone();
		vector<Point3f> temp_point,temp_line1,temp_line2;
		temp_point = cluster_point(point_scoreimage, score_t, Point(i.x, i.y), temp_flag_image.clone(),4);
		temp_point = cluster_point(temp_point, true, temp_flag_image);
		if (temp_point.size() < 1)
			continue;
		Mat temp_data;
		Point index; double maxval;
		int x1, x2,y1,y2;
		x1 = flag ? i.x-detx : i.x + 1;
		x2 = flag ? i.x : i.x + 1+detx;
		if (x1<0 || x2>line_scoreimage.cols)
			continue;
		y1 = i.y - dety>=0?i.y-dety:0;
		y2 = i.y + dety <=line_scoreimage.rows ? i.y + dety : line_scoreimage.rows;
		temp_data = -line_scoreimage(Range(y1, i.y), Range(x1, x2)).clone();
		minMaxLoc(temp_data, NULL,NULL, NULL, &index);
		float score_t1 = temp_data.at<float>(index.y, index.x)*0.5;
		temp_line1 = cluster_point(-line_scoreimage.clone(), score_t, Point(index.x+x1, index.y+y1), temp_flag_image.clone(),8);
		temp_line1 = cluster_point(temp_line1, false, temp_flag_image);
		if (temp_line1.size() < 1)
			continue;
		temp_data = line_scoreimage(Range(i.y + 1, y2), Range(x1, x2)).clone();
		minMaxLoc(temp_data, NULL,NULL, NULL, &index);
		float score_t2 = temp_data.at<float>(index.y, index.x)*0.5;
		temp_line2 = cluster_point(line_scoreimage.clone(), score_t2, Point(index.x+x1, index.y+i.y+1), temp_flag_image.clone(),8);
		temp_line2 = cluster_point(temp_line2, false, temp_flag_image);
		if (temp_line2.size() < 1 )
			continue;
		vector<vector<Point3f>> temp_result = new_point_line1_line2(dx,dy,temp_point, temp_line1, temp_line2,flag);
		if (temp_result.size() != 3)
			continue;
		if (temp_result[0].size() >  line_scoreimage.cols / 4|| 
			temp_result[0].size()>temp_result[1].size()|| temp_result[0].size()>temp_result[2].size())
			continue;
		int temp_value = 255;
		for (auto i : temp_result) {
			for(auto j:i)
				flag_image.at<uchar>(j.y, j.x) = temp_value;
			temp_value -= 100;
		}
		result.push_back(temp_result);
	}
	return result;
}

vector<Point3f> cluster_point(Mat score_image, float score_t,Point point, Mat flag,int number)
{
	if (score_t<50)
		return vector<Point3f>();
	vector<Point3f> result;
	int x = point.x;
	int y = point.y;
	if (x<0||y<0||x>score_image.cols-1||y>score_image.rows-1||flag.at<uchar>(y,x)!=0)
		return vector<Point3f>();
	flag.at<uchar>(y, x) = 1;
	Point3f temp_point;
	temp_point.x = (float)point.x;
	temp_point.y = (float)point.y;
	temp_point.z = score_image.at<float>(y, x);
	if (temp_point.z < score_t)
		return vector<Point3f>();
	// 四邻域
	result.push_back(temp_point);
	vector<Point3f> temp;
	temp = cluster_point(score_image, score_t, Point(point.x, point.y-1),flag,number);
	result.insert(result.end(), temp.begin(), temp.end());
	temp = cluster_point(score_image, score_t, Point(point.x, point.y+1),flag, number);
	result.insert(result.end(), temp.begin(), temp.end());
	temp = cluster_point(score_image, score_t, Point(point.x + 1, point.y), flag, number);
	result.insert(result.end(), temp.begin(), temp.end());
	temp = cluster_point(score_image, score_t, Point(point.x - 1, point.y), flag, number);
	if (number > 4) {
		result.insert(result.end(), temp.begin(), temp.end());
		temp = cluster_point(score_image, score_t, Point(point.x-1, point.y-1),flag, number);
		result.insert(result.end(), temp.begin(), temp.end());
		temp = cluster_point(score_image, score_t, Point(point.x+1, point.y-1),flag, number);
		result.insert(result.end(), temp.begin(), temp.end());
		temp = cluster_point(score_image, score_t, Point(point.x + 1, point.y+1), flag, number);
		result.insert(result.end(), temp.begin(), temp.end());
		temp = cluster_point(score_image, score_t, Point(point.x - 1, point.y+1), flag, number);
		result.insert(result.end(), temp.begin(), temp.end());
	}
	return result;
}

vector<Point3f> cluster_point(vector<Point3f> data, bool flag,Mat &flag_image)
{
	if (flag) {
		for (auto &i : data)
			std::swap(i.x, i.y);
	}
	stable_sort(data.begin(), data.end(),
		[](const Point3f&a, const Point3f&b) {return a.x < b.x; });
	for (int i = 0; i < data.size(); ++i) {
		if (data[i].x >= 1) {
			data.erase(data.begin(), data.begin() + i);
			break;
		}
	}
	vector<Point3f> result;
	for (auto i : data) {
		if (result.size() == 0)
			result.push_back(i);
		else if (result[result.size() - 1].x == i.x&&result[result.size() - 1].z < i.z) {
			result.pop_back();
			result.push_back(i);
		}
		else if(result[result.size() - 1].x != i.x){
			result.push_back(i);
		}
	}
	if (flag) {
		for (auto &i : result)
			std::swap(i.x, i.y);
	}
	for (auto i : result)
		flag_image.at<uchar>(i.y, i.x) = flag?255:125;
	return result;
}

vector<vector<Point3f>> new_point_line1_line2(Mat dx,Mat dy,vector<Point3f> point, vector<Point3f> line1_point, vector<Point3f> line2_point,bool flag)
{
	// 竖直直线的x坐标
	int x = 0;
	map<int,int> temp;
	int max = 0;
	for (auto i : point) {
		++temp[(int)i.x];
	}
	for (auto i : temp) {
		if (i.second > max) {
			x = i.first;
			max = i.second;
		}
		if (i.second == max && flag&&i.first < x)
			x = i.first;
		if (i.second == max && !flag&&i.first > x)
			x = i.first;

	}
	// 直线拟合
	vector<Point2f> temp_f_point;
	Point2f point_first(9999,0), point_end(-9999, 0);
	vector<Point> temp_point,temp_line1_point,temp_line2_point;
	Vec4f point_para, line1_para, line2_para;
	float mindx1 = 999;
	for (auto i : line1_point) {
		if (i.x <= x && flag) {
			temp_line1_point.push_back(Point((int)i.x,(int)i.y));
			mindx1 = mindx1 < abs(i.x - x) ? mindx1 : abs(i.x - x);
		}
		if (i.x >= x && !flag) {
			temp_line1_point.push_back(Point((int)i.x,(int)i.y));
			mindx1 = mindx1 < abs(i.x - x) ? mindx1 : abs(i.x - x);
		}
	}
	if (temp_line1_point.size()<2||mindx1>3)
		return vector<vector<Point3f>>();
	//
	float mindx2 = 999;
	for (auto i : line2_point) {
		if (i.x <= x && flag) {
			temp_line2_point.push_back(Point((int)i.x,(int)i.y));
			mindx2 = mindx2 < abs(i.x - x) ? mindx2 : abs(i.x - x);
		}

		if (i.x >= x && !flag) {
			temp_line2_point.push_back(Point((int)i.x,(int)i.y));
			mindx2 = mindx2 < abs(i.x - x) ? mindx2 : abs(i.x - x);
		}
	}
	if (temp_line2_point.size()<2 || mindx2>3)
		return vector<vector<Point3f>>();
	//
	fitLine(temp_line1_point, line1_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	temp_f_point.push_back(Point2f((float)x,
		(x - line1_para[2]) * line1_para[1] / line1_para[0] + line1_para[3]));
	for (auto i : temp_line1_point) {
		float d = (i.x - line1_para[2])*line1_para[0] + (i.y - line1_para[3])*line1_para[1];
		float x = d * line1_para[0] + line1_para[2];
		float y = d * line1_para[1] + line1_para[3];
		temp_f_point.push_back(Point2f(x, y));
	}
	for (auto i : temp_f_point) {
		if (i.x < point_first.x)
			point_first = i;
		if (i.x > point_end.x)
			point_end = i;
	}
	temp_line1_point = get_line_point(point_first, point_end);
	fitLine(temp_line2_point, line2_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	temp_f_point.clear();
	temp_f_point.push_back(Point2f((float)x,
		(x - line2_para[2]) * line2_para[1] / line2_para[0] + line2_para[3]));
	for (auto i : temp_line2_point) {
		float d = (i.x - line2_para[2])*line2_para[0] + (i.y - line2_para[3])*line2_para[1];
		float x = d * line2_para[0] + line2_para[2];
		float y = d * line2_para[1] + line2_para[3];
		temp_f_point.push_back(Point2f(x, y));
	}
	point_first.x = 9999;
	point_end.x = -9999;
	for (auto i : temp_f_point) {
		if (i.x < point_first.x)
			point_first = i;
		if (i.x > point_end.x)
			point_end = i;
	}
	temp_line2_point = get_line_point(point_first, point_end);
	point_first.y = 9999;
	point_end.y = -9999;
	if (flag) {
		point_first = *(temp_line1_point.end() - 1);
		point_end = *(temp_line2_point.end() - 1);
	}
	else {
		point_first = temp_line1_point[0];
		point_end = temp_line2_point[0];
	}
	temp_point = get_line_point(point_first, point_end);
	vector<vector<Point3f>> temp_result;
	point.clear();
	line1_point.clear();
	line2_point.clear();
	for (auto i : temp_point)
		point.push_back(Point3f((float)i.x, (float)i.y, dx.at<float>(i.y, i.x)));
	for (auto i : temp_line1_point)
		line1_point.push_back(Point3f((float)i.x, (float)i.y, dy.at<float>(i.y, i.x)));
	for (auto i : temp_line2_point)
		line2_point.push_back(Point3f((float)i.x, (float)i.y, dy.at<float>(i.y, i.x)));
	temp_result.push_back(point);
	temp_result.push_back(line1_point);
	temp_result.push_back(line2_point);
	return temp_result;
}
