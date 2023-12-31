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
   * @brief  Enter a new page, replace the old page
   * @param  name: The name of the page to enter
   * @param  stash: Parameters passed to the new page
   * @retval Return true if successful
   */
bool PageManager::Replace(const char* name, const PageBase::Stash_t* stash)
{
    /* Check whether the animation of switching pages is being executed */
    if (!SwitchAnimStateCheck())
    {
        return false;
    }

    /* Check whether the stack is repeatedly pushed  */
    if (FindPageInStack(name) != nullptr)
    {
        PM_LOG_ERROR("Page(%s) was multi push", name);
        return false;
    }

    /* Check if the page is registered in the page pool */
    PageBase* base = FindPageInPool(name);

    if (base == nullptr)
    {
        PM_LOG_ERROR("Page(%s) was not install", name);
        return false;
    }

    /* Get the top page of the stack */
    PageBase* top = GetStackTop();

    if (top == nullptr)
    {
        PM_LOG_ERROR("Stack top is NULL");
        return false;
    }

    /* Force disable cache */
    top->priv.IsCached = false;

    /* Synchronous automatic cache configuration */
    base->priv.IsDisableAutoCache = base->priv.ReqDisableAutoCache;

    /* Remove current page */
    _PageStack.pop();

    /* Push into the stack */
    _PageStack.push(base);

    PM_LOG_INFO("Page(%s) replace Page(%s) (stash = 0x%p)", name, top->_Name, stash);

    /* Page switching execution */
    return SwitchTo(base, true, stash);
}

/**
  * @brief  Enter a new page, the old page is pushed onto the stack
  * @param  name: The name of the page to enter 
  * @param  stash: Parameters passed to the new page
  * @retval Return true if successful
  */
bool PageManager::Push(const char* name, const PageBase::Stash_t* stash)
{
    /* Check whether the animation of switching pages is being executed */
    if (!SwitchAnimStateCheck())
    {
        return false;
    }

    /* Check whether the stack is repeatedly pushed  */
    if (FindPageInStack(name) != nullptr)
    {
        PM_LOG_ERROR("Page(%s) was multi push", name);
        return false;
    }

    /* Check if the page is registered in the page pool */
    PageBase* base = FindPageInPool(name);

    if (base == nullptr)
    {
        PM_LOG_ERROR("Page(%s) was not install", name);
        return false;
    }

    /* Synchronous automatic cache configuration */
    base->priv.IsDisableAutoCache = base->priv.ReqDisableAutoCache;

    /* Push into the stack */
    _PageStack.push(base);

    PM_LOG_INFO("Page(%s) push >> [Screen] (stash = 0x%p)", name, stash);

    /* Page switching execution */
    return SwitchTo(base, true, stash);
}

/**
  * @brief  Pop the current page
  * @param  None
  * @retval Return true if successful
  */
bool PageManager::Pop()
{
    /* Check whether the animation of switching pages is being executed */
    if (!SwitchAnimStateCheck())
    {
        return false;
    }

    /* Get the top page of the stack */
    PageBase* top = GetStackTop();

    if (top == nullptr)
    {
        PM_LOG_WARN("Page stack is empty, cat't pop");
        return false;
    }

    /* Whether to turn off automatic cache */
    if (!top->priv.IsDisableAutoCache)
    {
        PM_LOG_INFO("Page(%s) has auto cache, cache disabled", top->_Name);
        top->priv.IsCached = false;
    }

    PM_LOG_INFO("Page(%s) pop << [Screen]", top->_Name);

    /* Page popup */
    _PageStack.pop();

    /* Get the next page */
    top = GetStackTop();

    /* Page switching execution */
    return SwitchTo(top, false, nullptr);;
}

/**
  * @brief  Page switching
  * @param  newNode: Pointer to new page
  * @param  isEnterAct: Whether it is a ENTER action   页面切换分为enter exit两个情况,replace、push为enter，pop为exit
  * @param  stash: Parameters passed to the new page
  * @retval Return true if successful
  */
bool PageManager::SwitchTo(PageBase* newNode, bool isEnterAct, const PageBase::Stash_t* stash)
{
    if (newNode == nullptr)
    {
        PM_LOG_ERROR("newNode is nullptr");
        return false;
    }

    /* Whether page switching has been requested */
    if (_AnimState.IsSwitchReq)
    {
        PM_LOG_WARN("Page switch busy, reqire(%s) is ignore", newNode->_Name);
        return false;
    }

    _AnimState.IsSwitchReq = true;//set page switch request flag

    /* Is there a parameter to pass */
    if (stash != nullptr)
    {
        //将stash传递给new page
        PM_LOG_INFO("stash is detect, %s >> stash(0x%p) >> %s", GetPagePrevName(), stash, newNode->_Name);

        void* buffer = nullptr;
        //若newpage还没有stash，则为buffer分配内存
        if (newNode->priv.Stash.ptr == nullptr)
        {
            buffer = lv_malloc(stash->size);
            if (buffer == nullptr)
            {
                PM_LOG_ERROR("stash malloc failed");
            }
            else
            {
                PM_LOG_INFO("stash(0x%p) malloc[%d]", buffer, stash->size);
            }
        }
        //若newpage的stash大小一致,则直接使用newpage的stash，否则不进行传递
        else if(newNode->priv.Stash.size == stash->size)
        {
            buffer = newNode->priv.Stash.ptr;
            PM_LOG_INFO("stash(0x%p) is exist", buffer);
        }

        //若buffer不为空，则将stash拷贝到buffer中，并将buffer赋值给newpage的stash
        if (buffer != nullptr)
        {
            memcpy(buffer, stash->ptr, stash->size);
            PM_LOG_INFO("stash memcpy[%d] 0x%p >> 0x%p", stash->size, stash->ptr, buffer);
            newNode->priv.Stash.ptr = buffer;
            newNode->priv.Stash.size = stash->size;
        }
    }

    /* Record current page */
    _PageCurrent = newNode;

    //设置_PageCurrent的初始页面状态
    /* If the current page has a cache */
    if (_PageCurrent->priv.IsCached)
    {
        /* Direct display, no need to load */
        PM_LOG_INFO("Page(%s) has cached, appear driectly", _PageCurrent->_Name);
        _PageCurrent->priv.State = PageBase::PAGE_STATE_WILL_APPEAR;
    }
    else
    {
        /* Load page */
        _PageCurrent->priv.State = PageBase::PAGE_STATE_LOAD;//
    }

    //更新_PagePrev与_PageCurrent的IsEnter的状态机
    if (_PagePrev != nullptr)
    {
        _PagePrev->priv.Anim.IsEnter = false;//not entering party 
    }

    _PageCurrent->priv.Anim.IsEnter = true;//entering party 

    _AnimState.IsEntering = isEnterAct;//IsEntering setting

    //对于enter状态，用_PageCurrent更新系统的anim配置。对于exit状态，在onSwitchAnimFinish中设置
    if (_AnimState.IsEntering)
    {
        /* Update the animation configuration according to the current page */
        SwitchAnimTypeUpdate(_PageCurrent);
    }

    /* Update the state machine of the previous page */
    StateUpdate(_PagePrev);//PAGE_STATE_ACTIVITY -> PAGE_STATE_WILL_DISAPPEAR

    /* Update the state machine of the current page */
    StateUpdate(_PageCurrent);//如果IsCached为true是PAGE_STATE_LOAD->PAGE_STATE_WILL_APPEAR,否则PAGE_STATE_WILL_APPEAR -> PAGE_STATE_DID_APPEAR

    /* Move the layer, move the new page to the front */
    if (_AnimState.IsEntering)
    {
        PM_LOG_INFO("Page ENTER is detect, move Page(%s) to foreground", _PageCurrent->_Name);
        if (_PagePrev)
					lv_obj_move_foreground(_PagePrev->_root);
        lv_obj_move_foreground(_PageCurrent->_root);
    }
    else
    {
        PM_LOG_INFO("Page EXIT is detect, move Page(%s) to foreground", GetPagePrevName());
        lv_obj_move_foreground(_PageCurrent->_root);
        if (_PagePrev)
					lv_obj_move_foreground(_PagePrev->_root);
    }
    return true;
}

/**
  * @brief  Force the end of the life cycle of the page without animation 
  * @param  base: Pointer to the page being executed
  * @retval Return true if successful
  */
bool PageManager::FourceUnload(PageBase* base)
{
    if (base == nullptr)
    {
        PM_LOG_ERROR("Page is nullptr, Unload failed");
        return false;
    }

    PM_LOG_INFO("Page(%s) Fource unloading...", base->_Name);

    if (base->priv.State == PageBase::PAGE_STATE_ACTIVITY)
    {
        PM_LOG_INFO("Page state is ACTIVITY, Disappearing...");
        base->onViewWillDisappear();
        base->onViewDidDisappear();
    }

    base->priv.State = StateUnloadExecute(base);

    return true;
}

/**
  * @brief  Back to the main page (the page at the bottom of the stack) 
  * @param  None
  * @retval Return true if successful
  */
bool PageManager::BackHome()
{
    /* Check whether the animation of switching pages is being executed */
    if (!SwitchAnimStateCheck())
    {
        return false;
    }

    SetStackClear(true);

    _PagePrev = nullptr;

    PageBase* home = GetStackTop();

    SwitchTo(home, false);

    return true;
}

/**
  * @brief  Check if the page switching animation is being executed
  * @param  None
  * @retval Return true if it is executing
  */
bool PageManager::SwitchAnimStateCheck()
{
    if (_AnimState.IsSwitchReq || _AnimState.IsBusy)
    {
        PM_LOG_WARN(
            "Page switch busy[AnimState.IsSwitchReq = %d,"
            "AnimState.IsBusy = %d],"
            "request ignored",
            _AnimState.IsSwitchReq,
            _AnimState.IsBusy
        );
        return false;//说明当前系统有一个页面转换被请求或正在执行，此时不能进行新的页面转换，所以函数要返回false
    }

    return true;
}

/**
  * @brief  Page switching request check 
  * @param  None
  * @retval Return true if all pages are executed
  */
bool PageManager::SwitchReqCheck()
{
    bool ret = false;
    
    bool lastNodeBusy = _PagePrev && _PagePrev->priv.Anim.IsBusy;
    //_PagePrev与_PageCurrent不busy,说明_PagePrev与_PageCurrent页面转换已经完成
    if (!_PageCurrent->priv.Anim.IsBusy && !lastNodeBusy)
    {
        PM_LOG_INFO("----Page switch was all finished----");
        _AnimState.IsSwitchReq = false;//clear page switch request flag
        ret = true;//
        _PagePrev = _PageCurrent;
    }
    else
    {
        if (_PageCurrent->priv.Anim.IsBusy)
        {
            PM_LOG_WARN("Page PageCurrent(%s) is busy", _PageCurrent->_Name);
        }
        else
        {
            PM_LOG_WARN("Page PagePrev(%s) is busy", GetPagePrevName());
        }
    }

    return ret;
}

/**
  * @brief  PPage switching animation execution end callback 
  * @param  a: Pointer to animation
  * @retval None
  */
void PageManager::onSwitchAnimFinish(lv_anim_t* a)
{
    PageBase* base = (PageBase*)lv_anim_get_user_data(a);
    PageManager* manager = base->_Manager;

    PM_LOG_INFO("Page(%s) Anim finish", base->_Name);

    manager->StateUpdate(base);//更新页面状态机PAGE_STATE_DID_APPEAR -> PAGE_STATE_ACTIVITY 或者 PAGE_STATE_DID_DISAPPEAR -> PAGE_STATE_UNLOAD
    base->priv.Anim.IsBusy = false;	//clear priv.Anim.IsBusy flag
    bool isFinished = manager->SwitchReqCheck();//Page switch was all finished

    //对于exit状态，用_PageCurrent更新系统的anim配置
    if (!manager->_AnimState.IsEntering && isFinished)
    {
        manager->SwitchAnimTypeUpdate(manager->_PageCurrent);
    }
}

/**
  * @brief  Create page switching animation
  * @param  a: Point to the animated page
  * @retval None
  */
void PageManager::SwitchAnimCreate(PageBase* base)
{
    LoadAnimAttr_t animAttr;
    if (!GetCurrentLoadAnimAttr(&animAttr))
    {
        return;
    }

    lv_anim_t a;
    AnimDefaultInit(&a);
    lv_anim_set_user_data(&a, base);
    lv_anim_set_var(&a, base->_root);
    lv_anim_set_ready_cb(&a, onSwitchAnimFinish);//设置动画结束回调函数，onSwitchAnimFinish会更新page状态机
    lv_anim_set_exec_cb(&a, animAttr.setter);

    int32_t start = 0;

    if (animAttr.getter)
    {
        start = animAttr.getter(base->_root);
    }

    if (_AnimState.IsEntering)/* Push */
    {
        if (base->priv.Anim.IsEnter)
        {
            lv_anim_set_values(
                &a,
                animAttr.push.enter.start,
                animAttr.push.enter.end
            );
        }
        else /* Exit */
        {
            lv_anim_set_values(
                &a,
                start,
                animAttr.push.exit.end
            );
        }
    }
    else /* Pop */
    {
        if (base->priv.Anim.IsEnter)
        {
            lv_anim_set_values(
                &a,
                animAttr.pop.enter.start,
                animAttr.pop.enter.end
            );
        }
        else /* Exit */
        {
            lv_anim_set_values(
                &a,
                start,
                animAttr.pop.exit.end
            );
        }
    }

    lv_anim_start(&a);
    base->priv.Anim.IsBusy = true;	//set priv.Anim.IsBusy flag
}

/**
  * @brief  Set global animation properties 
  * @param  anim: Animation type
  * @param  time: Animation duration
  * @param  path: Animation curve
  * @retval None
  */
void PageManager::SetGlobalLoadAnimType(LoadAnim_t anim, uint16_t time, lv_anim_path_cb_t path)
{
    if (anim > _LOAD_ANIM_LAST)
    {
        anim = LOAD_ANIM_NONE;
    }

    _AnimState.Global.Type = anim;
    _AnimState.Global.Time = time;
    _AnimState.Global.Path = path;

    PM_LOG_INFO("Set global load anim type = %d", anim);
}

/**
  * @brief  Update current animation properties, apply page custom animation
  * @param  base: Pointer to page
  * @retval None
  */
void PageManager::SwitchAnimTypeUpdate(PageBase* base)
{
    if (base->priv.Anim.Attr.Type == LOAD_ANIM_GLOBAL)
    {
        PM_LOG_INFO(
            "Page(%s) Anim.Type was not set, use AnimState.Global.Type = %d",
            base->_Name,
            _AnimState.Global.Type
        );
        _AnimState.Current = _AnimState.Global;
    }
    else
    {
        if (base->priv.Anim.Attr.Type > _LOAD_ANIM_LAST)
        {
            PM_LOG_ERROR(
                "Page(%s) ERROR custom Anim.Type = %d, use AnimState.Global.Type = %d",
                base->_Name,
                base->priv.Anim.Attr.Type,
                _AnimState.Global.Type
            );
            base->priv.Anim.Attr = _AnimState.Global;
        }
        else
        {
            PM_LOG_INFO(
                "Page(%s) custom Anim.Type set = %d",
                base->_Name,
                base->priv.Anim.Attr.Type
            );
        }
        _AnimState.Current = base->priv.Anim.Attr;
    }
}

/**
  * @brief  Set animation default parameters
  * @param  a: Pointer to animation
  * @retval None
  */
void PageManager::AnimDefaultInit(lv_anim_t* a)
{
    lv_anim_init(a);

    uint32_t time = (GetCurrentLoadAnimType() == LOAD_ANIM_NONE) ? 0 : _AnimState.Current.Time;
    lv_anim_set_time(a, time);
    lv_anim_set_path_cb(a, _AnimState.Current.Path);
}
