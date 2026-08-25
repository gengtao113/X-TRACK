#ifndef __DIALPLATE_VIEW_H
#define __DIALPLATE_VIEW_H

#include "../Page.h"

/* namespace = 模块名前缀，可忽略。调用写作 Page::DialplateView。 */
namespace Page
{

/* View = 一堆 LVGL 控件句柄 + Create/Delete。
 * 只允许 lv_obj_create / 设样式 / 动画，禁止读 GPS、算速度。
 * class 对 C 就是 struct；View.Create(root) ≈ DialplateView_Create(&view, root)。 */
class DialplateView
{

public:
    /* 底部一组小信息：容器 + 数值 Label + 单位 Label。
     * 下面 labelInfoGrp[4] 用这个类型，对应 AVG / Time / Trip / Calorie。 */
    typedef struct
    {
        lv_obj_t* cont;
        lv_obj_t* lableValue;
        lv_obj_t* lableUnit;
    } SubInfo_t;

public:
    /* 匿名 struct 变量 ui：所有控件指针集中放这里。
     * Presenter 改速度文字：View.ui.topInfo.labelSpeed
     * C 里就是 DialplateView 结构体里嵌套的几个子结构体。 */
    struct
    {
        /* 上半：大号时速 */
        struct
        {
            lv_obj_t* cont;
            lv_obj_t* labelSpeed;
            lv_obj_t* labelUint;   /* 单位 km/h，拼写沿用源码 Uint */
        } topInfo;

        /* 下半：四组副信息，横排 */
        struct
        {
            lv_obj_t* cont;
            SubInfo_t labelInfoGrp[4];
        } bottomInfo;

        /* 底栏三个按钮：地图 / 录轨 / 菜单 */
        struct
        {
            lv_obj_t* cont;
            lv_obj_t* btnMap;
            lv_obj_t* btnRec;
            lv_obj_t* btnMenu;
        } btnCont;

        /* 入场动画时间线。控件本身挂在页面 root 上，Delete 时主要删这个。 */
        lv_anim_timeline_t* anim_timeline;
    } ui;

    void Create(lv_obj_t* root);  /* 在 root 下创建全部控件，C：xxx_create(v, root) */
    void Delete();                /* 释放动画时间线；root 由 PageManager 销毁 */

    /* reverse = false 是默认参数：C++ 里可不写第二实参。
     * AppearAnimStart()     = 正向入场
     * AppearAnimStart(true) = 反向（离场，本页里基本没用） */
    void AppearAnimStart(bool reverse = false);

private:  /* 下面只给 View 自己的 .cpp 用，Presenter 不要直接调 */

    void TopInfo_Create(lv_obj_t* par);
    void BottomInfo_Create(lv_obj_t* par);
    void SubInfoGrp_Create(lv_obj_t* par, SubInfo_t* info, const char* unitText);
    void BtnCont_Create(lv_obj_t* par);
    lv_obj_t* Btn_Create(lv_obj_t* par, const void* img_src, lv_coord_t x_ofs);
};

}

#endif // !__VIEW_H
