#include "DialplateView.h"
#include <stdarg.h>
#include <stdio.h>

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

using namespace Page;

void DialplateView::Create(lv_obj_t* root)
{
    BottomInfo_Create(root);
    TopInfo_Create(root);
    BtnCont_Create(root);

    //create animation timeline
    ui.anim_timeline = lv_anim_timeline_create();

#define ANIM_DEF(start_time, obj, attr, start, end) \
    {start_time, obj, LV_ANIM_EXEC(attr), start, end, 500, lv_anim_path_ease_out, true}

#define ANIM_OPA_DEF(start_time, obj) \
    ANIM_DEF(start_time, obj, opa_scale, LV_OPA_TRANSP, LV_OPA_COVER)

    lv_coord_t y_tar_top = lv_obj_get_y(ui.topInfo.cont);
    lv_coord_t y_tar_bottom = lv_obj_get_y(ui.bottomInfo.cont);
    lv_coord_t h_tar_btn = lv_obj_get_height(ui.btnCont.btnRec);

    lv_anim_timeline_wrapper_t wrapper[] =
    {
        ANIM_DEF(0, ui.topInfo.cont, y, -lv_obj_get_height(ui.topInfo.cont), y_tar_top),

        ANIM_DEF(200, ui.bottomInfo.cont, y, -lv_obj_get_height(ui.bottomInfo.cont), y_tar_bottom),
        ANIM_OPA_DEF(200, ui.bottomInfo.cont),

        ANIM_DEF(500, ui.btnCont.btnMap, height, 0, h_tar_btn),
        ANIM_DEF(600, ui.btnCont.btnRec, height, 0, h_tar_btn),
        ANIM_DEF(700, ui.btnCont.btnMenu, height, 0, h_tar_btn),
        LV_ANIM_TIMELINE_WRAPPER_END
    };
    lv_anim_timeline_add_wrapper(ui.anim_timeline, wrapper);//
}

void DialplateView::Delete()
{
    if(ui.anim_timeline)
    {
        lv_anim_timeline_del(ui.anim_timeline);
        ui.anim_timeline = nullptr;
    }
}

//create speed dialplate
void DialplateView::TopInfo_Create(lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_HOR_RES, 142);

    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x333333), 0);//设置bg color

    lv_obj_set_style_radius(cont, 27, 0);//设置圆角
    lv_obj_set_y(cont, -36);//往上移动36个像素 隐藏掉上面的两个圆角
    ui.topInfo.cont = cont;

    lv_obj_t* label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, ResourcePool::GetFont("bahnschrift_65"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, "00");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 63);
    ui.topInfo.labelSpeed = label;

    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, ResourcePool::GetFont("bahnschrift_17"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, "km/h");
    lv_obj_align_to(label, ui.topInfo.labelSpeed, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);//相对于labelSpeed的下边缘居中对齐
    ui.topInfo.labelUint = label;
}

//create SportStatus info view
void DialplateView::BottomInfo_Create(lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_size(cont, LV_HOR_RES, 130);//90
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 106);

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);//子元素将按行排列，并在行末自动换行。

    lv_obj_set_flex_align(
        cont,
        LV_FLEX_ALIGN_SPACE_EVENLY,//main_place 均匀分布子元素，并在子元素之间添加相等的间距，但是不在第一个子元素前和最后一个子元素后添加间距。
        LV_FLEX_ALIGN_CENTER,//cross_place 居中对齐
        LV_FLEX_ALIGN_CENTER //track_place 居中对齐
    );

    ui.bottomInfo.cont = cont;

    const char* unitText[4] =
    {
        "AVG",
        "Time",
        "Trip",
        "Calorie"
    };

    //create labelInfoGrp
    for (int i = 0; i < ARRAY_SIZE(ui.bottomInfo.labelInfoGrp); i++)
    {
        SubInfoGrp_Create(
            cont,
            &(ui.bottomInfo.labelInfoGrp[i]),
            unitText[i]
        );
    }
}

//create subinfo
void DialplateView::SubInfoGrp_Create(lv_obj_t* par, SubInfo_t* info, const char* unitText)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 93, 54);//93,39

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);//子元素将按列排列
    lv_obj_set_flex_align(
        cont,
        LV_FLEX_ALIGN_SPACE_AROUND,//main_place 均匀分布子元素，并在子元素之间添加相等的间距，同时在第一个子元素前和最后一个子元素后添加一半的间距。
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    //value
    lv_obj_t* label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, ResourcePool::GetFont("bahnschrift_17"), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    info->lableValue = label;

    //label
    label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, ResourcePool::GetFont("bahnschrift_13"), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xb3b3b3), 0);
    lv_label_set_text(label, unitText);
    info->lableUnit = label;

    info->cont = cont;
}

//create btns
void DialplateView::BtnCont_Create(lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_HOR_RES, 40);
    lv_obj_align_to(cont, ui.bottomInfo.cont, LV_ALIGN_OUT_BOTTOM_MID, 0,30);//外底部中心点对齐

    /*lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_place(
        cont,
        LV_FLEX_PLACE_SPACE_AROUND,
        LV_FLEX_PLACE_CENTER,
        LV_FLEX_PLACE_CENTER
    );*/

    ui.btnCont.cont = cont;

    ui.btnCont.btnMap = Btn_Create(cont, ResourcePool::GetImage("locate"), -80);
    ui.btnCont.btnRec = Btn_Create(cont, ResourcePool::GetImage("start"), 0);
    ui.btnCont.btnMenu = Btn_Create(cont, ResourcePool::GetImage("menu"), 80);
}

lv_obj_t* DialplateView::Btn_Create(lv_obj_t* par, const void* img_src, lv_coord_t x_ofs)
{
    lv_obj_t* obj = lv_obj_create(par);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 40, 31);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_align(obj, LV_ALIGN_CENTER, x_ofs, 0);//设置x_ofs
    lv_obj_set_style_bg_img_src(obj, img_src, 0);//设置img_src

    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_width(obj, 45, LV_STATE_PRESSED);
    lv_obj_set_style_height(obj, 25, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x666666), 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xbbbbbb), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff931e), LV_STATE_FOCUSED);
    lv_obj_set_style_radius(obj, 9, 0);//设置圆角


    //设置过渡动画
    static lv_style_transition_dsc_t tran;
    static const lv_style_prop_t prop[] = { LV_STYLE_WIDTH, LV_STYLE_HEIGHT, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(
        &tran,
        prop,
        lv_anim_path_ease_out,
        200,
        0,
        nullptr
    );
    lv_obj_set_style_transition(obj, &tran, LV_STATE_PRESSED);
    lv_obj_set_style_transition(obj, &tran, LV_STATE_FOCUSED);

    lv_obj_update_layout(obj);//更新其布局

    return obj;
}

//timeline anim start
void DialplateView::AppearAnimStart(bool reverse)
{
    lv_anim_timeline_set_reverse(ui.anim_timeline, reverse);
    lv_anim_timeline_start(ui.anim_timeline);
}
