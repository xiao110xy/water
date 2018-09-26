//


# define MAX_RETINEX_SCALES    8     /* Retinex最多可采用的尺度的数目 */   
# define MIN_GAUSSIAN_SCALE   16     /* 最小Gaussian尺度 */   
# define MAX_GAUSSIAN_SCALE  250     /* 最大Gaussian尺度 */   

typedef struct
{
	int     scale;         /* 最大Retinex尺度 */
	int     nscales;       /* 尺度个数        */
	int     scales_mode;   /* Retinex尺度计算模式，有3种：UNIFORM, LOW, HIGH */
	float   cvar;          /* 用于调整色彩动态范围的方差的倍乘系数           */
} RetinexParams;

/* 3种Retinex尺度计算模式，均匀，低和高，它们决定RetinexScales中的尺度数据 */
# define RETINEX_UNIFORM 0   
# define RETINEX_LOW     1   
# define RETINEX_HIGH    2   

/* 多尺度Retinex中需要的各个Retinex尺度保存在下面数组中 */
static float RetinexScales[MAX_RETINEX_SCALES];

typedef struct
{
	int    N;
	float  sigma;
	double B;
	double b[4];
} gauss3_coefs;

/*
 * Private variables.
 */
static RetinexParams rvals =
{
  240,             /* Scale */
  3,               /* Scales */
  RETINEX_UNIFORM, /* Retinex processing mode */
  1.2f             /* A variant */
};

# define clip( val, minv, maxv )    (( val = (val < minv ? minv : val ) ) > maxv ? maxv : val )   

void retinex_scales_distribution(float * scales, int nscales, int mode, int s);
void compute_coefs3(gauss3_coefs * c, float sigma);
void gausssmooth(float * in, float * out, int size, int rowstride, gauss3_coefs * c);
void MSRCR(unsigned char * src, int width, int height, int bytes);
void retinex_process(Mat data1, Mat &data2);