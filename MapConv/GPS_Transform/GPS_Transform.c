#include "GPS_Transform.h"
#include <stdbool.h>
#include <math.h>

// 谷歌地图(外国)与GPS模块出来的经纬度是WGS-84坐标系的，而国内使用的是经过加密的GCJ-02坐标系(除了百度地图,百度地图有自己的二次加密,是BD09坐标系)，所以需要进行转换
// Reference Link: https://blog.csdn.net/feinifi/article/details/120547127

//采用了克拉索索夫斯基椭球系数

#define ABS(x) (((x)>0)?(x):-(x))

static const double pi = 3.14159265358979324;
//地球长半轴轴长，也就是地球半径
static const double a = 6378245.0;
//地球第一偏心率的平方
static const double ee = 0.00669342162296594323;

//计算纬度转换
static double transformLat(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(ABS(x));
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
    return ret;
}

//计算经度转换
static double transformLon(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(ABS(x));
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
    return ret;
}

void GPS_Transform(double wgLat, double wgLon, double* mgLat, double* mgLon)
{
    //105,35是中国大地原点的偏移 位置在中国陕西省咸阳市下的泾阳县。
    double dLat = transformLat(wgLon - 105.0, wgLat - 35.0);
    double dLon = transformLon(wgLon - 105.0, wgLat - 35.0);
    double radLat = wgLat / 180.0 * pi;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
    *mgLat = wgLat + dLat;
    *mgLon = wgLon + dLon;
};
