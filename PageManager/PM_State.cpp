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

/**
  * @brief  Update page state machine
  * @param  base: Pointer to the updated page
  * @retval None
  */
void PageManager::StateUpdate(PageBase* base)
{
    if (base == nullptr)
        return;

    switch (base->priv.State)
    {
    case PageBase::PAGE_STATE_IDLE:
        PM_LOG_INFO("Page(%s) state idle", base->_Name);
        break;

    case PageBase::PAGE_STATE_LOAD:
        base->priv.State = StateLoadExecute(base);
        StateUpdate(base);//PAGE_STATE_WILL_APPEAR -> PAGE_STATE_DID_APPEAR
        break;

    case PageBase::PAGE_STATE_WILL_APPEAR:
        base->priv.State = StateWillAppearExecute(base);
        break;

    case PageBase::PAGE_STATE_DID_APPEAR:
        base->priv.State = StateDidAppearExecute(base);
        PM_LOG_INFO("Page(%s) state active", base->_Name);
        break;

    case PageBase::PAGE_STATE_ACTIVITY:
        PM_LOG_INFO("Page(%s) state active break", base->_Name);
        base->priv.State = PageBase::PAGE_STATE_WILL_DISAPPEAR;
        StateUpdate(base);//PAGE_STATE_WILL_DISAPPEAR ->   PAGE_STATE_DID_DISAPPEAR
        break;

    case PageBase::PAGE_STATE_WILL_DISAPPEAR:
        base->priv.State = StateWillDisappearExecute(base);
        break;

    case PageBase::PAGE_STATE_DID_DISAPPEAR:
        base->priv.State = StateDidDisappearExecute(base);
        if (base->priv.State == PageBase::PAGE_STATE_UNLOAD)
        {
            StateUpdate(base);//PAGE_STATE_UNLOAD ->  PAGE_STATE_IDLE 
        }
        break;

    case PageBase::PAGE_STATE_UNLOAD:
        base->priv.State = StateUnloadExecute(base);
        break;

    default:
        PM_LOG_ERROR("Page(%s) state[%d] was NOT FOUND!", base->_Name, base->priv.State);
        break;
    }
}

/**
  * @brief  Page loading status
  * @param  base: Pointer to the updated page
  * @retval Next state
  */
PageBase::State_t PageManager::StateLoadExecute(PageBase* base)
{
    PM_LOG_INFO("Page(%s) state load", base->_Name);

    
    if (base->_root != nullptr)
    {
        PM_LOG_ERROR("Page(%s) root must be nullptr", base->_Name);
    }

    lv_obj_t* root_obj = lv_obj_create(lv_scr_act());// create root obj
    
    lv_obj_clear_flag(root_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(root_obj, base);// set user data,RootEnableDrag里面会用到

    //对root_obj设置默认的style
    if (_RootDefaultStyle)
    {
        lv_obj_add_style(root_obj, _RootDefaultStyle, LV_PART_MAIN);
    }

    base->_root = root_obj;//_root在这里赋值
    base->onViewLoad();//调用该page 重载的onViewLoad函数

    //anim是Over类型的anim
    if (GetIsOverAnim(GetCurrentLoadAnimType()))
    {
        PageBase* bottomPage = GetStackTopAfter();
        //如果bottompage不为空且IsCached为true
        if (bottomPage != nullptr && bottomPage->priv.IsCached)
        {
            LoadAnimAttr_t animAttr;
            if (GetCurrentLoadAnimAttr(&animAttr))
            {
                if (animAttr.dragDir != ROOT_DRAG_DIR_NONE)
                {
                    RootEnableDrag(base->_root);//Enable root's drag function
                }
            }
        }
    }

    base->onViewDidLoad();//调用该page 重载的onViewDidLoad函数

    //set IsCached flag
    if (base->priv.IsDisableAutoCache)
    {
        PM_LOG_INFO("Page(%s) disable auto cache, ReqEnableCache = %d", base->_Name, base->priv.ReqEnableCache);
        base->priv.IsCached = base->priv.ReqEnableCache;
    }
    else
    {
        PM_LOG_INFO("Page(%s) AUTO cached", base->_Name);
        base->priv.IsCached = true;
    }

    return PageBase::PAGE_STATE_WILL_APPEAR;
}

/**
  * @brief  The page is about to show the status
  * @param  base: Pointer to the updated page
  * @retval Next state
  */
PageBase::State_t PageManager::StateWillAppearExecute(PageBase* base)
{
    PM_LOG_INFO("Page(%s) state will appear", base->_Name);
    base->onViewWillAppear();///调用该page 重载的onViewWillAppear函数
    lv_obj_clear_flag(base->_root, LV_OBJ_FLAG_HIDDEN);//clear hidden flag
    SwitchAnimCreate(base);//page switch anim
    return PageBase::PAGE_STATE_DID_APPEAR;
}

/**
  * @brief  The status of the page display
  * @param  base: Pointer to the updated page
  * @retval Next state
  */
PageBase::State_t PageManager::StateDidAppearExecute(PageBase* base)
{
    PM_LOG_INFO("Page(%s) state did appear", base->_Name);
    base->onViewDidAppear();//调用该page 重载的onViewDidAppear函数
    return PageBase::PAGE_STATE_ACTIVITY;
}

/**
  * @brief  The page is about to disappear
  * @param  base: Pointer to the updated page
  * @retval Next state
  */
PageBase::State_t PageManager::StateWillDisappearExecute(PageBase* base)
{
    PM_LOG_INFO("Page(%s) state will disappear", base->_Name);
    base->onViewWillDisappear();///调用该page 重载的onViewWillDisappear函数
    SwitchAnimCreate(base);////page switch anim
    return PageBase::PAGE_STATE_DID_DISAPPEAR;
}

/**
  * @brief  Page disappeared end state
  * @param  base: Pointer to the updated page
  * @retval Next state
  */
PageBase::State_t PageManager::StateDidDisappearExecute(PageBase* base)
{
    PM_LOG_INFO("Page(%s) state did disappear", base->_Name);
    lv_obj_add_flag(base->_root, LV_OBJ_FLAG_HIDDEN);////set hidden flag
    base->onViewDidDisappear();///调用该page 重载的onViewDidDisappear函数
    if (base->priv.IsCached)
    {
        PM_LOG_INFO("Page(%s) has cached", base->_Name);
        return PageBase::PAGE_STATE_WILL_APPEAR;
    }
    else
    {
        return PageBase::PAGE_STATE_UNLOAD;
    }
}

/**
  * @brief  Page unload complete
  * @param  base: Pointer to the updated page
  * @retval Next state
  */
PageBase::State_t PageManager::StateUnloadExecute(PageBase* base)
{
    PM_LOG_INFO("Page(%s) state unload", base->_Name);
    if (base->_root == nullptr)
    {
        PM_LOG_WARN("Page is loaded!");
        goto Exit;
    }

    base->onViewUnload();//调用该page 重载的onViewUnload函数

    //释放Stash
    if (base->priv.Stash.ptr != nullptr && base->priv.Stash.size != 0)
    {
        PM_LOG_INFO("Page(%s) free stash(0x%p)[%d]", base->_Name, base->priv.Stash.ptr, base->priv.Stash.size);
        lv_free(base->priv.Stash.ptr);
        base->priv.Stash.ptr = nullptr;
        base->priv.Stash.size = 0;
    }

    /* Delete after the end of the root animation life cycle */
    lv_obj_del_async(base->_root);//它会在下次调用 lv_timer_handler 时删除该对象
    base->_root = nullptr;
    base->priv.IsCached = false;//clear IsCached flag
    base->onViewDidUnload();//调用该page 重载的onViewDidUnload函数

Exit:
    return PageBase::PAGE_STATE_IDLE;
}
