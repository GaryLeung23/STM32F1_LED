#include "StartUp.h"

using namespace Page;

Startup::Startup()
{
}

Startup::~Startup()
{

}

void Startup::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Startup::onViewLoad()
{
    Model.Init();
//    Model.SetEncoderEnable(false);
    View.Create(_root);
    lv_timer_t* timer = lv_timer_create(onTimer, 2000, this);
    lv_timer_set_repeat_count(timer, 1);//timer只重复一次
}

void Startup::onViewDidLoad()
{
}

void Startup::onViewWillAppear()
{
    Model.PlayMusic("Startup");
    lv_anim_timeline_start(View.ui.anim_timeline);//start timeline
}

void Startup::onViewDidAppear()
{
    lv_obj_fade_out(_root, 500, 1500);//淡出动画  delay: 500ms, duration: 1500ms
}

void Startup::onViewWillDisappear()
{

}

void Startup::onViewDidDisappear()
{
    Model.SetStatusBarAppear(true);//显示状态栏
}

void Startup::onViewUnload()
{
    View.Delete();
//    Model.SetEncoderEnable(true);
    Model.Deinit();
}

void Startup::onViewDidUnload()
{
}

void Startup::onTimer(lv_timer_t* timer)
{
    Startup* instance = (Startup*)timer->user_data;

    instance->_Manager->Replace("Pages/Dialplate");//replace Dialplate page
}

void Startup::onEvent(lv_event_t* event)
{
    Startup* instance = (Startup*)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t* obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (obj == instance->_root)
    {
        if (code == LV_EVENT_LEAVE)
        {
            //instance->Manager->Pop();
        }
    }
}
