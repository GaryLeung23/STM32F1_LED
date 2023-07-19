/*
 * MIT License
 * Copyright (c) 2021 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "TileConv.h"
#include <string.h>
/**
 * @brief Construct a new Tile Conv:: Tile Conv object
 * @param viewWidth
 * @param viewHeight
 * @param tileSize
 */
TileConv::TileConv(uint32_t viewWidth, uint32_t viewHeight, uint32_t tileSize)
{
    memset(&priv, 0, sizeof(priv));
    priv.viewWidth = viewWidth;
    priv.viewHeight = viewHeight;
    priv.tileSize = tileSize;
}

/**
 * @brief 设置焦点位置pointFocus,顺便更新pointView与pointTileCont
 * @param x
 * @param y
 */
void TileConv::SetFocusPos(int32_t x, int32_t y)
{
    //focus point
    priv.pointFocus.x = x;
    priv.pointFocus.y = y;

    //一个view大小的移动窗口
    const int32_t viewHalfWidth = priv.viewWidth / 2;
    const int32_t viewHalfHeight = priv.viewHeight / 2;
    const uint32_t tileSize = priv.tileSize;

    priv.pointView[0].x = x - viewHalfWidth;
    priv.pointView[0].y = y - viewHalfHeight;
    priv.pointView[1].x = x + viewHalfWidth;
    priv.pointView[1].y = y - viewHalfHeight;
    priv.pointView[2].x = x - viewHalfWidth;
    priv.pointView[2].y = y + viewHalfHeight;
    priv.pointView[3].x = x + viewHalfWidth;
    priv.pointView[3].y = y + viewHalfHeight;

    //确保tileContainer的宽度高度比view的宽度高度大
    const int32_t tileContWidth = (priv.viewWidth / tileSize + 2) * tileSize; 
    const int32_t tileContHeight = (priv.viewHeight / tileSize + 2) * tileSize;

    //pointTileCont比pointView大一圈，最多大两个tileSize 对齐到tileSize大小
    priv.pointTileCont[0].x = FixTile(priv.pointView[0].x, false);
    priv.pointTileCont[0].y = FixTile(priv.pointView[0].y, false);
    priv.pointTileCont[1].x = priv.pointTileCont[0].x + tileContWidth;
    priv.pointTileCont[1].y = priv.pointTileCont[0].y;
    priv.pointTileCont[2].x = priv.pointTileCont[0].x;
    priv.pointTileCont[2].y = priv.pointTileCont[0].y + tileContHeight;
    priv.pointTileCont[3].x = priv.pointTileCont[0].x + tileContWidth;
    priv.pointTileCont[3].y = priv.pointTileCont[0].y + tileContHeight;
}
/**
 * @brief 获得TileContainer的info
 *        将pointTileCont转换为Rect_t
 * @param rect [out]
 * @return tile的数量
 */
uint32_t TileConv::GetTileContainer(Rect_t* rect)
{
    rect->x = priv.pointTileCont[0].x;
    rect->y = priv.pointTileCont[0].y;
    rect->width = priv.pointTileCont[1].x - priv.pointTileCont[0].x;
    rect->height = priv.pointTileCont[2].y - priv.pointTileCont[0].y;

    uint32_t size = (rect->width / priv.tileSize) * (rect->height / priv.tileSize);
    return size;
}

/**
 * @brief 获取pointView起点相对于pointTileCont起点的偏移
 * @param offset [out]
 */
void TileConv::GetTileContainerOffset(Point_t* offset)
{
    GetOffset(offset, &priv.pointView[0]);
}

/**
 * @brief 获取index对应的tile的起始坐标
 * @param index  是串行的，从左到右，从上到下，从0开始。
 * @param pos [out]
 */
void TileConv::GetTilePos(uint32_t index, Point_t* pos)
{
    int32_t width = priv.pointTileCont[1].x - priv.pointTileCont[0].x;
    int32_t widthIndexMax = width / priv.tileSize;
    pos->x = priv.pointTileCont[0].x + priv.tileSize * (index % widthIndexMax);
    pos->y = priv.pointTileCont[0].y + priv.tileSize * (index / widthIndexMax);
}

/**
 * @brief 获取pointFocus相对于pointTileCont起点的偏移
 * @param offset [out]
 */
void TileConv::GetFocusOffset(Point_t* offset)
{
    GetOffset(offset, &priv.pointFocus);
}

/**
 * @brief 让坐标x对齐到tileSize的整数倍
 * @param x
 * @param up true时向上对齐
 * @return
 */
int32_t TileConv::FixTile(int32_t x, bool up)
{
    int32_t r = x % priv.tileSize;

    if (r == 0)
    {
        return x;
    }

    int32_t ret = x - r;

    if (up)
    {
        ret += priv.tileSize;
    }

    return ret;
}

/**
 * @brief 获取point相对于pointTileCont起点的偏移
 * @param offset [out]
 * @param point
 */
void TileConv::GetOffset(Point_t* offset, const Point_t* point)
{
    offset->x = point->x - priv.pointTileCont[0].x;
    offset->y = point->y - priv.pointTileCont[0].y;
}
