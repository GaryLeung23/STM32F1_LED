#include "AppFactory.h"
#include "_Template/Template.h"
#include "StatusBar/StatusBar.h"


#define ACCOUNT_SEND_CMD(ACT, CMD) \
do{ \
    DataProc::ACT##_Info_t info; \
    DATA_PROC_INIT_STRUCT(info); \
    info.cmd = DataProc::CMD; \
    DataProc::Center()->AccountMain.Notify(#ACT, &info, sizeof(info)); \
}while(0)

void App_Init() {
    static AppFactory factory;
    static PageManager manager(&factory);

#if CONFIG_MONKEY_TEST_ENABLE
    lv_monkey_config_t config;
    lv_monkey_config_init(&config);
    config.type = CONFIG_MONKEY_INDEV_TYPE;
    config.period_range.min = CONFIG_MONKEY_PERIOD_MIN;
    config.period_range.max = CONFIG_MONKEY_PERIOD_MAX;
    config.input_range.min = CONFIG_MONKEY_INPUT_RANGE_MIN;
    config.input_range.max = CONFIG_MONKEY_INPUT_RANGE_MAX;
    lv_monkey_t* monkey = lv_monkey_create(&config);
    lv_monkey_set_enable(monkey, true);

    lv_group_t* group = lv_group_create();
    lv_indev_set_group(lv_monkey_get_indev(monkey), group);
    lv_group_set_default(group);

    LV_LOG_USER("lv_monkey test started!");
#endif

    /* Make sure the default group exists */
    if (!lv_group_get_default()) {
        lv_group_t *group = lv_group_create();
        //lv_indev_set_group(indev, group);
        lv_group_set_default(group);
    }


    /* Initialize the data processing node */
    DataProc_Init();
    ACCOUNT_SEND_CMD(Storage, STORAGE_CMD_LOAD);//load json file
    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_LOAD);//Configure system settings based on json file

    /* Set screen style */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_disp_set_bg_color(lv_disp_get_default(), lv_color_black());

    /* Set root default style */
    static lv_style_t rootStyle;
    lv_style_init(&rootStyle);
    lv_style_set_width(&rootStyle, LV_HOR_RES);
    lv_style_set_height(&rootStyle, LV_VER_RES);
    lv_style_set_bg_opa(&rootStyle, LV_OPA_COVER);
    lv_style_set_bg_color(&rootStyle, lv_color_black());
    manager.SetRootDefaultStyle(&rootStyle);

    /* Initialize resource pool */
    ResourcePool::Init();

    /* Initialize status bar */
    Page::StatusBar_Create(lv_layer_top());



    /* Initialize pages */
//    manager.Install("Template", "Pages/_Template");
    manager.Install("SystemInfos", "Pages/SystemInfos");
    manager.Install("Startup", "Pages/Startup");
    manager.Install("Dialplate", "Pages/Dialplate");
    manager.Install("LiveMap", "Pages/LiveMap");

    manager.SetGlobalLoadAnimType(PageManager::LOAD_ANIM_OVER_TOP);


    manager.Push("Pages/Startup");

    ACCOUNT_SEND_CMD(Storage, STORAGE_CMD_SAVE);
}


void App_Uninit(void) {
    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_SAVE);
    ACCOUNT_SEND_CMD(Storage, STORAGE_CMD_SAVE);
    ACCOUNT_SEND_CMD(Recorder, RECORDER_CMD_STOP);
}
