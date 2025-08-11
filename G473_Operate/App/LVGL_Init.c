#include "LVGL_Init.h"

// 启动画面持续时间（毫秒）
#define SPLASH_SCREEN_DURATION 2500

// 启动画面对象
static lv_obj_t * splash_screen;
static lv_timer_t * splash_timer;

// 启动画面中的动画对象
static lv_obj_t * btn_slider_y;
static lv_obj_t * btn_slider_x;
static lv_obj_t * label_symbol;
static lv_obj_t * img_haibara;

// 全局UI结构体实例
lv_ui_t g_ui;

bool main_screen_loaded = false;

// 创建屏幕对象函数
static void setup_scr_screen(lv_ui_t *ui);

// 动画完成回调函数声明
static void anim_x_completed_cb(lv_anim_t * anim);
static void anim_img_completed_cb(lv_anim_t * anim);

// 延迟删除启动画面的回调函数
static void delay_delete_splash_screen(lv_timer_t *t)
{
    if(splash_screen)
    {
        lv_obj_del_async(splash_screen);
        splash_screen = NULL; // 清除野指针
    }
    lv_anim_del_all();

    lv_timer_del(t);
}

// 启动画面定时器回调函数
static void splash_timer_cb(lv_timer_t * timer)
{
    // 1. 提前创建主屏幕（在后台准备，不立即显示）
    setup_scr_screen(&g_ui);
    if(g_ui.screen == NULL)
    {
        fr_printf("main screen create failed");
        return;
    }
    

    // 2. 停止所有启动画面动画（避免动画干扰）
    lv_anim_del_all();

    // 3. 删除定时器（防止重复触发）
    lv_timer_del(splash_timer);
    splash_timer = NULL;

    // 4. 切换到主屏幕（使用过渡动画）
    lv_scr_load_anim(g_ui.screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
    // lv_scr_load(g_ui.screen);
    main_screen_loaded = true;

    // 5. 延迟删除启动画面（确保主屏幕已渲染完成）
    lv_timer_create(delay_delete_splash_screen, 300, NULL);
}

// 创建启动画面
void create_splash_screen(void)
{
    // 创建启动画面屏幕
    splash_screen = lv_obj_create(NULL);
    lv_obj_set_size(splash_screen, 320, 240);
    lv_obj_set_scrollbar_mode(splash_screen, LV_SCROLLBAR_MODE_OFF);

    // 设置背景为黑色
    lv_obj_set_style_bg_opa(splash_screen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建背景容器
    lv_obj_t * cont_backgrn = lv_obj_create(splash_screen);
    lv_obj_set_pos(cont_backgrn, 0, 0);
    lv_obj_set_size(cont_backgrn, 320, 240);
    lv_obj_set_scrollbar_mode(cont_backgrn, LV_SCROLLBAR_MODE_OFF);

    // 设置背景样式
    lv_obj_set_style_border_width(cont_backgrn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cont_backgrn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont_backgrn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cont_backgrn, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(cont_backgrn, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cont_backgrn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cont_backgrn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cont_backgrn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cont_backgrn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(cont_backgrn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建垂直滑块（装饰元素）
    btn_slider_y = lv_btn_create(splash_screen);
    lv_obj_t * btn_slider_y_label = lv_label_create(btn_slider_y);
    lv_label_set_text(btn_slider_y_label, "");
    lv_label_set_long_mode(btn_slider_y_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(btn_slider_y_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(btn_slider_y, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(btn_slider_y_label, LV_PCT(100));
    lv_obj_set_pos(btn_slider_y, 51, 22);
    lv_obj_set_size(btn_slider_y, 12, 30);

    // 设置垂直滑块样式
    lv_obj_set_style_bg_opa(btn_slider_y, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_slider_y, lv_color_hex(0xffa412), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(btn_slider_y, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_slider_y, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_slider_y, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_slider_y, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_slider_y, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(btn_slider_y, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(btn_slider_y, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建标题标签
    label_symbol = lv_label_create(splash_screen);
    lv_label_set_text(label_symbol, "H TOOL");
    lv_label_set_long_mode(label_symbol, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(label_symbol, 54, 240);
    lv_obj_set_size(label_symbol, 128, 32);

    // 设置标题标签样式
    lv_obj_set_style_border_width(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_symbol, lv_color_hex(0xffa412), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_symbol, &My_start_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label_symbol, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label_symbol, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(label_symbol, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建启动图片
    img_haibara = lv_img_create(splash_screen);
    lv_obj_add_flag(img_haibara, LV_OBJ_FLAG_CLICKABLE);
    LV_IMG_DECLARE(start_haibara);
    lv_img_set_src(img_haibara, &start_haibara);
    lv_img_set_pivot(img_haibara, 50,50);
    lv_img_set_angle(img_haibara, 0);
    lv_obj_set_pos(img_haibara, 120, 34);
    lv_obj_set_size(img_haibara, 196, 150);

    // 设置图片样式
    lv_obj_set_style_img_recolor_opa(img_haibara, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_img_opa(img_haibara, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(img_haibara, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(img_haibara, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建版本标签
    lv_obj_t * label_version = lv_label_create(splash_screen);
    lv_label_set_text(label_version, "V1.0");
    lv_label_set_long_mode(label_version, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(label_version, 173, 193);
    lv_obj_set_size(label_version, 128, 32);

    // 设置版本标签样式
    lv_obj_set_style_border_width(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_version, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_version, &My_start_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label_version, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label_version, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建水平滑块（装饰元素）
    btn_slider_x = lv_btn_create(splash_screen);
    lv_obj_t * btn_slider_x_label = lv_label_create(btn_slider_x);
    lv_label_set_text(btn_slider_x_label, "");
    lv_label_set_long_mode(btn_slider_x_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(btn_slider_x_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(btn_slider_x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(btn_slider_x_label, LV_PCT(100));
    lv_obj_set_pos(btn_slider_x, 261, 10);
    lv_obj_set_size(btn_slider_x, 30, 12);

    // 设置水平滑块样式
    lv_obj_set_style_bg_opa(btn_slider_x, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_slider_x, lv_color_hex(0xffa412), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(btn_slider_x, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_slider_x, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_slider_x, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_slider_x, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_slider_x, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(btn_slider_x, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(btn_slider_x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建垂直滑块向下移动的动画
    lv_anim_t anim_y;
    lv_anim_init(&anim_y);
    lv_anim_set_var(&anim_y, btn_slider_y);
    lv_anim_set_values(&anim_y, -30, 150); // 从y=-30移动到y=150
    lv_anim_set_time(&anim_y, 500);
    lv_anim_set_path_cb(&anim_y, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&anim_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_start(&anim_y);

    // 创建水平滑块向左移动的动画
    lv_anim_t anim_x;
    lv_anim_init(&anim_x);
    lv_anim_set_var(&anim_x, btn_slider_x);
    lv_anim_set_values(&anim_x, 320, 62); // 从x=320移动到x=62
    lv_anim_set_time(&anim_x, 500);
    lv_anim_set_path_cb(&anim_x, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&anim_x, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_ready_cb(&anim_x, anim_x_completed_cb); // 设置完成回调
    lv_anim_start(&anim_x);

    // 更新布局
    lv_obj_update_layout(splash_screen);
    
    // 将启动画面设置为当前屏幕
    lv_scr_load(splash_screen);

    // 开启屏幕背光
    ST7789_BLK_LOW();
    // 强制刷新屏幕
    lv_refr_now(NULL);

    // 创建定时器，在指定时间后切换到主屏幕
    splash_timer = lv_timer_create(splash_timer_cb, SPLASH_SCREEN_DURATION, NULL);
}

// 水平滑块动画完成回调
static void anim_x_completed_cb(lv_anim_t * anim)
{
    // 创建版本标签从下往上移动的动画
    lv_anim_t anim_version;
    lv_anim_init(&anim_version);
    lv_anim_set_var(&anim_version, label_symbol);
    lv_anim_set_values(&anim_version, 240, 193); // 从y=240移动到y=193
    lv_anim_set_time(&anim_version, 500);
    lv_anim_set_path_cb(&anim_version, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&anim_version, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_ready_cb(&anim_version, anim_img_completed_cb); // 设置完成回调
    lv_anim_start(&anim_version);
}

// 图片动画完成回调
static void anim_img_completed_cb(lv_anim_t * anim)
{
    // 创建启动图片小段平移的动画
    lv_anim_t anim_img;
    lv_anim_init(&anim_img);
    lv_anim_set_var(&anim_img, img_haibara);
    lv_anim_set_values(&anim_img, 120, 90); // 从y=120移动到y=100
    lv_anim_set_time(&anim_img, 500);
    lv_anim_set_path_cb(&anim_img, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&anim_img, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_ready_cb(&anim_img, NULL);
    lv_anim_start(&anim_img);
}

void Gui_Init(void)
{
    // 初始化LVGL库
    lv_init();
    
    // 初始化显示端口
    lv_port_disp_init();

    // 创建启动画面
    create_splash_screen();
}

static void setup_scr_screen(lv_ui_t *ui)
{
    // 防止重复创建（如果已创建则直接返回）
    if(ui->screen != NULL) return;

    // 禁用屏幕更新，直到所有对象创建完成
    lv_obj_invalidate(lv_scr_act()); // 触发一次无效化
    
    // ======================================================================
    // 创建屏幕对象
    // ======================================================================
    // LV_IMG_DECLARE(background);

    ui->screen = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);

    // // 创建背景图像对象
    // lv_obj_t * bg_img = lv_img_create(ui->screen);
    // lv_img_set_src(bg_img, &background);
    // lv_obj_align(bg_img, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_move_background(bg_img);
// // 设置屏幕背景样式为透明，以便能看到背景图像
//     lv_obj_set_style_bg_opa(ui->screen, LV_OPA_TRANSP, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 设置屏幕背景样式
    lv_obj_set_style_bg_opa(ui->screen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->screen, 213, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(ui->screen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    

    // ======================================================================
    // 电压显示控件
    // ======================================================================
    // 电压标签对象
    ui->screen_label_voltage = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_voltage, "\n\n                 V");
    lv_label_set_long_mode(ui->screen_label_voltage, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_voltage, 5, 135);
    lv_obj_set_size(ui->screen_label_voltage, 100, 100);

    // 电压标签对象样式
    lv_obj_set_style_border_width(ui->screen_label_voltage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_voltage, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_voltage, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_voltage, &lv_font_montserrat_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_voltage, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_voltage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_voltage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_voltage, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_voltage, 179, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_label_voltage, lv_color_hex(0x008b32), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_label_voltage, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_voltage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_voltage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_voltage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_voltage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 电压按钮
    ui->screen_btn_Voltage = lv_btn_create(ui->screen);
    ui->screen_btn_Voltage_label = lv_label_create(ui->screen_btn_Voltage);
    lv_label_set_text(ui->screen_btn_Voltage_label, "电压");
    lv_label_set_long_mode(ui->screen_btn_Voltage_label, LV_LABEL_LONG_CLIP);
    lv_obj_align(ui->screen_btn_Voltage_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_btn_Voltage, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_btn_Voltage_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_btn_Voltage, 7, 137);
    lv_obj_set_size(ui->screen_btn_Voltage, 96, 20);

    // 电压按钮样式
    lv_obj_set_style_bg_opa(ui->screen_btn_Voltage, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_btn_Voltage, lv_color_hex(0x128505), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_btn_Voltage, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_btn_Voltage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_btn_Voltage, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_btn_Voltage, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui->screen_btn_Voltage, lv_color_hex(0x064501), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui->screen_btn_Voltage, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui->screen_btn_Voltage, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui->screen_btn_Voltage, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui->screen_btn_Voltage, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_btn_Voltage, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_btn_Voltage, &My_GUI_Chinese, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_btn_Voltage, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_btn_Voltage, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 当前电压值显示
    ui->screen_label_Voltage_now = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_Voltage_now, "0.000");
    lv_label_set_long_mode(ui->screen_label_Voltage_now, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_Voltage_now, 10, 166);
    lv_obj_set_size(ui->screen_label_Voltage_now, 86, 24);

    // 当前电压值样式
    lv_obj_set_style_border_width(ui->screen_label_Voltage_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_Voltage_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_Voltage_now, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_Voltage_now, &My_start_26, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_Voltage_now, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_Voltage_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_Voltage_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_Voltage_now, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_Voltage_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_Voltage_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_Voltage_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_Voltage_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_Voltage_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 设定电压值显示
    ui->screen_label_Voltage_Set = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_Voltage_Set, "0.000 V");
    lv_label_set_long_mode(ui->screen_label_Voltage_Set, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_Voltage_Set, 50, 198);
    lv_obj_set_size(ui->screen_label_Voltage_Set, 60, 15);

    // 设定电压值样式
    lv_obj_set_style_border_width(ui->screen_label_Voltage_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_Voltage_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_Voltage_Set, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_Voltage_Set, &lv_font_montserrat_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_Voltage_Set, 243, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_Voltage_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_Voltage_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_Voltage_Set, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_Voltage_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_Voltage_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_Voltage_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_Voltage_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_Voltage_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ======================================================================
    // 电流显示控件
    // ======================================================================
    // 电流标签对象
    ui->screen_label_Current = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_Current, "\n\n                 A");
    lv_label_set_long_mode(ui->screen_label_Current, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_Current, 110, 135);
    lv_obj_set_size(ui->screen_label_Current, 100, 100);

    // 电流标签对象样式
    lv_obj_set_style_border_width(ui->screen_label_Current, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_Current, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_Current, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_Current, &lv_font_montserrat_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_Current, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_Current, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_Current, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_Current, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_Current, 216, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_label_Current, lv_color_hex(0xb0a20f), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_label_Current, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_Current, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_Current, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_Current, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_Current, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 电流按钮
    ui->screen_btn_Current = lv_btn_create(ui->screen);
    ui->screen_btn_Current_label = lv_label_create(ui->screen_btn_Current);
    lv_label_set_text(ui->screen_btn_Current_label, "电流");
    lv_label_set_long_mode(ui->screen_btn_Current_label, LV_LABEL_LONG_CLIP);
    lv_obj_align(ui->screen_btn_Current_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_btn_Current, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_btn_Current_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_btn_Current, 112, 137);
    lv_obj_set_size(ui->screen_btn_Current, 96, 20);

    // 电流按钮样式
    lv_obj_set_style_bg_opa(ui->screen_btn_Current, 198, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_btn_Current, lv_color_hex(0xb7aa26), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_btn_Current, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_btn_Current, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_btn_Current, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_btn_Current, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui->screen_btn_Current, lv_color_hex(0x666008), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui->screen_btn_Current, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui->screen_btn_Current, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui->screen_btn_Current, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui->screen_btn_Current, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_btn_Current, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_btn_Current, &My_GUI_Chinese, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_btn_Current, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_btn_Current, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 当前电流值显示
    ui->screen_label_Current_now = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_Current_now, "0.000");
    lv_label_set_long_mode(ui->screen_label_Current_now, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_Current_now, 115, 166);
    lv_obj_set_size(ui->screen_label_Current_now, 86, 25);

    // 当前电流值样式
    lv_obj_set_style_border_width(ui->screen_label_Current_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_Current_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_Current_now, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_Current_now, &My_start_26, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_Current_now, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_Current_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_Current_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_Current_now, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_Current_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_Current_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_Current_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_Current_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_Current_now, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 设定电流值显示
    ui->screen_label_Current_Set = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_Current_Set, "0.000 A");
    lv_label_set_long_mode(ui->screen_label_Current_Set, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_Current_Set, 155, 198);
    lv_obj_set_size(ui->screen_label_Current_Set, 60, 15);

    // 设定电流值样式
    lv_obj_set_style_border_width(ui->screen_label_Current_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_Current_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_Current_Set, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_Current_Set, &lv_font_montserrat_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_Current_Set, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_Current_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_Current_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_Current_Set, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_Current_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_Current_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_Current_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_Current_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_Current_Set, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ======================================================================
    // 功率显示控件
    // ======================================================================
    // 功率标签对象
    ui->screen_label_power = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_power, "\n0.000 W");
    lv_label_set_long_mode(ui->screen_label_power, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_power, 215, 135);
    lv_obj_set_size(ui->screen_label_power, 100, 48);

    // 功率标签对象样式
    lv_obj_set_style_border_width(ui->screen_label_power, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_power, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_power, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_power, &lv_font_montserrat_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_power, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_power, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_power, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_power, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_power, 153, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_label_power, lv_color_hex(0x05e8f2), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_label_power, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_power, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_power, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_power, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_power, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 功率按钮
    ui->screen_btn_Power = lv_btn_create(ui->screen);
    ui->screen_btn_Power_label = lv_label_create(ui->screen_btn_Power);
    lv_label_set_text(ui->screen_btn_Power_label, "功率");
    lv_label_set_long_mode(ui->screen_btn_Power_label, LV_LABEL_LONG_CLIP);
    lv_obj_align(ui->screen_btn_Power_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_btn_Power, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_btn_Power_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_btn_Power, 217, 137);
    lv_obj_set_size(ui->screen_btn_Power, 96, 18);

    // 功率按钮样式
    lv_obj_set_style_bg_opa(ui->screen_btn_Power, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_btn_Power, lv_color_hex(0x069dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_btn_Power, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_btn_Power, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_btn_Power, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_btn_Power, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui->screen_btn_Power, lv_color_hex(0x10505b), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui->screen_btn_Power, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui->screen_btn_Power, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui->screen_btn_Power, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui->screen_btn_Power, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_btn_Power, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_btn_Power, &My_GUI_Chinese, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_btn_Power, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_btn_Power, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ======================================================================
    // 能量显示控件
    // ======================================================================
    // 能量标签对象
    ui->screen_label_energy = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_energy, "\n 0.00 mWh");
    lv_label_set_long_mode(ui->screen_label_energy, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_energy, 215, 186);
    lv_obj_set_size(ui->screen_label_energy, 100, 48);

    // 能量标签对象样式
    lv_obj_set_style_border_width(ui->screen_label_energy, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_energy, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_energy, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_energy, &lv_font_montserrat_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_energy, 251, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_energy, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_energy, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_energy, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_energy, 194, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_label_energy, lv_color_hex(0xa40242), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_label_energy, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_energy, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_energy, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_energy, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_energy, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 能量按钮
    ui->screen_btn_energy = lv_btn_create(ui->screen);
    ui->screen_btn_energy_label = lv_label_create(ui->screen_btn_energy);
    lv_label_set_text(ui->screen_btn_energy_label, "能量");
    lv_label_set_long_mode(ui->screen_btn_energy_label, LV_LABEL_LONG_CLIP);
    lv_obj_align(ui->screen_btn_energy_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_btn_energy, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_btn_energy_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_btn_energy, 217, 188);
    lv_obj_set_size(ui->screen_btn_energy, 96, 18);

    // 能量按钮样式
    lv_obj_set_style_bg_opa(ui->screen_btn_energy, 183, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_btn_energy, lv_color_hex(0x940140), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_btn_energy, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_btn_energy, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_btn_energy, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_btn_energy, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui->screen_btn_energy, lv_color_hex(0x24000e), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui->screen_btn_energy, 220, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui->screen_btn_energy, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(ui->screen_btn_energy, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(ui->screen_btn_energy, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_btn_energy, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_btn_energy, &My_GUI_Chinese, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_btn_energy, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_btn_energy, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ======================================================================
    // 状态信息显示控件
    // ======================================================================
    // 超时时间显示
    ui->screen_label_Timeout = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_Timeout, "00:00:00");
    lv_label_set_long_mode(ui->screen_label_Timeout, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_Timeout, 90, 5);
    lv_obj_set_size(ui->screen_label_Timeout, 100, 26);

    // 超时时间显示样式
    lv_obj_set_style_border_width(ui->screen_label_Timeout, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_label_Timeout, 51, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_label_Timeout, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_label_Timeout, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_Timeout, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_Timeout, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_Timeout, &lv_font_montserrat_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_Timeout, 226, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_Timeout, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_Timeout, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_Timeout, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_Timeout, 69, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_label_Timeout, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_label_Timeout, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_Timeout, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_Timeout, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_Timeout, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_Timeout, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 温度显示
    ui->screen_label_Temperature = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_Temperature, "0.0°C");
    lv_label_set_long_mode(ui->screen_label_Temperature, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_Temperature, 195, 5);
    lv_obj_set_size(ui->screen_label_Temperature, 80, 26);

    // 温度显示样式
    lv_obj_set_style_border_width(ui->screen_label_Temperature, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_label_Temperature, 37, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_label_Temperature, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_label_Temperature, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_Temperature, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_Temperature, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_Temperature, &lv_font_montserrat_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_Temperature, 213, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_Temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_Temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_Temperature, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_Temperature, 69, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_label_Temperature, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_label_Temperature, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_Temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_Temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_Temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_Temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 启动状态显示
    ui->screen_label_Start = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_Start, "  R");
    lv_label_set_long_mode(ui->screen_label_Start, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_Start, 279, 5);
    lv_obj_set_size(ui->screen_label_Start, 35, 26);

    // 启动状态显示样式
    lv_obj_set_style_border_width(ui->screen_label_Start, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_label_Start, 37, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_label_Start, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_label_Start, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_Start, &lv_font_montserrat_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_Start, 140, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_Start, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_Start, 69, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_label_Start, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_label_Start, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_Start, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_Start, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_Start, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_Start, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 工作模式显示
    ui->screen_label_mode = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_mode, "CC/CV");
    lv_label_set_long_mode(ui->screen_label_mode, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_mode, 5, 5);
    lv_obj_set_size(ui->screen_label_mode, 80, 26);

    // 工作模式显示样式
    lv_obj_set_style_border_width(ui->screen_label_mode, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_label_mode, 37, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_label_mode, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_label_mode, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_mode, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_mode, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_mode, &lv_font_montserrat_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_mode, 243, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_mode, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_mode, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_mode, 69, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_label_mode, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_label_mode, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 主显示值
    ui->screen_label_main = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_main, "0.000");
    lv_label_set_long_mode(ui->screen_label_main, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_main, 10, 52);
    lv_obj_set_size(ui->screen_label_main, 240, 80);

    // 主显示值样式
    lv_obj_set_style_border_width(ui->screen_label_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_main, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_main, &My_GUI_73, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_main, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_main, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_main, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 单位显示
    ui->screen_label_select_unit = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_select_unit, "V");
    lv_label_set_long_mode(ui->screen_label_select_unit, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_select_unit, 200, 87);
    lv_obj_set_size(ui->screen_label_select_unit, 100, 32);

    // 单位显示样式
    lv_obj_set_style_border_width(ui->screen_label_select_unit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_select_unit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_select_unit, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_select_unit, &My_start_26, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_select_unit, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_select_unit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_select_unit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_select_unit, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_select_unit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_select_unit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_select_unit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_select_unit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_select_unit, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ======================================================================
    // 输入信息显示控件
    // ======================================================================
    // 输入标签1
    ui->screen_label_in1 = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_in1, "输入:");
    lv_label_set_long_mode(ui->screen_label_in1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_in1, 19, 219);
    lv_obj_set_size(ui->screen_label_in1, 56, 15);

    // 输入标签1样式
    lv_obj_set_style_border_width(ui->screen_label_in1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_in1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_in1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_in1, &My_GUI_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_in1, 231, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_in1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_in1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_in1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_in1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_in1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_in1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_in1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_in1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 输入标签2
    ui->screen_label_in2 = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_in2, "输入:");
    lv_label_set_long_mode(ui->screen_label_in2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_in2, 124, 219);
    lv_obj_set_size(ui->screen_label_in2, 45, 14);

    // 输入标签2样式
    lv_obj_set_style_border_width(ui->screen_label_in2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_in2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_in2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_in2, &My_GUI_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_in2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_in2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_in2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_in2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_in2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_in2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_in2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_in2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_in2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 输入电压值
    ui->screen_label_in_v = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_in_v, "0.000 V");
    lv_label_set_long_mode(ui->screen_label_in_v, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_in_v, 50, 220);
    lv_obj_set_size(ui->screen_label_in_v, 56, 15);

    // 输入电压值样式
    lv_obj_set_style_border_width(ui->screen_label_in_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_in_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_in_v, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_in_v, &lv_font_montserrat_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_in_v, 243, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_in_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_in_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_in_v, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_in_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_in_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_in_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_in_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_in_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 输入电流值
    ui->screen_label_in_a = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_in_a, "0.000 A");
    lv_label_set_long_mode(ui->screen_label_in_a, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_in_a, 155, 220);
    lv_obj_set_size(ui->screen_label_in_a, 56, 15);

    // 输入电流值样式
    lv_obj_set_style_border_width(ui->screen_label_in_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_in_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_in_a, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_in_a, &lv_font_montserrat_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_in_a, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_in_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_in_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_in_a, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_in_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_in_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_in_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_in_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_in_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 设定电压标签
    ui->screen_label_set_v = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_set_v, "设定:");
    lv_label_set_long_mode(ui->screen_label_set_v, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_set_v, 19, 198);
    lv_obj_set_size(ui->screen_label_set_v, 56, 15);

    // 设定电压标签样式
    lv_obj_set_style_border_width(ui->screen_label_set_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_set_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_set_v, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_set_v, &My_GUI_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_set_v, 251, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_set_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_set_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_set_v, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_set_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_set_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_set_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_set_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_set_v, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 设定电流标签
    ui->screen_label_set_a = lv_label_create(ui->screen);
    lv_label_set_text(ui->screen_label_set_a, "设定:");
    lv_label_set_long_mode(ui->screen_label_set_a, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_label_set_a, 124, 198);
    lv_obj_set_size(ui->screen_label_set_a, 56, 15);

    // 设定电流标签样式
    lv_obj_set_style_border_width(ui->screen_label_set_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_label_set_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_label_set_a, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_label_set_a, &My_GUI_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_label_set_a, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_label_set_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_label_set_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_label_set_a, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_label_set_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_label_set_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_label_set_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_label_set_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_label_set_a, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // ======================================================================
    // 分隔线
    // ======================================================================
    // 电压分隔线
    ui->screen_line_V = lv_line_create(ui->screen);
    static lv_point_t screen_line_V[] = {{0, 0},{98, 0},};
    lv_line_set_points(ui->screen_line_V, screen_line_V, 2);
    lv_obj_set_pos(ui->screen_line_V, 6, 216);
    lv_obj_set_size(ui->screen_line_V, 103, 1);

    // 电压分隔线样式
    lv_obj_set_style_line_width(ui->screen_line_V, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->screen_line_V, lv_color_hex(0x054300), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->screen_line_V, 204, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->screen_line_V, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 电流分隔线
    ui->screen_line_A = lv_line_create(ui->screen);
    static lv_point_t screen_line_A[] = {{0, 0},{98, 0},};
    lv_line_set_points(ui->screen_line_A, screen_line_A, 2);
    lv_obj_set_pos(ui->screen_line_A, 111, 216);
    lv_obj_set_size(ui->screen_line_A, 103, 1);

    // 电流分隔线样式
    lv_obj_set_style_line_width(ui->screen_line_A, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->screen_line_A, lv_color_hex(0x716805), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->screen_line_A, 130, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->screen_line_A, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 更新屏幕布局
    lv_obj_update_layout(ui->screen);
}
