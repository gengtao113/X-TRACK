#include "LiveMap.h"
#include "Common/DataProc/dataproc_c.h"
#include <stdbool.h>
#include "Config/Config.h"
#include <string.h>

static LiveMapPage s_livemap;
static uint16_t s_map_level_current = CONFIG_LIVE_MAP_LEVEL_DEFAULT;

static void LiveMap_OnEvent(lv_event_t* event);
static void LiveMap_Update(LiveMapPage* d);
static void LiveMap_AttachEvent(LiveMapPage* d, lv_obj_t* obj);
static void LiveMap_SportInfoUpdate(LiveMapPage* d);
static void LiveMap_CheckPosition(LiveMapPage* d);
static void LiveMap_UpdateDelay(LiveMapPage* d, uint32_t ms);
static bool LiveMap_GetIsMapTileContChanged(LiveMapPage* d);
static void LiveMap_OnMapTileContRefresh(LiveMapPage* d, const LiveMapArea_t* area, int32_t x, int32_t y);
static void LiveMap_MapTileContUpdate(LiveMapPage* d, int32_t mapX, int32_t mapY, float course);
static void LiveMap_MapTileContReload(LiveMapPage* d);
static void LiveMap_TrackLineReload(LiveMapPage* d, const LiveMapArea_t* area, int32_t x, int32_t y);
static void LiveMap_TrackLineAppend(LiveMapPage* d, int32_t x, int32_t y);
static void LiveMap_TrackLineAppendToEnd(LiveMapPage* d, int32_t x, int32_t y);

static void LiveMap_AttachEvent(LiveMapPage* d, lv_obj_t* obj)
{
    lv_obj_add_event_cb(obj, LiveMap_OnEvent, LV_EVENT_ALL, d);
}

static void LiveMap_OnTimer(lv_timer_t* timer)
{
    LiveMapPage* d = (LiveMapPage*)timer->user_data;
    LiveMap_Update(d);
}

static void LiveMap_OnPointFilterOut(void* user, int32_t x, int32_t y)
{
    LiveMap_TrackLineAppendToEnd((LiveMapPage*)user, x, y);
}

static void LiveMap_OnTrackReloadPoint(void* user, int32_t x, int32_t y)
{
    LiveMapPage* d = (LiveMapPage*)user;
    LiveMapModel_LineFilterPush(&d->model, x, y);
}

static void LiveMap_OnTrackLineEvent(void* user, const LiveMapLineEvent_t* event)
{
    LiveMapPage* d = (LiveMapPage*)user;
    lv_poly_line* lineTrack = d->view.ui.track.lineTrack;

    switch (event->code)
    {
    case LIVEMAP_LINE_EVENT_START_LINE:
        lv_poly_line_start(lineTrack);
        LiveMap_TrackLineAppend(d, event->x, event->y);
        break;
    case LIVEMAP_LINE_EVENT_APPEND_POINT:
        LiveMap_TrackLineAppend(d, event->x, event->y);
        break;
    case LIVEMAP_LINE_EVENT_END_LINE:
        if (event->has_point)
        {
            LiveMap_TrackLineAppend(d, event->x, event->y);
        }
        lv_poly_line_stop(lineTrack);
        break;
    case LIVEMAP_LINE_EVENT_RESET:
        lv_poly_line_reset(lineTrack);
        break;
    default:
        break;
    }
}

static void LiveMap_Update(LiveMapPage* d)
{
    if (lv_tick_elaps(d->run.lastMapUpdateTime) >= CONFIG_GPS_REFR_PERIOD)
    {
        LiveMap_CheckPosition(d);
        LiveMap_SportInfoUpdate(d);
        d->run.lastMapUpdateTime = lv_tick_get();
    }
    else if (lv_tick_elaps(d->run.lastContShowTime) >= 3000)
    {
        lv_obj_add_state(d->view.ui.zoom.cont, LV_STATE_USER_1);
    }
}

static void LiveMap_UpdateDelay(LiveMapPage* d, uint32_t ms)
{
    d->run.lastMapUpdateTime = lv_tick_get() - 1000 + ms;
}

static void LiveMap_SportInfoUpdate(LiveMapPage* d)
{
    char buf[16];

    lv_label_set_text_fmt(
        d->view.ui.sportInfo.labelSpeed,
        "%02d",
        (int)d->model.sportStatusInfo.speedKph
    );

    lv_label_set_text_fmt(
        d->view.ui.sportInfo.labelTrip,
        "%0.1f km",
        d->model.sportStatusInfo.singleDistance / 1000
    );

    lv_label_set_text(
        d->view.ui.sportInfo.labelTime,
        DataProc_MakeTimeString(d->model.sportStatusInfo.singleTime, buf, sizeof(buf))
    );
}

static bool LiveMap_GetIsMapTileContChanged(LiveMapPage* d)
{
    LiveMapPoint_t pos;
    bool ret;

    LiveMapModel_TileGetPos(&d->model, 0, &pos);
    ret = (pos.x != d->run.lastTileContOriPoint.x || pos.y != d->run.lastTileContOriPoint.y);
    d->run.lastTileContOriPoint = pos;
    return ret;
}

static void LiveMap_MapTileContReload(LiveMapPage* d)
{
    uint32_t i;
    for (i = 0; i < d->view.ui.map.tileNum; i++)
    {
        LiveMapPoint_t pos;
        char path[64];
        LiveMapModel_TileGetPos(&d->model, i, &pos);
        LiveMapModel_MapConvertPath(&d->model, pos.x, pos.y, path, sizeof(path));
        LiveMapView_SetMapTileSrc(&d->view, i, path);
    }
}

static void LiveMap_TrackLineAppend(LiveMapPage* d, int32_t x, int32_t y)
{
    LiveMapPoint_t offset;
    LiveMapPoint_t curPoint = { x, y };
    LiveMapModel_TileGetOffset(&d->model, &offset, &curPoint);
    lv_poly_line_append(d->view.ui.track.lineTrack, (lv_coord_t)offset.x, (lv_coord_t)offset.y);
}

static void LiveMap_TrackLineAppendToEnd(LiveMapPage* d, int32_t x, int32_t y)
{
    LiveMapPoint_t offset;
    LiveMapPoint_t curPoint = { x, y };
    LiveMapModel_TileGetOffset(&d->model, &offset, &curPoint);
    lv_poly_line_append_to_end(d->view.ui.track.lineTrack, (lv_coord_t)offset.x, (lv_coord_t)offset.y);
}

static void LiveMap_TrackLineReload(LiveMapPage* d, const LiveMapArea_t* area, int32_t x, int32_t y)
{
    LiveMapModel_LineFilterSetClipArea(&d->model, area);
    LiveMapModel_LineFilterReset(&d->model);
    LiveMapModel_TrackReload(&d->model, LiveMap_OnTrackReloadPoint, d);
    LiveMapModel_LineFilterPush(&d->model, x, y);
    LiveMapModel_LineFilterPushEnd(&d->model);
}

static void LiveMap_OnMapTileContRefresh(LiveMapPage* d, const LiveMapArea_t* area, int32_t x, int32_t y)
{
    LV_LOG_INFO(
        "area: (%d, %d) [%dx%d]",
        area->x0, area->y0,
        area->x1 - area->x0 + 1,
        area->y1 - area->y0 + 1
    );

    LiveMap_MapTileContReload(d);

    if (d->run.isTrackAvtive)
    {
        LiveMap_TrackLineReload(d, area, x, y);
    }
}

static void LiveMap_MapTileContUpdate(LiveMapPage* d, int32_t mapX, int32_t mapY, float course)
{
    LiveMapPoint_t offset;
    LiveMapPoint_t curPoint = { mapX, mapY };
    lv_obj_t* img;
    lv_coord_t x;
    lv_coord_t y;
    lv_coord_t baseX;
    lv_coord_t baseY;

    LiveMapModel_TileGetOffset(&d->model, &offset, &curPoint);

    img = d->view.ui.map.imgArrow;
    LiveMapModel_TileGetFocusOffset(&d->model, &offset);
    x = (lv_coord_t)(offset.x - lv_obj_get_width(img) / 2);
    y = (lv_coord_t)(offset.y - lv_obj_get_height(img) / 2);
    LiveMapView_SetImgArrowStatus(&d->view, x, y, course);

    if (d->run.isTrackAvtive)
    {
        LiveMapView_SetLineActivePoint(&d->view, (lv_coord_t)offset.x, (lv_coord_t)offset.y);
    }

    LiveMapModel_TileGetContainerOffset(&d->model, &offset);
    baseX = (LV_HOR_RES - CONFIG_LIVE_MAP_VIEW_WIDTH) / 2;
    baseY = (LV_VER_RES - CONFIG_LIVE_MAP_VIEW_HEIGHT) / 2;
    lv_obj_set_pos(d->view.ui.map.cont, baseX - (lv_coord_t)offset.x, baseY - (lv_coord_t)offset.y);
}

static void LiveMap_CheckPosition(LiveMapPage* d)
{
    bool refreshMap = false;
    GPS_Info_t gpsInfo;
    int32_t mapX;
    int32_t mapY;

    LiveMapModel_GetGPS_Info(&d->model, &gpsInfo);

    s_map_level_current = (uint16_t)lv_slider_get_value(d->view.ui.zoom.slider);
    if (s_map_level_current != (uint16_t)LiveMapModel_MapGetLevel(&d->model))
    {
        refreshMap = true;
        LiveMapModel_MapSetLevel(&d->model, (int16_t)s_map_level_current);
    }

    LiveMapModel_MapConvertCoord(&d->model, gpsInfo.longitude, gpsInfo.latitude, &mapX, &mapY);
    LiveMapModel_TileSetFocus(&d->model, mapX, mapY);

    if (LiveMap_GetIsMapTileContChanged(d))
    {
        refreshMap = true;
    }

    if (refreshMap)
    {
        LiveMapRect_t rect;
        LiveMapArea_t area;

        LiveMapModel_TileGetContainer(&d->model, &rect);
        area.x0 = rect.x;
        area.y0 = rect.y;
        area.x1 = rect.x + rect.width - 1;
        area.y1 = rect.y + rect.height - 1;
        LiveMap_OnMapTileContRefresh(d, &area, mapX, mapY);
    }

    LiveMap_MapTileContUpdate(d, mapX, mapY, gpsInfo.course);

    if (d->run.isTrackAvtive)
    {
        LiveMapModel_PointFilterPush(&d->model, mapX, mapY);
    }
}

static void LiveMap_OnEvent(lv_event_t* event)
{
    LiveMapPage* d = (LiveMapPage*)lv_event_get_user_data(event);
    lv_obj_t* obj;
    lv_event_code_t code;

    LV_ASSERT_NULL(d);

    obj = lv_event_get_current_target(event);
    code = lv_event_get_code(event);

    if (code == LV_EVENT_LEAVE)
    {
        page_pop(&d->base);
        return;
    }

    if (obj == d->view.ui.zoom.slider)
    {
        if (code == LV_EVENT_VALUE_CHANGED)
        {
            int32_t level = lv_slider_get_value(obj);
            int32_t levelMax = LiveMapModel_MapGetLevelMax();
            lv_label_set_text_fmt(d->view.ui.zoom.labelInfo, "%d/%d", (int)level, (int)levelMax);

            lv_obj_clear_state(d->view.ui.zoom.cont, LV_STATE_USER_1);
            d->run.lastContShowTime = lv_tick_get();
            LiveMap_UpdateDelay(d, 200);
        }
        else if (code == LV_EVENT_PRESSED)
        {
            page_pop(&d->base);
        }
    }

    if (obj == d->view.ui.sportInfo.cont)
    {
        if (code == LV_EVENT_PRESSED)
        {
            page_pop(&d->base);
        }
    }
}

static void on_custom_attr(PageBase* page)
{
    Page_SetCustomCacheEnable(page, false);
}

static void on_load(PageBase* page)
{
    LiveMapPage* d = (LiveMapPage*)page;
    const uint32_t tileSize = 256;
    LiveMapRect_t rect;
    uint32_t tileNum;

    LiveMapModel_TileSetSize(&d->model, tileSize);
    LiveMapModel_TileSetViewSize(&d->model, CONFIG_LIVE_MAP_VIEW_WIDTH, CONFIG_LIVE_MAP_VIEW_HEIGHT);
    LiveMapModel_TileSetFocus(&d->model, 0, 0);

    tileNum = LiveMapModel_TileGetContainer(&d->model, &rect);
    LiveMapView_Create(&d->view, page->_root, tileNum);
    lv_slider_set_range(
        d->view.ui.zoom.slider,
        LiveMapModel_MapGetLevelMin(),
        LiveMapModel_MapGetLevelMax()
    );
    LiveMapView_SetMapTile(&d->view, tileSize, (uint32_t)(rect.width / (int32_t)tileSize));

#if CONFIG_LIVE_MAP_DEBUG_ENABLE
    {
        lv_obj_t* contView = lv_obj_create(page->_root);
        lv_obj_center(contView);
        lv_obj_set_size(contView, CONFIG_LIVE_MAP_VIEW_WIDTH, CONFIG_LIVE_MAP_VIEW_HEIGHT);
        lv_obj_set_style_border_color(contView, lv_palette_main(LV_PALETTE_RED), 0);
        lv_obj_set_style_border_width(contView, 1, 0);
    }
#endif

    LiveMap_AttachEvent(d, page->_root);
    LiveMap_AttachEvent(d, d->view.ui.zoom.slider);
    LiveMap_AttachEvent(d, d->view.ui.sportInfo.cont);

    lv_slider_set_value(d->view.ui.zoom.slider, s_map_level_current, LV_ANIM_OFF);
    LiveMapModel_MapSetLevel(&d->model, (int16_t)s_map_level_current);
    lv_obj_add_flag(d->view.ui.map.cont, LV_OBJ_FLAG_HIDDEN);

    LiveMapModel_PointFilterSetThreshold(&d->model, CONFIG_TRACK_FILTER_OFFSET_THRESHOLD);
    LiveMapModel_PointFilterSetCallback(&d->model, LiveMap_OnPointFilterOut, d);
    LiveMapModel_LineFilterSetCallback(&d->model, LiveMap_OnTrackLineEvent, d);
}

static void on_did_load(PageBase* page)
{
    (void)page;
}

static void on_will_appear(PageBase* page)
{
    LiveMapPage* d = (LiveMapPage*)page;
    char theme[16];

    lv_obj_set_style_opa(page->_root, LV_OPA_COVER, LV_PART_MAIN);
    LiveMapModel_Init(&d->model);

    LiveMapModel_GetArrowTheme(&d->model, theme, sizeof(theme));
    LiveMapView_SetArrowTheme(&d->view, theme);

    d->run.isTrackAvtive = LiveMapModel_GetTrackFilterActive(&d->model);
    LiveMapModel_SetStatusBarStyle(&d->model, STATUS_BAR_STYLE_BLACK);
    LiveMap_SportInfoUpdate(d);
    lv_obj_clear_flag(d->view.ui.labelInfo, LV_OBJ_FLAG_HIDDEN);
}

static void on_did_appear(PageBase* page)
{
    LiveMapPage* d = (LiveMapPage*)page;
    lv_group_t* group;

    d->run.timer = lv_timer_create(LiveMap_OnTimer, 100, d);
    d->run.lastMapUpdateTime = 0;
    lv_obj_clear_flag(d->view.ui.map.cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(d->view.ui.labelInfo, LV_OBJ_FLAG_HIDDEN);

    d->run.lastTileContOriPoint.x = 0;
    d->run.lastTileContOriPoint.y = 0;

    d->run.isTrackAvtive = LiveMapModel_GetTrackFilterActive(&d->model);
    if (!d->run.isTrackAvtive)
    {
        LiveMapModel_PointFilterClearCallback(&d->model);
    }

    group = lv_group_get_default();
    lv_group_add_obj(group, d->view.ui.zoom.slider);
    lv_group_set_editing(group, true);
}

static void on_will_disappear(PageBase* page)
{
    LiveMapPage* d = (LiveMapPage*)page;
    lv_timer_del(d->run.timer);
    lv_obj_add_flag(d->view.ui.map.cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_fade_out(page->_root, 250, 250);
}

static void on_did_disappear(PageBase* page)
{
    LiveMapPage* d = (LiveMapPage*)page;
    LiveMapModel_Deinit(&d->model);
}

static void on_unload(PageBase* page)
{
    LiveMapPage* d = (LiveMapPage*)page;
    LiveMapView_Delete(&d->view);
}

static void on_did_unload(PageBase* page)
{
    (void)page;
}

static const PageOps s_livemap_ops = {
    .on_custom_attr    = on_custom_attr,
    .on_load           = on_load,
    .on_did_load       = on_did_load,
    .on_will_appear    = on_will_appear,
    .on_did_appear     = on_did_appear,
    .on_will_disappear = on_will_disappear,
    .on_did_disappear  = on_did_disappear,
    .on_unload         = on_unload,
    .on_did_unload     = on_did_unload,
    .destroy           = NULL,  /* ??????? delete */
};

PageBase* LiveMap_Create(void)
{
    LiveMapModel_Destruct(&s_livemap.model);
    memset(&s_livemap, 0, sizeof(s_livemap));
    LiveMapModel_Construct(&s_livemap.model);
    s_livemap.base.ops = &s_livemap_ops;
    return &s_livemap.base;
}
