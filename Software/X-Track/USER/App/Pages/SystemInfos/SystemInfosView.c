#include "SystemInfosView.h"
#include "Resource/ResourcePool.h"

#define ITEM_HEIGHT_MIN   100
#define ITEM_PAD          ((LV_VER_RES - ITEM_HEIGHT_MIN) / 2)

static void SystemInfosView_Group_Init(SystemInfosView* view);
static void SystemInfosView_Style_Init(SystemInfosView* view);
static void SystemInfosView_Style_Reset(SystemInfosView* view);
static void SystemInfosView_Item_Create(
    SystemInfosView* view,
    SystemInfosItem_t* item,
    lv_obj_t* par,
    const char* name,
    const char* img_src,
    const char* infos
);

void SystemInfosView_Create(SystemInfosView* view, lv_obj_t* root)
{
    lv_obj_set_style_pad_ver(root, ITEM_PAD, 0);

    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        root,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER
    );

    SystemInfosView_Style_Init(view);

    SystemInfosView_Item_Create(
        view, &view->ui.sport, root, "Sport", "bicycle",
        "Total trip\n"
        "Total time\n"
        "Max speed"
    );

    SystemInfosView_Item_Create(
        view, &view->ui.gps, root, "GPS", "map_location",
        "Latitude\n"
        "Longitude\n"
        "Altitude\n"
        "UTC Time\n\n"
        "Course\n"
        "Speed"
    );

    SystemInfosView_Item_Create(
        view, &view->ui.mag, root, "MAG", "compass",
        "Compass\n"
        "X\n"
        "Y\n"
        "Z"
    );

    SystemInfosView_Item_Create(
        view, &view->ui.imu, root, "IMU", "gyroscope",
        "Step\n"
        "Ax\n"
        "Ay\n"
        "Az\n"
        "Gx\n"
        "Gy\n"
        "Gz"
    );

    SystemInfosView_Item_Create(
        view, &view->ui.rtc, root, "RTC", "time_info",
        "Date\n"
        "Time"
    );

    SystemInfosView_Item_Create(
        view, &view->ui.battery, root, "Battery", "battery_info",
        "Usage\n"
        "Voltage\n"
        "Status"
    );

    SystemInfosView_Item_Create(
        view, &view->ui.storage, root, "Storage", "storage",
        "Status\n"
        "Size\n"
        "Type\n"
        "Version"
    );

    SystemInfosView_Item_Create(
        view, &view->ui.system, root, "System", "system_info",
        "Firmware\n"
        "Author\n"
        "LVGL\n"
        "SysTick\n"
        "Compiler\n\n"
        "Build\n"
    );

    SystemInfosView_Group_Init(view);
}

static void SystemInfosView_Group_Init(SystemInfosView* view)
{
    lv_group_t* group = lv_group_get_default();
    SystemInfosItem_t* item_grp = (SystemInfosItem_t*)&view->ui;
    int i;

    lv_group_set_wrap(group, true);
    lv_group_set_focus_cb(group, SystemInfosView_OnFocus);

    for (i = (int)(sizeof(view->ui) / sizeof(SystemInfosItem_t)) - 1; i >= 0; i--)
    {
        lv_group_add_obj(group, item_grp[i].icon);
    }

    lv_group_focus_obj(item_grp[0].icon);
}

void SystemInfosView_Delete(SystemInfosView* view)
{
    lv_group_set_focus_cb(lv_group_get_default(), NULL);
    SystemInfosView_Style_Reset(view);
}

void SystemInfosView_SetScrollToY(lv_obj_t* obj, lv_coord_t y, lv_anim_enable_t en)
{
    lv_coord_t scroll_y = lv_obj_get_scroll_y(obj);
    lv_coord_t diff = -y + scroll_y;

    lv_obj_scroll_by(obj, 0, diff, en);
}

void SystemInfosView_OnFocus(lv_group_t* g)
{
    lv_obj_t* icon = lv_group_get_focused(g);
    lv_obj_t* cont = lv_obj_get_parent(icon);
    lv_coord_t y = lv_obj_get_y(cont);
    lv_obj_scroll_to_y(lv_obj_get_parent(cont), y, LV_ANIM_ON);
}

static void SystemInfosView_Style_Init(SystemInfosView* view)
{
    static const lv_style_prop_t style_prop[] =
    {
        LV_STYLE_WIDTH,
        LV_STYLE_PROP_INV
    };
    static lv_style_transition_dsc_t trans;

    lv_style_init(&view->style.icon);
    lv_style_set_width(&view->style.icon, 220);
    lv_style_set_bg_color(&view->style.icon, lv_color_black());
    lv_style_set_bg_opa(&view->style.icon, LV_OPA_COVER);
    lv_style_set_text_font(&view->style.icon, ResourcePool_GetFont("bahnschrift_17"));
    lv_style_set_text_color(&view->style.icon, lv_color_white());

    lv_style_init(&view->style.focus);
    lv_style_set_width(&view->style.focus, 70);
    lv_style_set_border_side(&view->style.focus, LV_BORDER_SIDE_RIGHT);
    lv_style_set_border_width(&view->style.focus, 2);
    lv_style_set_border_color(&view->style.focus, lv_color_hex(0xff931e));

    lv_style_transition_dsc_init(
        &trans,
        style_prop,
        lv_anim_path_overshoot,
        200,
        0,
        NULL
    );
    lv_style_set_transition(&view->style.focus, &trans);
    lv_style_set_transition(&view->style.icon, &trans);

    lv_style_init(&view->style.info);
    lv_style_set_text_font(&view->style.info, ResourcePool_GetFont("bahnschrift_13"));
    lv_style_set_text_color(&view->style.info, lv_color_hex(0x999999));

    lv_style_init(&view->style.data);
    lv_style_set_text_font(&view->style.data, ResourcePool_GetFont("bahnschrift_13"));
    lv_style_set_text_color(&view->style.data, lv_color_white());
}

static void SystemInfosView_Style_Reset(SystemInfosView* view)
{
    lv_style_reset(&view->style.icon);
    lv_style_reset(&view->style.info);
    lv_style_reset(&view->style.data);
    lv_style_reset(&view->style.focus);
}

static void SystemInfosView_Item_Create(
    SystemInfosView* view,
    SystemInfosItem_t* item,
    lv_obj_t* par,
    const char* name,
    const char* img_src,
    const char* infos
)
{
    lv_obj_t* cont;
    lv_obj_t* icon;
    lv_obj_t* img;
    lv_obj_t* label;
    lv_coord_t height;

    cont = lv_obj_create(par);
    lv_obj_enable_style_refresh(false);
    lv_obj_remove_style_all(cont);
    lv_obj_set_width(cont, 220);

    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    item->cont = cont;

    icon = lv_obj_create(cont);
    lv_obj_enable_style_refresh(false);
    lv_obj_remove_style_all(icon);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_style(icon, &view->style.icon, 0);
    lv_obj_add_style(icon, &view->style.focus, LV_STATE_FOCUSED);
    lv_obj_set_style_align(icon, LV_ALIGN_LEFT_MID, 0);

    lv_obj_set_flex_flow(icon, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        icon,
        LV_FLEX_ALIGN_SPACE_AROUND,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    img = lv_img_create(icon);
    lv_obj_enable_style_refresh(false);
    lv_img_set_src(img, ResourcePool_GetImage(img_src));

    label = lv_label_create(icon);
    lv_obj_enable_style_refresh(false);
    lv_label_set_text(label, name);
    item->icon = icon;

    label = lv_label_create(cont);
    lv_obj_enable_style_refresh(false);
    lv_label_set_text(label, infos);
    lv_obj_add_style(label, &view->style.info, 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 75, 0);
    item->labelInfo = label;

    label = lv_label_create(cont);
    lv_obj_enable_style_refresh(false);
    lv_label_set_text(label, "-");
    lv_obj_add_style(label, &view->style.data, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 60, 0);
    item->labelData = label;

    lv_obj_move_foreground(icon);
    lv_obj_enable_style_refresh(true);

    lv_obj_update_layout(item->labelInfo);
    height = lv_obj_get_height(item->labelInfo);
    height = LV_MAX(height, ITEM_HEIGHT_MIN);
    lv_obj_set_height(cont, height);
    lv_obj_set_height(icon, height);
}

void SystemInfosView_SetSport(SystemInfosView* view, float trip, const char* time, float maxSpd)
{
    lv_label_set_text_fmt(
        view->ui.sport.labelData,
        "%0.2fkm\n"
        "%s\n"
        "%0.1fkm/h",
        trip,
        time,
        maxSpd
    );
}

void SystemInfosView_SetGPS(SystemInfosView* view, float lat, float lng, float alt, const char* utc, float course, float speed)
{
    lv_label_set_text_fmt(
        view->ui.gps.labelData,
        "%0.6f\n"
        "%0.6f\n"
        "%0.2fm\n"
        "%s\n"
        "%0.1f deg\n"
        "%0.1fkm/h",
        lat,
        lng,
        alt,
        utc,
        course,
        speed
    );
}

void SystemInfosView_SetMAG(SystemInfosView* view, float dir, int x, int y, int z)
{
    lv_label_set_text_fmt(
        view->ui.mag.labelData,
        "%0.1f deg\n"
        "%d\n"
        "%d\n"
        "%d",
        dir,
        x,
        y,
        z
    );
}

void SystemInfosView_SetIMU(SystemInfosView* view, int step, const char* info)
{
    lv_label_set_text_fmt(
        view->ui.imu.labelData,
        "%d\n"
        "%s",
        step,
        info
    );
}

void SystemInfosView_SetRTC(SystemInfosView* view, const char* dateTime)
{
    lv_label_set_text(view->ui.rtc.labelData, dateTime);
}

void SystemInfosView_SetBattery(SystemInfosView* view, int usage, float voltage, const char* state)
{
    lv_label_set_text_fmt(
        view->ui.battery.labelData,
        "%d%%\n"
        "%0.2fV\n"
        "%s",
        usage,
        voltage,
        state
    );
}

void SystemInfosView_SetStorage(SystemInfosView* view, const char* detect, const char* size, const char* type, const char* version)
{
    lv_label_set_text_fmt(
        view->ui.storage.labelData,
        "%s\n"
        "%s\n"
        "%s\n"
        "%s",
        detect,
        size,
        type,
        version
    );
}

void SystemInfosView_SetSystem(
    SystemInfosView* view,
    const char* firmVer,
    const char* authorName,
    const char* lvglVer,
    const char* bootTime,
    const char* compilerName,
    const char* bulidTime
)
{
    lv_label_set_text_fmt(
        view->ui.system.labelData,
        "%s\n"
        "%s\n"
        "%s\n"
        "%s\n"
        "%s\n"
        "%s",
        firmVer,
        authorName,
        lvglVer,
        bootTime,
        compilerName,
        bulidTime
    );
}
