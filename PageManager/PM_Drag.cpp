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
#include "PageManager.h"
#include "PM_Log.h"
#include <stdlib.h>

#define CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

/* The distance threshold to trigger the drag */
#define PM_INDEV_DEF_DRAG_THROW    20 //0~100 相当于衰减系数0.2

/**
  * @brief  Page drag event callback
  * @param  event: Pointer to event structure
  * @retval None
  */
void PageManager::onRootDragEvent(lv_event_t* event)
{
    lv_event_code_t eventCode = lv_event_get_code(event);

    //当用户按下触摸屏时，会触发 LV_EVENT_PRESSED 事件。该事件表示触摸屏被按下，但尚未移动。
    //如果用户在触摸屏上移动手指，会触发 LV_EVENT_PRESSING 事件。该事件表示触摸屏正在被按下并移动。
    //当用户松开触摸屏时，会触发 LV_EVENT_RELEASED 事件。该事件表示触摸屏被释放。
    if (!(eventCode == LV_EVENT_PRESSED || eventCode == LV_EVENT_PRESSING || eventCode == LV_EVENT_RELEASED))
    {
        return;
    }
    //根据lv_obj_add_event_cb参入的参数
    lv_obj_t* root = lv_event_get_current_target(event);
    PageBase* base = (PageBase*)lv_event_get_user_data(event);

    if (base == nullptr)
    {
        PM_LOG_ERROR("Page base is NULL");
        return;
    }

    PageManager* manager = base->_Manager;
    LoadAnimAttr_t animAttr;

    //获取切换page的anim
    if (!manager->GetCurrentLoadAnimAttr(&animAttr))
    {
        PM_LOG_ERROR("Can't get current anim attr");
        return;
    }
    //LV_EVENT_PRESSED事件，中断系统当前的anim，并且向bottompage clear LV_OBJ_FLAG_HIDDEN flag
    if (eventCode == LV_EVENT_PRESSED)
    {
        if (manager->_AnimState.IsSwitchReq)
        {
            return;
        }

        if (!manager->_AnimState.IsBusy)
        {
            return;
        }  

        PM_LOG_INFO("Root anim interrupted");
        lv_anim_del(root, animAttr.setter);
        manager->_AnimState.IsBusy = false;//clear _AnimState.IsBusy flag

        /* Temporary showing the bottom page */
        PageBase* bottomPage = manager->GetStackTopAfter();
        lv_obj_clear_flag(bottomPage->_root, LV_OBJ_FLAG_HIDDEN);
    }
    //LV_EVENT_PRESSING事件，则根据拖动方向计算当前页面的位置，并设置页面的位置。
    else if (eventCode == LV_EVENT_PRESSING)
    {
        lv_coord_t cur = animAttr.getter(root);//获取当前页面的上一次的位置

        lv_coord_t max = std::max(animAttr.pop.exit.start, animAttr.pop.exit.end);
        lv_coord_t min = std::min(animAttr.pop.exit.start, animAttr.pop.exit.end);

        //该函数返回的是输入设备在 X 和 Y 方向上的偏移量，即输入设备当前位置与上一次位置之间的差值。
        lv_point_t offset;
        lv_indev_get_vect(lv_indev_get_act(), &offset);

        if (animAttr.dragDir == ROOT_DRAG_DIR_HOR)
        {
            cur += offset.x;
        }
        else if (animAttr.dragDir == ROOT_DRAG_DIR_VER)
        {
            cur += offset.y;
        }

        animAttr.setter(root, CONSTRAIN(cur, min, max));//设置页面drag后的位置
    }
    //LV_EVENT_RELEASED事件，则根据拖动的距离和方向判断是否需要切换页面。
    else if (eventCode == LV_EVENT_RELEASED)
    {
        if (manager->_AnimState.IsSwitchReq)
        {
            return;
        }
        
        lv_coord_t offset_sum = animAttr.push.enter.end - animAttr.push.enter.start;

        lv_coord_t x_predict = 0;
        lv_coord_t y_predict = 0;
        RootGetDragPredict(&x_predict, &y_predict);//释放时根据drag的偏移量，添加一个移动惯性后，计算出的预测值x,y

        lv_coord_t start = animAttr.getter(root);//获取当前页面的上一次的位置(相对于父对象的坐标)
        lv_coord_t end = start;

        if (animAttr.dragDir == ROOT_DRAG_DIR_HOR)
        {
            end += x_predict;
            PM_LOG_INFO("Root drag x_predict = %d", end);
        }
        else if (animAttr.dragDir == ROOT_DRAG_DIR_VER)
        {
            end += y_predict;
            PM_LOG_INFO("Root drag y_predict = %d", end);
        }

        if (std::abs(end) > std::abs((int)offset_sum) / 2)//大于一半
        {
            lv_async_call(onRootAsyncLeave, base);//在下一次调用 lv_timer_handler 时被调用  send LV_EVENT_LEAVE 表示鼠标或触摸屏离开对象的事件。
        }
        else if(end != animAttr.push.enter.end)//恢复原样
        {
            manager->_AnimState.IsBusy = true;//set _AnimState.IsBusy flag

            lv_anim_t a;
            manager->AnimDefaultInit(&a);
            lv_anim_set_user_data(&a, manager);
            lv_anim_set_var(&a, root);
            lv_anim_set_values(&a, start, animAttr.push.enter.end);
            lv_anim_set_exec_cb(&a, animAttr.setter);
            lv_anim_set_ready_cb(&a, onRootDragAnimFinish);//hide the bottom page，向其添加LV_OBJ_FLAG_HIDDEN
            lv_anim_start(&a);
            PM_LOG_INFO("Root drag anim start");
        }
    }
}

/**
  * @brief  Drag animation end event callback
  * @param  a: Pointer to animation
  * @retval None
  */
void PageManager::onRootDragAnimFinish(lv_anim_t* a)
{
    PageManager* manager = (PageManager*)lv_anim_get_user_data(a);
    PM_LOG_INFO("Root drag anim finish");
    manager->_AnimState.IsBusy = false;//clear _AnimState.IsBusy flag

    /* Hide the bottom page */
    PageBase* bottomPage = manager->GetStackTopAfter();
    if (bottomPage)
    {
        lv_obj_add_flag(bottomPage->_root, LV_OBJ_FLAG_HIDDEN);//向bottompage 添加LV_OBJ_FLAG_HIDDEN
    }
}

/**
  * @brief  Enable root's drag function
  * @param  root: Pointer to the root object
  * @retval None
  */
void PageManager::RootEnableDrag(lv_obj_t* root)
{
    PageBase* base = (PageBase*)lv_obj_get_user_data(root);
    lv_obj_add_event_cb(
        root,
        onRootDragEvent,
        LV_EVENT_ALL,
        base
    );
    PM_LOG_INFO("Page(%s) Root drag enabled", base->_Name);
}

/**
  * @brief  Asynchronous callback when dragging ends
  * @param  data: Pointer to the base class of the page
  * @retval None
  */
void PageManager::onRootAsyncLeave(void* data)
{
    PageBase* base = (PageBase*)data;
    PM_LOG_INFO("Page(%s) send event: LV_EVENT_LEAVE, need to handle...", base->_Name);
    lv_event_send(base->_root, LV_EVENT_LEAVE, base);//send LV_EVENT_LEAVE
}

/**
  * @brief  Get drag inertia prediction stop point   添加一个移动惯性
  * @param  x: x stop point
  * @param  y: y stop point
  * @retval None
  */
void PageManager::RootGetDragPredict(lv_coord_t* x, lv_coord_t* y)
{
    lv_indev_t* indev = lv_indev_get_act();
    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);

    lv_coord_t y_predict = 0;
    lv_coord_t x_predict = 0;

    while (vect.y != 0)
    {
        y_predict += vect.y;
        vect.y = vect.y * (100 - PM_INDEV_DEF_DRAG_THROW) / 100;
    }

    while (vect.x != 0)
    {
        x_predict += vect.x;
        vect.x = vect.x * (100 - PM_INDEV_DEF_DRAG_THROW) / 100;
    }

    *x = x_predict;
    *y = y_predict;
}
