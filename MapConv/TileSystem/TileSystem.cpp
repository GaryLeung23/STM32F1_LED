#include "TileSystem.h"

#include <algorithm>
#include <math.h>
#include <string.h>

//Bing Maps Tile System

using namespace Microsoft_MapPoint;

static const double EarthRadius = 6378137;
static const double MinLatitude = -85.05112878;
static const double MaxLatitude = 85.05112878;
static const double MinLongitude = -180;
static const double MaxLongitude = 180;
static const double MATH_PI = 3.1415926535897932384626433832795;

/// <summary>
/// Clips a number to the specified minimum and maximum values. 限制在一个范围内
/// </summary>
/// <param name="n">The number to clip.</param>
/// <param name="minValue">Minimum allowable value.</param>
/// <param name="maxValue">Maximum allowable value.</param>
/// <returns>The clipped value.</returns>
static double Clip(double n, double minValue, double maxValue)
{
    return std::min(std::max(n, minValue), maxValue);
}

/// <summary>
/// Determines the map width and height (in pixels) at a specified level
/// of detail. 详细程度1~23,level约小，地图越小，最小是512x512
/// </summary>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <returns>The map width and height in pixels.</returns>
uint32_t TileSystem::MapSize(int levelOfDetail)
{
    return (uint32_t)256 << levelOfDetail;
}

/// <summary>
/// Determines the ground resolution (in meters per pixel) at a specified
/// latitude and level of detail. 地面分辨率表示地图中单个像素表示的地面距离，单位 meter/pixel
/// </summary>
/// <param name="latitude">Latitude (in degrees) at which to measure the
/// ground resolution.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <returns>The ground resolution, in meters per pixel.</returns>
double TileSystem::GroundResolution(double latitude, int levelOfDetail)
{
    latitude = Clip(latitude, MinLatitude, MaxLatitude);
    return cos(latitude * MATH_PI / 180) * 2 * MATH_PI * EarthRadius / MapSize(levelOfDetail);
}

/// <summary>
/// Determines the map scale at a specified latitude, level of detail,
/// and screen resolution.  地图比例尺表示地图距离与实际距离的比例，其中 1inch = 0.0254mter，dpi是每英寸点数(Dots/Inch)
/// 先转化为单个像素表示的地面距离(单位 inch/pixel),在计算地图比例尺，表示比率1:N的分母N。
/// </summary>
/// <param name="latitude">Latitude (in degrees) at which to measure the
/// map scale.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <param name="screenDpi">Resolution of the screen, in dots per inch.</param>
/// <returns>The map scale, expressed as the denominator N of the ratio 1 : N.</returns>
double TileSystem::MapScale(double latitude, int levelOfDetail, int screenDpi)
{
    return GroundResolution(latitude, levelOfDetail) * screenDpi / 0.0254;
}


// 地图左上角的像素始终具有像素坐标 (0, 0)。地图右下角的像素坐标为(width-1,height-1)，
// 或者参考上一节的等式，(256 * 2 level –1, 256 * 2 level –1)

//纬度和经度假定在 WGS 84 基准上。尽管 Bing 地图使用球面投影，但将所有地理坐标转换为通用基准很重要，因此选择 WGS 84 作为该基准。
// 假设经度的范围为 -180 到 +180 度，纬度必须被裁剪到 -85.05112878 到 85.05112878 的范围内。
// 这避免了极点处的奇点，并导致投影地图为正方形。

/// <summary>
/// Converts a point from latitude/longitude WGS-84 coordinates (in degrees)
/// into pixel XY coordinates at a specified level of detail.
/// </summary>
/// <param name="latitude">Latitude of the point, in degrees.</param>
/// <param name="longitude">Longitude of the point, in degrees.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <param name="pixelX">Output parameter receiving the X coordinate in pixels.</param>
/// <param name="pixelY">Output parameter receiving the Y coordinate in pixels.</param>
void TileSystem::LatLongToPixelXY(double latitude, double longitude, int levelOfDetail, int *pixelX, int *pixelY)
{
    latitude = Clip(latitude, MinLatitude, MaxLatitude);
    longitude = Clip(longitude, MinLongitude, MaxLongitude);

    double x = (longitude + 180) / 360;
    double sinLatitude = sin(latitude * MATH_PI / 180);
    double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * MATH_PI);

    uint32_t mapSize = MapSize(levelOfDetail);
    *pixelX = (int)Clip(x * mapSize + 0.5, 0, mapSize - 1);
    *pixelY = (int)Clip(y * mapSize + 0.5, 0, mapSize - 1);
}

/// <summary>
/// Converts a pixel from pixel XY coordinates at a specified level of detail
/// into latitude/longitude WGS-84 coordinates (in degrees).
/// </summary>
/// <param name="pixelX">X coordinate of the point, in pixels.</param>
/// <param name="pixelY">Y coordinates of the point, in pixels.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <param name="latitude">Output parameter receiving the latitude in degrees.</param>
/// <param name="longitude">Output parameter receiving the longitude in degrees.</param>
void TileSystem::PixelXYToLatLong(int pixelX, int pixelY, int levelOfDetail, double *latitude, double *longitude)
{
    double mapSize = MapSize(levelOfDetail);
    double x = (Clip(pixelX, 0, mapSize - 1) / mapSize) - 0.5;
    double y = 0.5 - (Clip(pixelY, 0, mapSize - 1) / mapSize);

    *latitude = 90 - 360 * atan(exp(-y * 2 * MATH_PI)) / MATH_PI;
    *longitude = 360 * x;
}

/// <summary>
/// Converts pixel XY coordinates into tile XY coordinates of the tile containing
/// the specified pixel.
/// </summary>
/// <param name="pixelX">Pixel X coordinate.</param>
/// <param name="pixelY">Pixel Y coordinate.</param>
/// <param name="tileX">Output parameter receiving the tile X coordinate.</param>
/// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>
void TileSystem::PixelXYToTileXY(int pixelX, int pixelY, int *tileX, int *tileY)
{
    *tileX = pixelX / 256;
    *tileY = pixelY / 256;
}

/// <summary>
/// Converts tile XY coordinates into pixel XY coordinates of the upper-left pixel
/// of the specified tile.
/// </summary>
/// <param name="tileX">Tile X coordinate.</param>
/// <param name="tileY">Tile Y coordinate.</param>
/// <param name="pixelX">Output parameter receiving the pixel X coordinate.</param>
/// <param name="pixelY">Output parameter receiving the pixel Y coordinate.</param>
void TileSystem::TileXYToPixelXY(int tileX, int tileY, int *pixelX, int *pixelY)
{
    *pixelX = tileX * 256;
    *pixelY = tileY * 256;
}

// 为了优化瓦片的索引和存储，将二维瓦片 XY 坐标组合成一维字符串，称为四叉树键，简称“quadkeys”。
// 要将瓦片坐标转换为四元键，Y 和 X 坐标的位将交错，结果将被解释为 base-4 数字（保留前导零）并转换为字符串。
//  First, the length of a quadkey (the number of digits) equals the level of detail of the corresponding tile. 
//  Second, the quadkey of any tile starts with the quadkey of its parent tile (the containing tile at the previous level).

/// <summary>
/// Converts tile XY coordinates into a QuadKey at a specified level of detail.
/// </summary>
/// <param name="tileX">Tile X coordinate.</param>
/// <param name="tileY">Tile Y coordinate.</param>
/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
/// to 23 (highest detail).</param>
/// <returns>A string containing the QuadKey.</returns>
void TileSystem::TileXYToQuadKey(int tileX, int tileY, int levelOfDetail, char *quadKeyBuffer, uint32_t len)
{
    uint32_t quadKeyIndex = 0;
    for (int i = levelOfDetail; i > 0; i--)
    {
        char digit = '0';
        int mask = 1 << (i - 1);
        if ((tileX & mask) != 0)
        {
            digit++;
        }
        if ((tileY & mask) != 0)
        {
            digit++;
            digit++;
        }
        quadKeyBuffer[quadKeyIndex] = digit;
        quadKeyIndex++;

        if (quadKeyIndex >= len - 1)
        {
            break;
        }
    }

    quadKeyBuffer[quadKeyIndex] = '\0';
}

/// <summary>
/// Converts a QuadKey into tile XY coordinates.
/// </summary>
/// <param name="quadKey">QuadKey of the tile.</param>
/// <param name="tileX">Output parameter receiving the tile X coordinate.</param>
/// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>
/// <param name="levelOfDetail">Output parameter receiving the level of detail.</param>
void TileSystem::QuadKeyToTileXY(const char *quadKey, int *tileX, int *tileY, int *levelOfDetail)
{
    *tileX = *tileY = 0;
    int len = (int)strlen(quadKey);
    *levelOfDetail = len;
    for (int i = len; i > 0; i--)
    {
        int mask = 1 << (i - 1);
        switch (quadKey[len - i])
        {
        case '0':
            break;

        case '1':
            *tileX |= mask;
            break;

        case '2':
            *tileY |= mask;
            break;

        case '3':
            *tileX |= mask;
            *tileY |= mask;
            break;

        default:
            // throw new ArgumentException("Invalid QuadKey digit sequence.");
            break;
        }
    }
}
