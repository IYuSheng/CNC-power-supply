/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>
#include "st7789.h"

/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
    #define MY_DISP_HOR_RES    320
#endif

#ifndef MY_DISP_VER_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen height, default value 240 is used for now.
    #define MY_DISP_VER_RES    240
#endif

lv_disp_drv_t * current_drv = NULL;
const lv_area_t * current_area = NULL;

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();
    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    /* Example for 1) */
//    static lv_disp_draw_buf_t draw_buf_dsc_1;
//    static lv_color_t buf_1[MY_DISP_HOR_RES * 5];                          /*A buffer for 10 rows*/
//    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 5);   /*Initialize the display buffer*/

    /* Example for 2) */
    static lv_disp_draw_buf_t draw_buf_dsc_2;
    static lv_color_t buf_2_1[MY_DISP_HOR_RES * 20];                        /*A buffer for 10 rows*/
    static lv_color_t buf_2_2[MY_DISP_HOR_RES * 20];                        /*An other buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * 20);   /*Initialize the display buffer*/

//    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
//    static lv_disp_draw_buf_t draw_buf_dsc_3;
//    static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*A screen sized buffer*/
//    static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*Another screen sized buffer*/
//    lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2,
//                          MY_DISP_VER_RES * LV_VER_RES_MAX);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_2;

    /*Required for Example 3)*/
    //disp_drv.full_refresh = 1;

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
	ST7789_Init();
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{   
    if(!disp_flush_enabled)
    {
        lv_disp_flush_ready(disp_drv);
        return;
    }
    
    /* 保存当前驱动和区域信息，用于中断处理 */
    current_drv = disp_drv;
    current_area = area;
    
    /* 设置显示窗口 */
    ST7789_SetWindow(area->x1, area->y1, area->x2, area->y2);
    
    /* 计算需要传输的像素数量和字节数 */
    uint32_t width = area->x2 - area->x1 + 1;
    uint32_t height = area->y2 - area->y1 + 1;
    uint32_t pixel_count = width * height;
    uint32_t byte_count = pixel_count * 2; // 每个像素2字节
    
    /* 确保DMA通道未启用 */
    LL_DMA_DisableChannel(ST7789_DMA, ST7789_DMA_CHANNEL);
    while(LL_DMA_IsEnabledChannel(ST7789_DMA, ST7789_DMA_CHANNEL));
    
    /* 配置DMA传输 */
    LL_DMA_SetMemoryAddress(ST7789_DMA, ST7789_DMA_CHANNEL, (uint32_t)color_p);
    LL_DMA_SetDataLength(ST7789_DMA, ST7789_DMA_CHANNEL, byte_count);
    
    /* 拉低CS使能LCD，拉高D/C表示发送数据 */
    ST7789_CS_LOW();
    ST7789_DC_HIGH();
    
    /* 清除DMA传输完成标志 */
    LL_DMA_ClearFlag_TC3(ST7789_DMA);
    
    /* 启用SPI的DMA请求 */
    LL_SPI_EnableDMAReq_TX(ST7789_SPI);

    /* 启用DMA通道开始传输 */
    LL_DMA_EnableChannel(ST7789_DMA, ST7789_DMA_CHANNEL);

    /* 注意：lv_disp_flush_ready()将在DMA中断处理函数中调用 */
}

// static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
// {
//   if(disp_flush_enabled)
//   {
//     // 正确提取LVGL的区域坐标（左、上、右、下）
//     uint16_t screen_left = area->x1;
//     uint16_t screen_top = area->y1;
//     uint16_t screen_right = area->x2;
//     uint16_t screen_bottom = area->y2;

//     // 设置显示窗口（使用正确的方向，与ST7789_Init一致）
//     ST7789_SetWindow(screen_left, screen_top, screen_right, screen_bottom, ST7789_DIR_PORTRAIT_FLIP);

//     // 计算像素数（每个像素2字节RGB565）
//     uint32_t pixel_num = (screen_right - screen_left + 1) * (screen_bottom - screen_top + 1);
//     uint8_t *color_buf = (uint8_t *)color_p;  // LVGL的颜色数据直接映射

//     // 发送数据到屏幕（保持SPI逻辑不变）
//     ST7789_CS_LOW();
//     ST7789_DC_HIGH();
//     while (!LL_SPI_IsActiveFlag_TXE(SPI1));

//     for (uint32_t i = 0; i < pixel_num * 2; i++)
//     {
//       LL_SPI_TransmitData8(SPI1, color_buf[i]);
//       if(i < (pixel_num * 2 - 1) && !LL_SPI_IsActiveFlag_TXE(SPI1))
//       {
//         while (!LL_SPI_IsActiveFlag_TXE(SPI1));
//       }
//     }

//     while (LL_SPI_IsActiveFlag_BSY(SPI1));
//     ST7789_CS_HIGH();
//   }
//   lv_disp_flush_ready(disp_drv);  // 通知LVGL刷新完成
// }

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
