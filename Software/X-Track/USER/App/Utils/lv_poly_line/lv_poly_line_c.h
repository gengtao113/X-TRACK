#ifndef LV_POLY_LINE_C_H
#define LV_POLY_LINE_C_H

#include "lvgl/lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
class lv_poly_line;
extern "C" {
#else
typedef struct lv_poly_line lv_poly_line;
#endif

lv_poly_line* lv_poly_line_create(lv_obj_t* par);
void          lv_poly_line_destroy(lv_poly_line* p);
void          lv_poly_line_set_style(lv_poly_line* p, lv_style_t* style);
void          lv_poly_line_start(lv_poly_line* p);
void          lv_poly_line_append(lv_poly_line* p, lv_coord_t x, lv_coord_t y);
void          lv_poly_line_append_to_end(lv_poly_line* p, lv_coord_t x, lv_coord_t y);
void          lv_poly_line_stop(lv_poly_line* p);
void          lv_poly_line_reset(lv_poly_line* p);
bool          lv_poly_line_get_end_point(lv_poly_line* p, lv_point_t* point);

#ifdef __cplusplus
}
#endif

#endif
