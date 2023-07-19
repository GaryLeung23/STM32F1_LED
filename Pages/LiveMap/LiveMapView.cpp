#include "LiveMapView.h"
#include "Config.h"
#include <stdarg.h>
#include <stdio.h>

using namespace Page;

#if CONFIG_MAP_IMG_PNG_ENABLE
#include "Utils/lv_img_png/lv_img_png.h"
#  define TILE_IMG_CREATE  lv_img_png_create
#  define TILE_IMG_SET_SRC lv_img_png_set_src
#else
#  define TILE_IMG_CREATE  lv_img_create
#  define TILE_IMG_SET_SRC lv_img_set_src
#endif

void LiveMapView::Create(lv_obj_t* root, uint32_t tileNum)
{
    lv_obj_set_style_bg_color(root, lv_color_white(), 0);

    //root -> ui.labelInfo
    lv_obj_t* label = lv_label_create(root);
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, ResourcePool::GetFont("bahnschrift_17"), 0);
    lv_label_set_text(label, "LOADING...");
    ui.labelInfo = label;

    //Init style
    Style_Create();

    //create map obj
    Map_Create(root, tileNum);
    //create zoomctrl obj
    ZoomCtrl_Create(root);
    //create sportinfo obj
    SportInfo_Create(root);
}

void LiveMapView::Delete()
{
    if (ui.track.lineTrack)
    {
        delete ui.track.lineTrack;
        ui.track.lineTrack = nullptr;
    }

    if (ui.map.imgTiles)
    {
        lv_free(ui.map.imgTiles);
        ui.map.imgTiles = nullptr;
    }

    lv_style_reset(&ui.styleCont);
    lv_style_reset(&ui.styleLabel);
    lv_style_reset(&ui.styleLine);
}
/**
 * @brief Init style
 * 
 */
void LiveMapView::Style_Create()
{
    //set Contstyle
    lv_style_init(&ui.styleCont);
    lv_style_set_bg_color(&ui.styleCont, lv_color_black());
    lv_style_set_bg_opa(&ui.styleCont, LV_OPA_60);
    lv_style_set_radius(&ui.styleCont, 6);//设置圆角
    lv_style_set_shadow_width(&ui.styleCont, 10);
    lv_style_set_shadow_color(&ui.styleCont, lv_color_black());

    //set Label style
    lv_style_init(&ui.styleLabel);
    lv_style_set_text_font(&ui.styleLabel, ResourcePool::GetFont("bahnschrift_17"));
    lv_style_set_text_color(&ui.styleLabel, lv_color_white());

    //set Line style
    lv_style_init(&ui.styleLine);
    lv_style_set_line_color(&ui.styleLine, lv_color_hex(0xff931e));
    lv_style_set_line_width(&ui.styleLine, 5);
    lv_style_set_line_opa(&ui.styleLine, LV_OPA_COVER);
    lv_style_set_line_rounded(&ui.styleLine, true);//圆角线
}
/**
 * @brief Create map obj
 * 
 * @param par 
 * @param tileNum 
 */
void LiveMapView::Map_Create(lv_obj_t* par, uint32_t tileNum)
{
    //root -> ui.map.cont
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
#if CONFIG_LIVE_MAP_DEBUG_ENABLE
    lv_obj_set_style_outline_color(cont, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_outline_width(cont, 2, 0);
#endif
    ui.map.cont = cont;

    ui.map.imgTiles = (lv_obj_t**)lv_malloc(tileNum * sizeof(lv_obj_t*));//分配tileNum个img obj指针
    ui.map.tileNum = tileNum;

    //root -> ui.map.cont ->ui.map.imgTiles[]
    //创建tileNum个img obj
    for (uint32_t i = 0; i < tileNum; i++)
    {
        lv_obj_t* img = TILE_IMG_CREATE(cont);
        lv_obj_remove_style_all(img);
        ui.map.imgTiles[i] = img;
    }
    //create track obj
    Track_Create(cont);

    //root -> ui.map.cont ->ui.map.imgArrow
    lv_obj_t* img = lv_img_create(cont);
    lv_img_set_src(img, ResourcePool::GetImage("gps_arrow_dark"));

    lv_img_t* imgOri = (lv_img_t*)img;
    //默认是在父对象的左上角，这里 ui.map.cont还没有设置pos，所以默认在root的左上角的，所以这里设置(-imgOri->w, -imgOri->h),才能隐藏arrow
    lv_obj_set_pos(img, -imgOri->w, -imgOri->h);
    ui.map.imgArrow = img;
}
/**
 * @brief 设置ui.map.cont的size 给各个ui.map.imgTiles 设置size与pos
 * 
 * @param tileSize 
 * @param widthCnt   pointTileCont 的横向的tile个数
 */
void LiveMapView::SetMapTile(uint32_t tileSize, uint32_t widthCnt)
{
     //pointTileCont 的tileNum
    uint32_t tileNum = ui.map.tileNum;

    //pointTileCont 的w与h
    lv_coord_t width = (lv_coord_t)(tileSize * widthCnt);
    lv_coord_t height = (lv_coord_t)(tileSize * (tileNum/ widthCnt));

    //设置ui.map.cont的size(pointTileCont),这个是必定大于屏幕size(pointView)的
    lv_obj_set_size(ui.map.cont, width, height);

    //给各个ui.map.imgTiles 设置size与pos
    for (uint32_t i = 0; i < tileNum; i++)
    {
        lv_obj_t* img = ui.map.imgTiles[i];

        lv_obj_set_size(img, tileSize, tileSize);

        lv_coord_t x = (i % widthCnt) * tileSize;
        lv_coord_t y = (i / widthCnt) * tileSize;
        lv_obj_set_pos(img, x, y);
    }
}
/**
 * @brief 设置ui.map.imgTiles 的img src
 * @param index
 * @param src
 */
void LiveMapView::SetMapTileSrc(uint32_t index, const char* src)
{
    if (index >= ui.map.tileNum)
    {
        return;
    }

    TILE_IMG_SET_SRC(ui.map.imgTiles[index], src);
}
/**
 * @brief set Arrow Theme
 * @param theme dark default light
 */
void LiveMapView::SetArrowTheme(const char* theme)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "gps_arrow_%s", theme);

    const void* src = ResourcePool::GetImage(buf);

    if (src == nullptr)
    {
        ResourcePool::GetImage("gps_arrow_default");
    }

    lv_img_set_src(ui.map.imgArrow, src);
}
/**
 * @brief 描绘LineActive (临时Line) 一条从poly_line的最后一条(current_index - 1 )single_line的最后一个point 到 (x,y)的线
 * @param x
 * @param y
 */
void LiveMapView::SetLineActivePoint(lv_coord_t x, lv_coord_t y)
{
    lv_point_t end_point;
    //获取poly_line的最后一条(current_index - 1 )single_line的最后一个point的地址
    if (!ui.track.lineTrack->get_end_point(&end_point))
    {
        return;
    }
    //根据end_point与(x,y)绘制lineActive
    ui.track.pointActive[0] = end_point;
    ui.track.pointActive[1].x = x;
    ui.track.pointActive[1].y = y;
    lv_line_set_points(ui.track.lineActive, ui.track.pointActive, 2);
}

void LiveMapView::ZoomCtrl_Create(lv_obj_t* par)
{
    //root -> ui.zoom.cont
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_add_style(cont, &ui.styleCont, 0);
    lv_obj_set_style_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_size(cont, 50, 30);
    //LV_STATE_DEFAULT状态下是 设置在父对象的右上角。不是隐藏的
    lv_obj_set_pos(cont, lv_obj_get_style_width(par, 0) - lv_obj_get_style_width(cont, 0) + 5, 40);
    ui.zoom.cont = cont;

    //设置cont的style的过渡动画
    static const lv_style_prop_t prop[] =
    {
        LV_STYLE_X,
        LV_STYLE_OPA,
        LV_STYLE_PROP_INV
    };
    static lv_style_transition_dsc_t tran;
    lv_style_transition_dsc_init(&tran, prop, lv_anim_path_ease_out, 200, 0, nullptr);
    //LV_STATE_USER_1 状态下是在父对象的右外上角，并且是透明的。即是隐藏起来的
    lv_obj_set_style_x(cont, lv_obj_get_style_width(par, 0), LV_STATE_USER_1);
    lv_obj_set_style_opa(cont, LV_OPA_TRANSP, LV_STATE_USER_1);
    lv_obj_set_style_transition(cont, &tran, LV_STATE_USER_1);
    lv_obj_set_style_transition(cont, &tran, LV_STATE_DEFAULT);
    lv_obj_add_state(cont, LV_STATE_USER_1);//默认是 LV_STATE_USER_1 状态

    //root -> ui.zoom.cont -> ui.zoom.labelInfo
    lv_obj_t* label = lv_label_create(cont);
    lv_obj_add_style(label, &ui.styleLabel, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, -2, 0);
    lv_label_set_text(label, "--");
    ui.zoom.labelInfo = label;

    //root -> ui.zoom.cont -> ui.zoom.slider
    lv_obj_t* slider = lv_slider_create(cont);
    lv_obj_remove_style_all(slider);
//    lv_slider_set_value(slider, 15, LV_ANIM_OFF);
    ui.zoom.slider = slider;
}

void LiveMapView::SportInfo_Create(lv_obj_t* par)
{
    //root -> ui.sportInfo.cont
    lv_obj_t* obj = lv_obj_create(par);
    lv_obj_remove_style_all(obj);
    lv_obj_add_style(obj, &ui.styleCont, 0);
    lv_obj_set_size(obj, 159, 66);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_LEFT, -10, 10);
    lv_obj_set_style_radius(obj, 10, 0);//圆角
    ui.sportInfo.cont = obj;

    //root -> ui.sportInfo.cont -> ui.sportInfo.labelSpeed
    lv_obj_t* label = lv_label_create(obj);
    lv_label_set_text(label, "00");
    lv_obj_set_style_text_font(label, ResourcePool::GetFont("bahnschrift_32"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 20, -10);
    ui.sportInfo.labelSpeed = label;

    // root -> ui.sportInfo.cont ->label
    label = lv_label_create(obj);
    lv_label_set_text(label, "km/h");
    lv_obj_set_style_text_font(label, ResourcePool::GetFont("bahnschrift_13"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align_to(label, ui.sportInfo.labelSpeed, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);

    //root -> ui.sportInfo.cont -> ui.sportInfo.labelTrip
    ui.sportInfo.labelTrip = ImgLabel_Create(obj, ResourcePool::GetImage("trip"), 5, 10);
    //root -> ui.sportInfo.cont -> ui.sportInfo.labelTime
    ui.sportInfo.labelTime = ImgLabel_Create(obj, ResourcePool::GetImage("alarm"), 5, 30);
}

lv_obj_t* LiveMapView::ImgLabel_Create(lv_obj_t* par, const void* img_src, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    lv_obj_t* img = lv_img_create(par);
    lv_img_set_src(img, img_src);

    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, y_ofs);

    lv_obj_t* label = lv_label_create(par);
    lv_label_set_text(label, "--");
    lv_obj_add_style(label, &ui.styleLabel, 0);
    lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, x_ofs, 0);
    return label;
}
/**
 * @brief Create track obj
 * 
 * @param par 
 */
void LiveMapView::Track_Create(lv_obj_t* par)
{
    //root -> ui.map.cont -> ui.track.cont
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));//这个对象的大小将被设置为 100% 的父对象大小
    ui.track.cont = cont;

    //root -> ui.map.cont -> ui.track.cont -> ui.track.lineTrack
    ui.track.lineTrack = new lv_poly_line(cont);

    ui.track.lineTrack->set_style(&ui.styleLine);

    //root -> ui.map.cont -> ui.track.cont -> ui.track.lineActive
    lv_obj_t* line = lv_line_create(cont);
    lv_obj_remove_style_all(line);
    lv_obj_add_style(line, &ui.styleLine, 0);
#if CONFIG_LIVE_MAP_DEBUG_ENABLE
    lv_obj_set_style_line_color(line, lv_palette_main(LV_PALETTE_BLUE), 0);
#endif
    ui.track.lineActive = line;
}
