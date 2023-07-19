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
#ifndef __PAGE_MANAGER_H
#define __PAGE_MANAGER_H

#include "PageBase.h"
#include "PageFactory.h"
#include <vector>
#include <stack>

class PageManager
{
public:

    /* Page switching animation type  */
    typedef enum
    {
        /* Default (global) animation type  */
        LOAD_ANIM_GLOBAL = 0,

        /* New page overwrites old page  */
        LOAD_ANIM_OVER_LEFT,
        LOAD_ANIM_OVER_RIGHT,
        LOAD_ANIM_OVER_TOP,
        LOAD_ANIM_OVER_BOTTOM,

        /* New page pushes old page  */
        LOAD_ANIM_MOVE_LEFT,
        LOAD_ANIM_MOVE_RIGHT,
        LOAD_ANIM_MOVE_TOP,
        LOAD_ANIM_MOVE_BOTTOM,

        /* The new interface fades in, the old page fades out */
        LOAD_ANIM_FADE_ON,

        /* No animation */
        LOAD_ANIM_NONE,

        _LOAD_ANIM_LAST = LOAD_ANIM_NONE
    } LoadAnim_t;

    /* Page dragging direction */
    typedef enum
    {
        ROOT_DRAG_DIR_NONE,
        ROOT_DRAG_DIR_HOR,
        ROOT_DRAG_DIR_VER,
    } RootDragDir_t;

    /* Animated setter */
    typedef void(*lv_anim_setter_t)(void*, int32_t);

    /* Animated getter */
    typedef int32_t(*lv_anim_getter_t)(void*);

    /* Animation switching record  */
    typedef struct
    {
        /* As the entered party */
        struct
        {
            int32_t start;
            int32_t end;
        } enter;

        /* As the exited party */
        struct
        {
            int32_t start;
            int32_t end;
        } exit;
    } AnimValue_t;

    /* Page loading animation properties */
    typedef struct
    {
        lv_anim_setter_t setter;	//函数指针  用于设置anim的值
        lv_anim_getter_t getter;	//函数指针  用于得到anim的值
        RootDragDir_t dragDir;		
        AnimValue_t push;					//
        AnimValue_t pop;					//
    } LoadAnimAttr_t;//页面切换这个过程中的动画属性

public:
    PageManager(PageFactory* factory = nullptr);
    ~PageManager();

    /* Loader */
    bool Install(const char* className, const char* appName);
    bool Uninstall(const char* appName);
    bool Register(PageBase* base, const char* name);
    bool Unregister(const char* name);

    /* Router */
    bool Replace(const char* name, const PageBase::Stash_t* stash = nullptr);
    bool Push(const char* name, const PageBase::Stash_t* stash = nullptr);
    bool Pop();
    bool BackHome();
    const char* GetPagePrevName();

    /* Global Animation */
    void SetGlobalLoadAnimType(
        LoadAnim_t anim = LOAD_ANIM_OVER_LEFT,
        uint16_t time = 500,
        lv_anim_path_cb_t path = lv_anim_path_ease_out
    );

    void SetRootDefaultStyle(lv_style_t* style)
    {
        _RootDefaultStyle = style;
    }

private:
    /* Page Pool */
    PageBase* FindPageInPool(const char* name);

    /* Page Stack */
    PageBase* FindPageInStack(const char* name);
    PageBase* GetStackTop();
    PageBase* GetStackTopAfter();
    void SetStackClear(bool keepBottom = false);
    bool FourceUnload(PageBase* base);

    /* Animation */
    bool GetLoadAnimAttr(uint8_t anim, LoadAnimAttr_t* attr);
    bool GetIsOverAnim(uint8_t anim)
    {
        return (anim >= LOAD_ANIM_OVER_LEFT && anim <= LOAD_ANIM_OVER_BOTTOM);
    }
    bool GetIsMoveAnim(uint8_t anim)
    {
        return (anim >= LOAD_ANIM_MOVE_LEFT && anim <= LOAD_ANIM_MOVE_BOTTOM);
    }
    void AnimDefaultInit(lv_anim_t* a);
    bool GetCurrentLoadAnimAttr(LoadAnimAttr_t* attr)
    {
        return GetLoadAnimAttr(GetCurrentLoadAnimType(), attr);
    }
    LoadAnim_t GetCurrentLoadAnimType()
    {
        return (LoadAnim_t)_AnimState.Current.Type;
    }

    /* Drag */
    static void onRootDragEvent(lv_event_t* event);
    static void onRootDragAnimFinish(lv_anim_t* a);
    static void onRootAsyncLeave(void* base);
    void RootEnableDrag(lv_obj_t* root);
    static void RootGetDragPredict(lv_coord_t* x, lv_coord_t* y);

    /* Switch */
    bool SwitchTo(PageBase* base, bool isEnterAct, const PageBase::Stash_t* stash = nullptr);
    static void onSwitchAnimFinish(lv_anim_t* a);
    void SwitchAnimCreate(PageBase* base);
    void SwitchAnimTypeUpdate(PageBase* base);
    bool SwitchReqCheck();
    bool SwitchAnimStateCheck();

    /* State */
    PageBase::State_t StateLoadExecute(PageBase* base);
    PageBase::State_t StateWillAppearExecute(PageBase* base);
    PageBase::State_t StateDidAppearExecute(PageBase* base);
    PageBase::State_t StateWillDisappearExecute(PageBase* base);
    PageBase::State_t StateDidDisappearExecute(PageBase* base);
    PageBase::State_t StateUnloadExecute(PageBase* base);
    void StateUpdate(PageBase* base);
    PageBase::State_t GetState()
    {
        return _PageCurrent->priv.State;
    }

private:

    /* Page factory */
    PageFactory* _Factory;

    /* Page pool */
    std::vector<PageBase*> _PagePool;

    /* Page stack */
    std::stack<PageBase*> _PageStack;

    /* Previous page */
    PageBase* _PagePrev;

    /* The current page */
    PageBase* _PageCurrent;

    /* Page animation status */
    struct
    {
        bool IsSwitchReq;              // Has switch request     当前系统是否有页面切换请求，从SwitchTo开始直到page切换动画完成
        bool IsBusy;                   // Is switching           当前系统是否busy，目前只运用在Drag操作下恢复原样的情况下。
        bool IsEntering;               // Is in entering action  当前系统页面切换是否是enter状态（还有exit状态）

        PageBase::AnimAttr_t Current;  // Current animation properties
        PageBase::AnimAttr_t Global;   // Global animation properties
    } _AnimState;//系统页面动画状态与配置,将某个page的anim配置设置到这里

    /* Root style */
    lv_style_t* _RootDefaultStyle;
};

#endif
