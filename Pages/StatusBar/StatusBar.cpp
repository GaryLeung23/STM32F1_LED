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
#include "StatusBar.h"
#include "Page.h"


#define BATT_USAGE_HEIGHT (lv_obj_get_style_height(ui.battery.img, 0) - 4)
#define BATT_USAGE_WIDTH  (lv_obj_get_style_width(ui.battery.img, 0) - 4)

#define STATUS_BAR_HEIGHT 25

static Account* actStatusBar;

static void StatusBar_AnimCreate(lv_obj_t* contBatt);

struct
{
    lv_obj_t* cont;

    struct
    {
        lv_obj_t* img;
        lv_obj_t* label;
    } satellite;

    lv_obj_t* imgSD;

    lv_obj_t* labelClock;

    lv_obj_t* labelRec;

    struct
    {
        lv_obj_t* img;
        lv_obj_t* objUsage;
        lv_obj_t* label;
    } battery;
} ui;

//设置电池透明度
static void StatusBar_ConBattSetOpa(lv_obj_t* obj, int32_t opa)
{
    lv_obj_set_style_opa(obj, opa, 0);
}

//StatusBar_onAnimOpaFinish与StatusBar_onAnimHeightFinish与StatusBar_AnimCreate配合，构成一循环充电动画

static void StatusBar_onAnimOpaFinish(lv_anim_t* a)
{
    lv_obj_t* obj = (lv_obj_t*)a->var;//a->var is contBatt
    StatusBar_ConBattSetOpa(obj, LV_OPA_COVER);
    StatusBar_AnimCreate(obj);
}

static void StatusBar_onAnimHeightFinish(lv_anim_t* a)
{
    lv_anim_t a_opa;
    lv_anim_init(&a_opa);
    lv_anim_set_var(&a_opa, a->var);//a->var is contBatt,即ui.battery.objUsage
    lv_anim_set_exec_cb(&a_opa, (lv_anim_exec_xcb_t)StatusBar_ConBattSetOpa);
    lv_anim_set_ready_cb(&a_opa, StatusBar_onAnimOpaFinish);//
    lv_anim_set_values(&a_opa, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_early_apply(&a_opa, true);
    lv_anim_set_delay(&a_opa, 500);
    lv_anim_set_time(&a_opa, 500);
    lv_anim_start(&a_opa);
}

static void StatusBar_AnimCreate(lv_obj_t* contBatt)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, contBatt);//set contBatt to var
    lv_anim_set_exec_cb(&a, [](void* var, int32_t v)
    {
        lv_obj_set_height((lv_obj_t*)var, v);//set contBatt height。因为以LV_ALIGN_BOTTOM_MID与img对齐，所以动画是从下到上变化
    });
    lv_anim_set_values(&a, 0, BATT_USAGE_HEIGHT);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_ready_cb(&a, StatusBar_onAnimHeightFinish);//
    lv_anim_start(&a);
}



// recorder animation label
static lv_obj_t* StatusBar_RecAnimLabelCreate(lv_obj_t* par)
{
    //设置字体
    static lv_style_t style_label;
    lv_style_init(&style_label);
    lv_style_set_text_color(&style_label, lv_color_white());
    lv_style_set_text_font(&style_label, ResourcePool::GetFont("bahnschrift_13"));

    //创建anim label    
    lv_obj_t* alabel = lv_anim_label_create(par);
    lv_obj_set_size(alabel, 50, STATUS_BAR_HEIGHT - 4);// weight 50
    lv_anim_label_set_enter_dir(alabel, LV_DIR_TOP);
    lv_anim_label_set_exit_dir(alabel, LV_DIR_BOTTOM);
    lv_anim_label_set_path(alabel, lv_anim_path_ease_out);
    lv_anim_label_set_time(alabel, 500);
    lv_anim_label_add_style(alabel, &style_label);

    lv_obj_align(alabel, LV_ALIGN_RIGHT_MID, -45, 0);//与父对象对齐
    //lv_obj_set_style_border_color(alabel, lv_color_white(), 0);
    //lv_obj_set_style_border_width(alabel, 1, 0);

    //设置anim label enter exit动画
    lv_anim_t a_enter;
    lv_anim_init(&a_enter);
    lv_anim_set_early_apply(&a_enter, true);
    lv_anim_set_values(&a_enter, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_exec_cb(&a_enter, [](void* var, int32_t v)
    {
        lv_obj_set_style_opa((lv_obj_t*)var, v, 0);//设置透明度
    });
    lv_anim_set_time(&a_enter, 300);

    lv_anim_t a_exit = a_enter;
    lv_anim_set_values(&a_exit, LV_OPA_COVER, LV_OPA_TRANSP);

    lv_anim_label_set_custom_enter_anim(alabel, &a_enter);
    lv_anim_label_set_custom_exit_anim(alabel, &a_exit);

    return alabel;
}


//定时更新callback
static void StatusBar_Update(lv_timer_t* timer)
{
    /* satellite */
    HAL::GPS_Info_t gps;
    if(actStatusBar->Pull("GPS", &gps, sizeof(gps)) == Account::RES_OK)
    {
        lv_label_set_text_fmt(ui.satellite.label, "%d", gps.satellites);
    }

    DataProc::Storage_Basic_Info_t sdInfo;
    if(actStatusBar->Pull("Storage", &sdInfo, sizeof(sdInfo)) == Account::RES_OK)
    {
        sdInfo.isDetect ? lv_obj_clear_state(ui.imgSD, LV_STATE_DISABLED) : lv_obj_add_state(ui.imgSD, LV_STATE_DISABLED);//sdcard状态
    }

    /* clock */
    HAL::Clock_Info_t clock;
    if(actStatusBar->Pull("Clock", &clock, sizeof(clock)) == Account::RES_OK)
    {
        lv_label_set_text_fmt(ui.labelClock, "%02d:%02d", clock.hour, clock.minute);
    }

    /* battery */
    HAL::Power_Info_t power;
    if(actStatusBar->Pull("Power", &power, sizeof(power)) == Account::RES_OK)
    {
        lv_label_set_text_fmt(ui.battery.label, "%d", power.usage);
    }

    bool Is_BattCharging = power.isCharging;//充电状态
    lv_obj_t* contBatt = ui.battery.objUsage;
    static bool Is_BattChargingAnimActive = false;//状态变化标志
    if(Is_BattCharging)
    {
        if(!Is_BattChargingAnimActive)
        {
            StatusBar_AnimCreate(contBatt);//充电动画
            Is_BattChargingAnimActive = true;
        }
    }
    else
    {
        if(Is_BattChargingAnimActive)
        {
            lv_anim_del(contBatt, nullptr);
            StatusBar_ConBattSetOpa(contBatt, LV_OPA_COVER);
            Is_BattChargingAnimActive = false;
        }
        lv_coord_t height = lv_map(power.usage, 0, 100, 0, BATT_USAGE_HEIGHT);
        lv_obj_set_height(contBatt, height);
    }
}


//statusBar cont style init,according to user's setting
static void StatusBar_StyleInit(lv_obj_t* cont)
{
    /* style1 */
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, LV_STATE_DEFAULT);//全透明
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x333333), LV_STATE_DEFAULT);

    /* style2 */
    lv_obj_set_style_bg_opa(cont, LV_OPA_60, LV_STATE_USER_1);
    lv_obj_set_style_bg_color(cont, lv_color_black(), LV_STATE_USER_1);
    lv_obj_set_style_shadow_color(cont, lv_color_black(), LV_STATE_USER_1);
    lv_obj_set_style_shadow_width(cont, 10, LV_STATE_USER_1);

    //设置过渡效果 LV_STATE_DEFAULT->LV_STATE_USER_1
    static lv_style_transition_dsc_t tran;
    static const lv_style_prop_t prop[] =
    {
        LV_STYLE_BG_COLOR,
        LV_STYLE_OPA,
        LV_STYLE_PROP_INV
    };
    lv_style_transition_dsc_init(
        &tran,
        prop,
        lv_anim_path_ease_out,
        200,
        0,
        nullptr
    );
    lv_obj_set_style_transition(cont, &tran, LV_STATE_USER_1);
}


//sd card image
static lv_obj_t* StatusBar_SdCardImage_Create(lv_obj_t* par)
{
    lv_obj_t* img = lv_img_create(par);
    lv_img_set_src(img, ResourcePool::GetImage("sd_card"));
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 55, -1);

    //设置img的垂直偏移量  以便在sdcard LV_STATE_DISABLED时隐藏 img。
    lv_obj_set_style_translate_y(img, -STATUS_BAR_HEIGHT, LV_STATE_DISABLED);

    //设置过渡效果   两种状态的过渡效果
    static lv_style_transition_dsc_t tran;
    static const lv_style_prop_t prop[] =
    {
        LV_STYLE_TRANSLATE_Y,
        LV_STYLE_PROP_INV
    };
    lv_style_transition_dsc_init(
        &tran,
        prop,
        lv_anim_path_overshoot,
        100,
        0,
        nullptr
    );
    lv_obj_set_style_transition(img, &tran, LV_STATE_DISABLED);//设置sdcard禁用状态下的过渡效果
    lv_obj_set_style_transition(img, &tran, LV_STATE_DEFAULT);//设置sdcard默认状态下的过渡效果

    return img;
}

//set cont style's type
static void StatusBar_SetStyle(DataProc::StatusBar_Style_t style)
{
    lv_obj_t* cont = ui.cont;
    switch(style)
    {
    case DataProc::STATUS_BAR_STYLE_TRANSP:
        lv_obj_add_state(cont, LV_STATE_DEFAULT);
        lv_obj_clear_state(cont, LV_STATE_USER_1);
        break;
    case DataProc::STATUS_BAR_STYLE_BLACK:
        lv_obj_add_state(cont, LV_STATE_USER_1);//设置LV_STATE_USER_1
        break;
    default:
        break;
    }
}

//create status bar
lv_obj_t* Page::StatusBar_Create(lv_obj_t* par)
{
    lv_obj_t* cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);

    lv_obj_set_size(cont, LV_HOR_RES, STATUS_BAR_HEIGHT);
    lv_obj_set_y(cont, -STATUS_BAR_HEIGHT);//开始时隐藏状态栏
    StatusBar_StyleInit(cont);//statusbar cont style init
    ui.cont = cont;

    //font style
    static lv_style_t style_label;
    lv_style_init(&style_label);
    lv_style_set_text_color(&style_label, lv_color_white());
    lv_style_set_text_font(&style_label, ResourcePool::GetFont("bahnschrift_17"));


    //sub objects

    /* satellite */
    lv_obj_t* img = lv_img_create(cont);
    lv_img_set_src(img, ResourcePool::GetImage("satellite"));
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 14, 0);//表示对象的左侧与其父容器对象的左侧对齐，垂直居中对齐，在进行偏移
    ui.satellite.img = img;

    lv_obj_t* label = lv_label_create(cont);
    lv_obj_add_style(label, &style_label, 0);
    lv_obj_align_to(label, ui.satellite.img, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_label_set_text(label, "0");
    ui.satellite.label = label;

    /* sd card */
    ui.imgSD = StatusBar_SdCardImage_Create(cont);

    /* clock */
    label = lv_label_create(cont);
    lv_obj_add_style(label, &style_label, 0);
    lv_label_set_text(label, "00:00");
    lv_obj_center(label);
    ui.labelClock = label;

    /* recorder */ 
    //还没有设置text
    ui.labelRec = StatusBar_RecAnimLabelCreate(cont);

    /* battery */
    // cont -> img
    img = lv_img_create(cont);
    lv_img_set_src(img, ResourcePool::GetImage("battery"));
    lv_obj_align(img, LV_ALIGN_RIGHT_MID, -35, 0);
    lv_img_t* img_ext = (lv_img_t*)img;
    lv_obj_set_size(img, img_ext->w, img_ext->h);//根据图片的大小来设置的
    ui.battery.img = img;

    //img ->  objUsage
    lv_obj_t* obj = lv_obj_create(img);
    lv_obj_remove_style_all(obj);
    lv_obj_set_style_bg_color(obj, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);//设置对象背景area的不透明度。它只影响对象的背景色,不影响对象的子对象。
    lv_obj_set_style_opa(obj, LV_OPA_COVER, 0);//设置对象整体(包括背景和子对象)的不透明度。
    lv_obj_set_size(obj, BATT_USAGE_WIDTH, BATT_USAGE_HEIGHT);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, -2);//以LV_ALIGN_BOTTOM_MID与img对齐
    ui.battery.objUsage = obj;

    // cont -> label
    label = lv_label_create(cont);
    lv_obj_add_style(label, &style_label, 0);
    lv_obj_align_to(label, ui.battery.img, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_label_set_text(label, "100%");
    ui.battery.label = label;

    StatusBar_SetStyle(DataProc::STATUS_BAR_STYLE_TRANSP);//set style,StatusBar_StyleInit 初始化了状态栏的样式

    lv_timer_t* timer = lv_timer_create(StatusBar_Update, 1000, nullptr);//创建定时器
    lv_timer_ready(timer);

    return ui.cont;
}

static void StatusBar_Appear(bool en)
{
    int32_t start = -STATUS_BAR_HEIGHT;
    int32_t end = 0;

    if (!en)
    {
        int32_t temp = start;
        start = end;
        end = temp;
    }

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ui.cont);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_time(&a, 500);
    lv_anim_set_delay(&a, 1000);
    lv_anim_set_exec_cb(&a, LV_ANIM_EXEC(y));
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_early_apply(&a, true);//它设置动画是否应立即应用或在第一次 tick 后应用。
    lv_anim_start(&a);
}

static int onEvent(Account* account, Account::EventParam_t* param)
{
    if (param->event != Account::EVENT_NOTIFY)
    {
        return Account::RES_UNSUPPORTED_REQUEST;
    }

    if (param->size != sizeof(DataProc::StatusBar_Info_t))
    {
        return Account::RES_SIZE_MISMATCH;
    }

    DataProc::StatusBar_Info_t* info = (DataProc::StatusBar_Info_t*)param->data_p;

    switch(info->cmd)
    {
    case DataProc::STATUS_BAR_CMD_APPEAR:
        StatusBar_Appear(info->param.appear);
        break;
    case DataProc::STATUS_BAR_CMD_SET_STYLE:
        StatusBar_SetStyle(info->param.style);
        break;
    case DataProc::STATUS_BAR_CMD_SET_LABEL_REC:
        lv_anim_label_push_text(ui.labelRec, info->param.labelRec.show ? info->param.labelRec.str : " ");//更新一个动画标签对象的文本内容
        break;
    default:
        return Account::RES_PARAM_ERROR;
    }

    return Account::RES_OK;
}


//statusBar有点特别，他既是page也是dataProc，所以需要在这里初始化
DATA_PROC_INIT_DEF(StatusBar)
{
    account->Subscribe("GPS");
    account->Subscribe("Power");
    account->Subscribe("Clock");
    account->Subscribe("Storage");
    account->SetEventCallback(onEvent);

    actStatusBar = account;
}
