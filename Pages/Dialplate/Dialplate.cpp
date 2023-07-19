#include "Dialplate.h"

using namespace Page;

Dialplate::Dialplate()
    : recState(RECORD_STATE_READY)
    , lastFocus(nullptr)
{
}

Dialplate::~Dialplate()
{
}

void Dialplate::onCustomAttrConfig()
{

    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Dialplate::onViewLoad()
{
    Model.Init();
    View.Create(_root);

    //set event callback
    AttachEvent(View.ui.btnCont.btnMap);
    AttachEvent(View.ui.btnCont.btnRec);
    AttachEvent(View.ui.btnCont.btnMenu);
}

void Dialplate::onViewDidLoad()
{

}

void Dialplate::onViewWillAppear()
{
    lv_indev_wait_release(lv_indev_get_act());//用于等待输入设备的释放。它的作用是阻塞程序，直到当前输入设备被释放为止。
    lv_group_t* group = lv_group_get_default();
    LV_ASSERT_NULL(group);

    lv_group_set_wrap(group, false);//不循环

    //add object to group
    lv_group_add_obj(group, View.ui.btnCont.btnMap);
    lv_group_add_obj(group, View.ui.btnCont.btnRec);
    lv_group_add_obj(group, View.ui.btnCont.btnMenu);

    //初始focus
    if (lastFocus)
    {
        lv_group_focus_obj(lastFocus);
    }
    else
    {
        lv_group_focus_obj(View.ui.btnCont.btnRec);
    }

    Model.SetStatusBarStyle(DataProc::STATUS_BAR_STYLE_TRANSP);//set status bar style

    Update();//update data

    View.AppearAnimStart();//start timeline animation

}

void Dialplate::onViewDidAppear()
{
    timer = lv_timer_create(onTimerUpdate, 1000, this);//create timer

}

void Dialplate::onViewWillDisappear()
{
    lv_group_t* group = lv_group_get_default();
    LV_ASSERT_NULL(group);
    lastFocus = lv_group_get_focused(group);//last Focus btn
    lv_group_remove_all_objs(group);//remove all object from group
    lv_timer_del(timer);//delete timer
    //View.AppearAnimStart(true);

}

void Dialplate::onViewDidDisappear()
{
}

void Dialplate::onViewUnload()
{
    Model.Deinit();
    View.Delete();
}

void Dialplate::onViewDidUnload()
{

}

void Dialplate::AttachEvent(lv_obj_t* obj)
{
    lv_obj_add_event_cb(obj, onEvent, LV_EVENT_ALL, this);//传递this指针
}

void Dialplate::Update()
{
    char buf[16];
    lv_label_set_text_fmt(View.ui.topInfo.labelSpeed, "%02d", (int)Model.GetSpeed());

    lv_label_set_text_fmt(View.ui.bottomInfo.labelInfoGrp[0].lableValue, "%0.1f km/h", Model.GetAvgSpeed());
    lv_label_set_text(
        View.ui.bottomInfo.labelInfoGrp[1].lableValue,
        DataProc::MakeTimeString(Model.sportStatusInfo.singleTime, buf, sizeof(buf))
    );
    lv_label_set_text_fmt(
        View.ui.bottomInfo.labelInfoGrp[2].lableValue,
        "%0.1f km",
        Model.sportStatusInfo.singleDistance / 1000
    );
    lv_label_set_text_fmt(
        View.ui.bottomInfo.labelInfoGrp[3].lableValue,
        "%d k",
        int(Model.sportStatusInfo.singleCalorie)
    );
}

void Dialplate::onTimerUpdate(lv_timer_t* timer)
{
    Dialplate* instance = (Dialplate*)timer->user_data;

    instance->Update();//update data
}

//switch page
void Dialplate::onBtnClicked(lv_obj_t* btn)
{
    if (btn == View.ui.btnCont.btnMap)
    {
        _Manager->Push("Pages/LiveMap");
    }
    else if (btn == View.ui.btnCont.btnMenu)
    {
        _Manager->Push("Pages/SystemInfos");
    }
}

/**
 * @brief 长按开始，短按暂停后，再次长按两次结束。或者短按暂停后，再次短按继续
 *        控制Record account 的状态，而Record account也会控制TrackFilter account的状态
 * @param longPress 是否长按
 */
void Dialplate::onRecord(bool longPress)
{
    switch (recState)
    {
    case RECORD_STATE_READY:
        if (longPress)
        {
            if (!Model.GetGPSReady())
            {
                LV_LOG_WARN("GPS has not ready, can't start record");
                Model.PlayMusic("Error");
                return;
            }

            Model.PlayMusic("Connect");
            Model.RecorderCommand(Model.REC_START);
            SetBtnRecImgSrc("pause");
            recState = RECORD_STATE_RUN;
        }
        break;
    case RECORD_STATE_RUN:
        if (!longPress)
        {
            Model.PlayMusic("UnstableConnect");
            Model.RecorderCommand(Model.REC_PAUSE);
            SetBtnRecImgSrc("start");
            recState = RECORD_STATE_PAUSE;
        }
        break;
    case RECORD_STATE_PAUSE:
        if (longPress)
        {
            Model.PlayMusic("NoOperationWarning");
            SetBtnRecImgSrc("stop");
            Model.RecorderCommand(Model.REC_READY_STOP);
            recState = RECORD_STATE_STOP;
        }
        else
        {
            Model.PlayMusic("Connect");
            Model.RecorderCommand(Model.REC_CONTINUE);
            SetBtnRecImgSrc("pause");
            recState = RECORD_STATE_RUN;
        }
        break;
    case RECORD_STATE_STOP:
        if (longPress)
        {
            Model.PlayMusic("Disconnect");
            Model.RecorderCommand(Model.REC_STOP);
            SetBtnRecImgSrc("start");
            recState = RECORD_STATE_READY;
        }
        else
        {
            Model.PlayMusic("Connect");
            Model.RecorderCommand(Model.REC_CONTINUE);
            SetBtnRecImgSrc("pause");
            recState = RECORD_STATE_RUN;
        }
        break;
    default:
        break;
    }
}
//set btn rec image
void Dialplate::SetBtnRecImgSrc(const char* srcName)
{
    lv_obj_set_style_bg_img_src(View.ui.btnCont.btnRec, ResourcePool::GetImage(srcName), 0);
}

void Dialplate::onEvent(lv_event_t* event)
{
    Dialplate* instance = (Dialplate*)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t* obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);


    //btn clicked
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        instance->onBtnClicked(obj);
    }

    if (obj == instance->View.ui.btnCont.btnRec)
    {
        if (code == LV_EVENT_SHORT_CLICKED)
        {
            instance->onRecord(false);
        }
        else if (code == LV_EVENT_LONG_PRESSED)
        {
            instance->onRecord(true);
        }
    }
}
