#include "LiveMap.h"
#include "Config.h"

using namespace Page;

uint16_t LiveMap::mapLevelCurrent = CONFIG_LIVE_MAP_LEVEL_DEFAULT;

LiveMap::LiveMap()
{
    memset(&priv, 0, sizeof(priv));
}

LiveMap::~LiveMap()
{

}

void LiveMap::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
}

void LiveMap::onViewLoad()
{
    const uint32_t tileSize = 256;
    //设置Tilesize 与 viewsize
    Model.tileConv.SetTileSize(tileSize);
    Model.tileConv.SetViewSize(
        CONFIG_LIVE_MAP_VIEW_WIDTH,
        CONFIG_LIVE_MAP_VIEW_HEIGHT
    );
    //一开始设置焦点为0，0，顺便更新pointView与pointTileCont
    Model.tileConv.SetFocusPos(0, 0);

    TileConv::Rect_t rect;
    //获取pointTileCont的tilenum 和  将pointTileCont转换为Rect_t
    uint32_t tileNum = Model.tileConv.GetTileContainer(&rect);

    //create view
    View.Create(_root, tileNum);
    //set slider range
    lv_slider_set_range(
        View.ui.zoom.slider,
        Model.mapConv.GetLevelMin(),
        Model.mapConv.GetLevelMax()
    );
    //设置ui.map.cont的size 给各个ui.map.imgTiles 设置size与pos
    View.SetMapTile(tileSize, rect.width / tileSize);

#if CONFIG_LIVE_MAP_DEBUG_ENABLE
    lv_obj_t* contView = lv_obj_create(root);
    lv_obj_center(contView);
    lv_obj_set_size(contView, CONFIG_LIVE_MAP_VIEW_WIDTH, CONFIG_LIVE_MAP_VIEW_HEIGHT);
    lv_obj_set_style_border_color(contView, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_border_width(contView, 1, 0);
#endif
    //为obj绑定事件
    AttachEvent(_root);
    AttachEvent(View.ui.zoom.slider);
    AttachEvent(View.ui.sportInfo.cont);

    //设置slider的value
    lv_slider_set_value(View.ui.zoom.slider, mapLevelCurrent, LV_ANIM_OFF);
    //set map level
    Model.mapConv.SetLevel(mapLevelCurrent);
    //ui.map.cont set hidden flag
    lv_obj_add_flag(View.ui.map.cont, LV_OBJ_FLAG_HIDDEN);

    /* Point filter */
    //将datapoint 推入Point filter中，过滤掉过于密集的点,有效点会发送到callback处理
    Model.pointFilter.SetOffsetThreshold(CONFIG_TRACK_FILTER_OFFSET_THRESHOLD);
    Model.pointFilter.SetOutputPointCallback([](TrackPointFilter * filter, const TrackPointFilter::Point_t* point)
    {
        LiveMap* instance = (LiveMap*)filter->userData;
        //向poly_line中最后一条(index = current_index - 1 )single_line添加点,并且绘制该single_line
        instance->TrackLineAppendToEnd((int32_t)point->x, (int32_t)point->y);
    });
    Model.pointFilter.userData = this;

    /* Line filter */
    //将datapoint 推入Line filter中，在clipArea内的点会被发送到onTrackLineEvent处理
    Model.lineFilter.SetOutputPointCallback(onTrackLineEvent);
    Model.lineFilter.userData = this;
}

void LiveMap::onViewDidLoad()
{

}

void LiveMap::onViewWillAppear()
{
    //set root opa
    lv_obj_set_style_opa(_root, LV_OPA_COVER, LV_PART_MAIN);

    Model.Init();
    //设置arrow state
    char theme[16];
    Model.GetArrowTheme(theme, sizeof(theme));
    View.SetArrowTheme(theme);
    //get TrackFilter服务的状态
    priv.isTrackActive = Model.GetTrackFilterActive();
    //设置StatusBar的style
    Model.SetStatusBarStyle(DataProc::STATUS_BAR_STYLE_BLACK);
    //update SportInfo ui
    SportInfoUpdate();
    //ui.labelInfo clear hidden flag 显示Loading提示
    lv_obj_clear_flag(View.ui.labelInfo, LV_OBJ_FLAG_HIDDEN);
}

void LiveMap::onViewDidAppear()
{
    //create timer 定时更新数据
    priv.timer = lv_timer_create([](lv_timer_t* timer)
    {
        LiveMap* instance = (LiveMap*)timer->user_data;
        instance->Update();
    },
    100,
    this);

    //Init

    priv.lastMapUpdateTime = 0;
    //ui.map.cont clear hidden flag 显示map cont
    lv_obj_clear_flag(View.ui.map.cont, LV_OBJ_FLAG_HIDDEN);
    //ui.labelInfo set hidden flag 隐藏Loading提示
    lv_obj_add_flag(View.ui.labelInfo, LV_OBJ_FLAG_HIDDEN);

    //pointTileCont last startPoint
    priv.lastTileContOriPoint.x = 0;
    priv.lastTileContOriPoint.y = 0;

    //get TrackFilter服务的状态
    priv.isTrackActive = Model.GetTrackFilterActive();
    if (!priv.isTrackActive)
    {
        //如果TrackFilter服务没开始录制，将pointFilter的callback设置为nullptr
        Model.pointFilter.SetOutputPointCallback(nullptr);
    }

    lv_group_t* group = lv_group_get_default();
    lv_group_add_obj(group, View.ui.zoom.slider);//添加slider到group
    lv_group_set_editing(group, View.ui.zoom.slider);//设置为正在编辑的对象

    Model.SetKeySliderMode(true);
}


void LiveMap::onViewWillDisappear()
{
    lv_timer_del(priv.timer);
    //ui.map.cont set hidden flag 隐藏map cont
    lv_obj_add_flag(View.ui.map.cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_fade_out(_root, 250, 250);

    lv_group_t* group = lv_group_get_default();
    LV_ASSERT_NULL(group);
    lv_group_remove_all_objs(group);//remove all object from group
}

void LiveMap::onViewDidDisappear()
{
    Model.SetKeySliderMode(false);
    Model.Deinit();
}

void LiveMap::onViewUnload()
{
    View.Delete();
}

void LiveMap::onViewDidUnload()
{

}

void LiveMap::AttachEvent(lv_obj_t* obj)
{
    lv_obj_add_event_cb(obj, onEvent, LV_EVENT_ALL, this);//传递this指针
}

void LiveMap::Update()
{
    if (lv_tick_elaps(priv.lastMapUpdateTime) >= CONFIG_GPS_REFR_PERIOD)
    {
        //更新地图与Track
        CheckPosition();
        //更新SportInfo ui
        SportInfoUpdate();
        priv.lastMapUpdateTime = lv_tick_get();
    }
    //ui.zoom.cont 显示时间为3s，超过后隐藏
    else if (lv_tick_elaps(priv.lastContShowTime) >= 3000)
    {
        lv_obj_add_state(View.ui.zoom.cont, LV_STATE_USER_1);
    }
}

void LiveMap::UpdateDelay(uint32_t ms)
{
    priv.lastMapUpdateTime = lv_tick_get() - 1000 + ms; //-1000ms 是需要延迟1000ms
}

/**
 * @brief 更新SportInfo ui
 * 
 */
void LiveMap::SportInfoUpdate()
{
    lv_label_set_text_fmt(
        View.ui.sportInfo.labelSpeed,
        "%02d",
        (int)Model.sportStatusInfo.speedKph
    );

    lv_label_set_text_fmt(
        View.ui.sportInfo.labelTrip,
        "%0.1f km",
        Model.sportStatusInfo.singleDistance / 1000
    );

    char buf[16];
    lv_label_set_text(
        View.ui.sportInfo.labelTime,
        DataProc::MakeTimeString(Model.sportStatusInfo.singleTime, buf, sizeof(buf))
    );
}
/**
 * @brief  更新地图与Track
 */
void LiveMap::CheckPosition()
{
    //检查pointTileCont与 mapLevel 的改变，有改变就需要更新MAP
    bool refreshMap = false;

    //获取gps信息
    HAL::GPS_Info_t gpsInfo;
    Model.GetGPS_Info(&gpsInfo);
    //get slider level
    mapLevelCurrent = lv_slider_get_value(View.ui.zoom.slider);
    //maplevel changed
    if (mapLevelCurrent != Model.mapConv.GetLevel())
    {
        refreshMap = true; //set refresh Map flag
        Model.mapConv.SetLevel(mapLevelCurrent);
    }

    int32_t mapX, mapY;
    //经纬度转为地图XY坐标
    Model.mapConv.ConvertMapCoordinate(
        gpsInfo.longitude, gpsInfo.latitude,
        &mapX, &mapY
    );
    //设置焦点位置pointFocus,顺便更新pointView与pointTileCont
    Model.tileConv.SetFocusPos(mapX, mapY);

    //pointTileCont changed
    if (GetIsMapTileContChanged())
    {
        refreshMap = true;//set refresh Map flag
    }

    if (refreshMap)
    {
        TileConv::Rect_t rect;
        //将pointTileCont转换为Rect_t
        Model.tileConv.GetTileContainer(&rect);

        Area_t area =
        {
            .x0 = rect.x,
            .y0 = rect.y,
            .x1 = rect.x + rect.width - 1,
            .y1 = rect.y + rect.height - 1
        };
        //重新加载合适的地图与描绘在area上的的TrackLine
        onMapTileContRefresh(&area, mapX, mapY);

        //Reset TrackPointFilter for pointFilter  作用不大
        Model.pointFilter.Reset();

    }
    //设置arrow的位置与角度; 绘制LineActive; 设置了ui.map.cont的pos
    MapTileContUpdate(mapX, mapY, gpsInfo.course);

    if (priv.isTrackActive)
    {
        //Push to TrackPointFilter 使用callback处理
        //向poly_line中最后一条(index = current_index - 1 )single_line添加点,并且绘制该single_line
        //覆盖LineActive,描绘真正的TrackLine
        Model.pointFilter.PushPoint(mapX, mapY);
    }
}
/**
 * @brief 重新加载合适的地图;对已记录的TrackLine's DataPoint 根据level进行坐标变换，最后也记录new data Point
 * @param area  TrackLineFilter's ClipArea
 * @param x new datapoint x
 * @param y new datapoint y
 */
void LiveMap::onMapTileContRefresh(const Area_t* area, int32_t x, int32_t y)
{
    LV_LOG_USER(
        "area: (%d, %d) [%dx%d]",
        area->x0, area->y0,
        area->x1 - area->x0 + 1,
        area->y1 - area->y0 + 1
    );
    //重新加载tile地图
    MapTileContReload();

    //TraceFilter is active
    if (priv.isTrackActive)
    {
        //重新描绘TrackFilter服务中已记录的坐标点 (只描绘在area中的Track)，并描绘新的坐标点(x,y)到最后的line中
        //这里area就是pointTileCont  datapoint 是最新的坐标 (Tile Map坐标系)
        TrackLineReload(area, x, y);
    }
}
/**
 * @brief 设置arrow的位置与角度; 绘制LineActive; 设置了ui.map.cont的pos
 * @param mapX
 * @param mapY
 * @param course
 */
void LiveMap::MapTileContUpdate(int32_t mapX, int32_t mapY, float course)
{
    TileConv::Point_t offset;
    TileConv::Point_t curPoint = { mapX, mapY };
    //获取map xy 相对于pointTileCont起点的偏移    与下面的获取offset一样
//    Model.tileConv.GetOffset(&offset, &curPoint);

    /* arrow */
    lv_obj_t* img = View.ui.map.imgArrow;
    //获取pointFocus相对于pointTileCont起点的偏移
    Model.tileConv.GetFocusOffset(&offset);
    //以arrow的中心为基准 这样更加符合直觉
    lv_coord_t x = offset.x - lv_obj_get_width(img) / 2;
    lv_coord_t y = offset.y - lv_obj_get_height(img) / 2;
    //设置arrow的pos与角度
    View.SetImgArrowStatus(x, y, course);

    /* active line */
    if (priv.isTrackActive)
    {
        //根据offset 描绘LineActive
        //一条从poly_line的最后一条(current_index - 1 )single_line的最后一个point 到 (offset.x,offset.y)的线
        //与CheckPosition中的Model.pointFilter.PushPoint(mapX, mapY)对应
        View.SetLineActivePoint((lv_coord_t)offset.x, (lv_coord_t)offset.y);
    }

    /* map cont */
    //获取pointView起点相对于pointTileCont起点的偏移
    Model.tileConv.GetTileContainerOffset(&offset);

    //LIVE_MAP_VIEW居中摆放在pointView
    lv_coord_t baseX = (LV_HOR_RES - CONFIG_LIVE_MAP_VIEW_WIDTH) / 2;
    lv_coord_t baseY = (LV_VER_RES - CONFIG_LIVE_MAP_VIEW_HEIGHT) / 2;
    //设置ui.map.cont的pos  lv_obj_set_pos使用的是屏幕的坐标系,不是TileMap的坐标系(pointTileCont、PointView就是TileMap的坐标系)
    lv_obj_set_pos(View.ui.map.cont, - offset.x + baseX, - offset.y + baseY);
}
/**
 * @brief 重新加载tile地图
 * obj= ui.map.imgTiles
 * size = pointTileCont size
 */
void LiveMap::MapTileContReload()
{
    /* tile src */
    for (uint32_t i = 0; i < View.ui.map.tileNum; i++)
    {
        TileConv::Point_t pos;
        //获取index=i对应的tile的起始坐标
        Model.tileConv.GetTilePos(i, &pos);

        char path[64];
        //根据XY坐标得到tile图片路径
        Model.mapConv.ConvertMapPath(pos.x, pos.y, path, sizeof(path));

        //设置ui.map.imgTiles 的img src
        View.SetMapTileSrc(i, path);
    }
}

/**
 * @brief 获取pointTileCont是否发生了变化
 * @return true 发生了变化
 */
bool LiveMap::GetIsMapTileContChanged()
{
    TileConv::Point_t pos;
    //获取index=0对应的tile的起始坐标，即pointTileCont的起始坐标
    Model.tileConv.GetTilePos(0, &pos);

    bool ret = (pos.x != priv.lastTileContOriPoint.x || pos.y != priv.lastTileContOriPoint.y);

    priv.lastTileContOriPoint = pos;

    return ret;
}

/**
 * @brief 对将TrackFilter服务中已经保存的点，更改level后的新坐标点 push 到一个TrackPointFilter后（这里主要是过滤掉太密集的点）再push到一个TrackPointFilter,其对应的callback会进行描绘
 *        同时也push new focusPoint to TrackPointFilter,其对应的callback会进行描绘，手动发送EVENT_END_LINE事件   结束当前single_line的绘制
 * @param area TrackLineFilter必须设置的有效范围
 * @param x new focusPoint x
 * @param y new focusPoint y
 */
void LiveMap::TrackLineReload(const Area_t* area, int32_t x, int32_t y)
{
    //set new clipArea
    Model.lineFilter.SetClipArea(area);
    //reset TrackLineFilter
    //并且current_index清0，poly_line的所有single_line的points清空，但是poly_line.size不变
    Model.lineFilter.Reset();

    //对将TrackFilter服务中已经保存的点，更改level后的新坐标点 push 到一个TrackPointFilter后（这里主要是过滤掉太密集的点）再push到一个TrackPointFilter,其对应的callback会进行描绘
    Model.TrackReload([](TrackPointFilter * filter, const TrackPointFilter::Point_t* point)
    {
        LiveMap* instance = (LiveMap*)filter->userData;
        //将datapoint 推入过滤器中，在clipArea内的点(也就是说只处理pointTileCont内的点) 会被发送到callback (onTrackLineEvent)处理
        instance->Model.lineFilter.PushPoint((int32_t)point->x, (int32_t)point->y);
    }, this);

    //将最新的datapoint 推入过滤器中，在clipArea内的点会被发送到callback处理
    Model.lineFilter.PushPoint(x, y);
    //手动发送EVENT_END_LINE事件   结束当前single_line的绘制，current_index++
    Model.lineFilter.PushEnd();
}
/**
 * @brief 向poly_line中当前的single_line添加点
 *        传递进来的point的都是Tile Map的坐标系上的点
 * @param x
 * @param y
 */
void LiveMap::TrackLineAppend(int32_t x, int32_t y)
{
    TileConv::Point_t offset;
    TileConv::Point_t curPoint = { x, y };
    //获取curPoint相对于pointTileCont起点的偏移
    Model.tileConv.GetOffset(&offset, &curPoint);
    //ui.map.cont -> ui.track.cont -> ui.track.lineTrack, 所以起点应该是pointTileCont起点
    //向当前的single_line添加点
    View.ui.track.lineTrack->append((lv_coord_t)offset.x, (lv_coord_t)offset.y);
}


/**
 * @brief 向poly_line中最后一条(index = current_index - 1 )single_line添加点,并且绘制该single_line
 *        传递进来的point的都是Tile Map的坐标系上的点
 * @param x
 * @param y
 */
void LiveMap::TrackLineAppendToEnd(int32_t x, int32_t y)
{
    TileConv::Point_t offset;
    TileConv::Point_t curPoint = { x, y };
    //获取curPoint到pointTileCont起点的偏移offset
    Model.tileConv.GetOffset(&offset, &curPoint);
    //ui.map.cont -> ui.track.cont -> ui.track.lineTrack,因此记录的是在pointTileCont上的Track Point
    //添加到最后一条(index = current_index - 1 )single_line,并且绘制该single_line
    View.ui.track.lineTrack->append_to_end((lv_coord_t)offset.x, (lv_coord_t)offset.y);
}
/**
 * @brief TrackLineFilter的回调函数  对poly_line进行操作
 * @param filter
 * @param event
 */
void LiveMap::onTrackLineEvent(TrackLineFilter* filter, TrackLineFilter::Event_t* event)
{
    LiveMap* instance = (LiveMap*)filter->userData;
    lv_poly_line* lineTrack = instance->View.ui.track.lineTrack;

    switch (event->code)
    {
    case TrackLineFilter::EVENT_START_LINE:
        //开始收集point绘制new single_line
        lineTrack->start();
        //向poly_line中当前的single_line添加点
        instance->TrackLineAppend(event->point->x, event->point->y);
        break;
    case TrackLineFilter::EVENT_APPEND_POINT:
        //向poly_line中当前的single_line添加点
        instance->TrackLineAppend(event->point->x, event->point->y);
        break;

    case TrackLineFilter::EVENT_END_LINE:
        if (event->point != nullptr)
        {
            //向poly_line中当前的single_line添加点
            instance->TrackLineAppend(event->point->x, event->point->y);
        }
        //结束当前single_line的绘制  current_index++
        lineTrack->stop();
        break;
    case TrackLineFilter::EVENT_RESET:
        //current_index清0，poly_line的所有single_line的points清空，但是poly_line.size不变
        lineTrack->reset();
        break;
    default:
        break;
    }
}

void LiveMap::onEvent(lv_event_t* event)
{
    LiveMap* instance = (LiveMap*)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t* obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (code == LV_EVENT_LEAVE)
    {
        instance->_Manager->Pop();
        return;
    }
    //对于slider对象
    if (obj == instance->View.ui.zoom.slider)
    {
        //当slider的值发生变化时 显示zoom obj
        if (code == LV_EVENT_VALUE_CHANGED)
        {
            //get curLevel and maxLevel
            int32_t level = lv_slider_get_value(obj);
            int32_t levelMax = instance->Model.mapConv.GetLevelMax();
            lv_label_set_text_fmt(instance->View.ui.zoom.labelInfo, "%d/%d", level, levelMax);

            //ui.zoom.cont clear LV_STATE_USER_1 state
            //显示zoom obj
            lv_obj_clear_state(instance->View.ui.zoom.cont, LV_STATE_USER_1);
            //update lastContShowTime
            instance->priv.lastContShowTime = lv_tick_get();
            //update lastMapUpdateTime  延迟1000ms -200ms的事件才会继续调用update() 可以理解为这是一段稳定期
            instance->UpdateDelay(200);
        }
        else if (code == LV_EVENT_PRESSED)
        {
            instance->_Manager->Pop();
        }
    }

    if (obj == instance->View.ui.sportInfo.cont)
    {

        if (code == LV_EVENT_PRESSED)
        {
            instance->_Manager->Pop();
        }
    }
}
