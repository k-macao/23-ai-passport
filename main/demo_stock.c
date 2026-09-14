// main/demo_stock.c —— 主页循环播放全球核心指数（A股、港股、美股）
#include "demo.h"
#include "stock_data.h"
#include "bsp_battery.h"
#include "ui_pixel.h"
#include "lvgl.h"
#include "esp_timer.h"
#include <stdio.h>

static const stock_item_t STOCKS[] = {
    {
        .market = "A-SHARE | PANORAMA",
        .name = "SHANGHAI (000001)",
        .points_str = "3,885.33",
        .chg_str = "-0.07%",
        .is_up = false,
    },
    {
        .market = "HK-STOCK | PANORAMA",
        .name = "HANG SENG (HSI)",
        .points_str = "24,917.60",
        .chg_str = "+0.45%",
        .is_up = true,
    },
    {
        .market = "US-STOCK | PANORAMA",
        .name = "NASDAQ (IXIC)",
        .points_str = "26,064.96",
        .chg_str = "-1.02%",
        .is_up = false,
    },
};
#define STOCK_COUNT (sizeof(STOCKS) / sizeof(STOCKS[0]))

static stock_carousel_t s_carousel;
static lv_obj_t *s_scr;
static lv_obj_t *s_card;
static lv_obj_t *s_lbl_market;
static lv_obj_t *s_lbl_name;
static lv_obj_t *s_lbl_points;
static lv_obj_t *s_lbl_chg;
static lv_obj_t *s_lbl_dots;
static lv_obj_t *s_lbl_battery;
static lv_obj_t *s_mascot;
static esp_timer_handle_t s_timer;

static void update_battery_label(void) {
    if (!s_lbl_battery) return;
    int soc = bsp_battery_soc();
    if (soc >= 0 && soc <= 100) {
        lv_label_set_text_fmt(s_lbl_battery, "BAT %d%%", soc);
    } else {
        lv_label_set_text(s_lbl_battery, "");
    }
}

static void stock_refresh_ui(void) {
    if (!s_card) return;
    const stock_item_t *item = &STOCKS[s_carousel.current_index];

    lv_label_set_text(s_lbl_market, item->market);
    lv_label_set_text(s_lbl_name, item->name);
    lv_label_set_text(s_lbl_points, item->points_str);
    lv_label_set_text(s_lbl_chg, item->chg_str);

    // 涨跌色彩：涨=红色/UI_RED，跌=绿色/UI_GRASS_DARK（符合国内股票惯例与LVGL配色）
    uint32_t trend_color = item->is_up ? UI_RED : UI_GRASS_DARK;
    lv_obj_set_style_text_color(s_lbl_points, lv_color_hex(trend_color), 0);
    lv_obj_set_style_text_color(s_lbl_chg, lv_color_hex(trend_color), 0);

    // 指示点：如 ● ○ ○
    char dots[32];
    snprintf(dots, sizeof(dots), "%s  %s  %s",
             s_carousel.current_index == 0 ? "[1]" : " 1 ",
             s_carousel.current_index == 1 ? "[2]" : " 2 ",
             s_carousel.current_index == 2 ? "[3]" : " 3 ");
    lv_label_set_text(s_lbl_dots, dots);

    update_battery_label();
}

static void timer_cb(void *arg) {
    (void)arg;
    if (!bsp_lvgl_lock(200)) return;
    stock_carousel_next(&s_carousel);
    stock_refresh_ui();
    ui_pixel_mascot_jump(s_mascot);
    bsp_lvgl_unlock();
}

void demo_stock_enter(void) {
    stock_carousel_init(&s_carousel, STOCK_COUNT);

    // 标题展示：OCTOPUS AI (章鱼 AI·全景分析)
    s_scr = ui_pixel_screen_create("OCTOPUS AI");

    // 右上角电量显示（避开 x≈188, y≈8 的白云装饰）
    s_lbl_battery = lv_label_create(s_scr);
    lv_obj_set_pos(s_lbl_battery, 160, 16);
    lv_obj_set_style_text_font(s_lbl_battery, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_lbl_battery, lv_color_hex(UI_INK), 0);

    // 主展示面板
    s_card = ui_pixel_panel_create(s_scr, 14, 52, 212, 190, UI_PAPER);

    // 市场分类与全景副标
    s_lbl_market = lv_label_create(s_card);
    lv_obj_set_pos(s_lbl_market, 10, 8);
    lv_obj_set_style_text_font(s_lbl_market, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_lbl_market, lv_color_hex(0x555555), 0);

    // 指数全名/代码
    s_lbl_name = lv_label_create(s_card);
    lv_obj_set_pos(s_lbl_name, 10, 28);
    lv_obj_set_style_text_font(s_lbl_name, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_lbl_name, lv_color_hex(UI_INK), 0);

    // 点数（大字 20）
    s_lbl_points = lv_label_create(s_card);
    lv_obj_set_pos(s_lbl_points, 10, 56);
    lv_obj_set_style_text_font(s_lbl_points, &lv_font_montserrat_20, 0);

    // 涨跌幅
    s_lbl_chg = lv_label_create(s_card);
    lv_obj_set_pos(s_lbl_chg, 10, 86);
    lv_obj_set_style_text_font(s_lbl_chg, &lv_font_montserrat_14, 0);

    // 轮播页面指示器
    s_lbl_dots = lv_label_create(s_card);
    lv_obj_set_pos(s_lbl_dots, 10, 116);
    lv_obj_set_style_text_font(s_lbl_dots, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_lbl_dots, lv_color_hex(0x666666), 0);

    // 提示信息
    lv_obj_t *hint = lv_label_create(s_card);
    lv_obj_set_pos(hint, 10, 142);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x888888), 0);
    lv_label_set_text(hint, "UP/DN: SWITCH");

    s_mascot = ui_pixel_mascot_create(s_scr, 101, 244);

    stock_refresh_ui();
    lv_screen_load(s_scr);
}

void demo_stock_exit(void) {
    if (s_timer) {
        esp_timer_stop(s_timer);
        esp_timer_delete(s_timer);
        s_timer = NULL;
    }
    if (s_scr) {
        lv_obj_delete(s_scr);
        s_scr = NULL;
        s_card = NULL;
        s_lbl_market = NULL;
        s_lbl_name = NULL;
        s_lbl_points = NULL;
        s_lbl_chg = NULL;
        s_lbl_dots = NULL;
        s_lbl_battery = NULL;
        s_mascot = NULL;
    }
}

void demo_stock_key(bsp_btn_t btn, bsp_btn_ev_t ev) {
    if (ev != BSP_BTN_CLICK) return;
    if (!bsp_lvgl_lock(250)) return;
    if (btn == BSP_BTN_DOWN || btn == BSP_BTN_OK) {
        stock_carousel_next(&s_carousel);
    } else if (btn == BSP_BTN_UP) {
        stock_carousel_prev(&s_carousel);
    }
    stock_refresh_ui();
    ui_pixel_mascot_jump(s_mascot);
    bsp_lvgl_unlock();
}

esp_err_t demo_stock_start(void) {
    const esp_timer_create_args_t timer_args = {
        .callback = timer_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "stock_carousel",
    };
    esp_err_t err = esp_timer_create(&timer_args, &s_timer);
    if (err == ESP_OK) {
        // 每 3.5 秒自动轮播切换
        err = esp_timer_start_periodic(s_timer, 3500 * 1000);
    }
    return err;
}

esp_err_t demo_stock_stop(void) {
    if (s_timer) {
        esp_timer_stop(s_timer);
        esp_timer_delete(s_timer);
        s_timer = NULL;
    }
    return ESP_OK;
}
