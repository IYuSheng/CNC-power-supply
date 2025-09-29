# Copyright 2025 NXP
# NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
# accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
# activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
# comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
# terms, then you may not retain, install, activate or otherwise use the software.

import SDL
import utime as time
import usys as sys
import lvgl as lv
import lodepng as png
import ustruct
import fs_driver

lv.init()
SDL.init(w=320,h=240)

# Register SDL display driver.
disp_buf1 = lv.disp_draw_buf_t()
buf1_1 = bytearray(320*240*4)
disp_buf1.init(buf1_1, None, len(buf1_1)//4)
disp_drv = lv.disp_drv_t()
disp_drv.init()
disp_drv.draw_buf = disp_buf1
disp_drv.flush_cb = SDL.monitor_flush
disp_drv.hor_res = 320
disp_drv.ver_res = 240
disp_drv.register()

# Regsiter SDL mouse driver
indev_drv = lv.indev_drv_t()
indev_drv.init()
indev_drv.type = lv.INDEV_TYPE.POINTER
indev_drv.read_cb = SDL.mouse_read
indev_drv.register()

fs_drv = lv.fs_drv_t()
fs_driver.fs_register(fs_drv, 'Z')

# Below: Taken from https://github.com/lvgl/lv_binding_micropython/blob/master/driver/js/imagetools.py#L22-L94

COLOR_SIZE = lv.color_t.__SIZE__
COLOR_IS_SWAPPED = hasattr(lv.color_t().ch,'green_h')

class lodepng_error(RuntimeError):
    def __init__(self, err):
        if type(err) is int:
            super().__init__(png.error_text(err))
        else:
            super().__init__(err)

# Parse PNG file header
# Taken from https://github.com/shibukawa/imagesize_py/blob/ffef30c1a4715c5acf90e8945ceb77f4a2ed2d45/imagesize.py#L63-L85

def get_png_info(decoder, src, header):
    # Only handle variable image types

    if lv.img.src_get_type(src) != lv.img.SRC.VARIABLE:
        return lv.RES.INV

    data = lv.img_dsc_t.__cast__(src).data
    if data == None:
        return lv.RES.INV

    png_header = bytes(data.__dereference__(24))

    if png_header.startswith(b'\211PNG\r\n\032\n'):
        if png_header[12:16] == b'IHDR':
            start = 16
        # Maybe this is for an older PNG version.
        else:
            start = 8
        try:
            width, height = ustruct.unpack(">LL", png_header[start:start+8])
        except ustruct.error:
            return lv.RES.INV
    else:
        return lv.RES.INV

    header.always_zero = 0
    header.w = width
    header.h = height
    header.cf = lv.img.CF.TRUE_COLOR_ALPHA

    return lv.RES.OK

def convert_rgba8888_to_bgra8888(img_view):
    for i in range(0, len(img_view), lv.color_t.__SIZE__):
        ch = lv.color_t.__cast__(img_view[i:i]).ch
        ch.red, ch.blue = ch.blue, ch.red

# Read and parse PNG file

def open_png(decoder, dsc):
    img_dsc = lv.img_dsc_t.__cast__(dsc.src)
    png_data = img_dsc.data
    png_size = img_dsc.data_size
    png_decoded = png.C_Pointer()
    png_width = png.C_Pointer()
    png_height = png.C_Pointer()
    error = png.decode32(png_decoded, png_width, png_height, png_data, png_size)
    if error:
        raise lodepng_error(error)
    img_size = png_width.int_val * png_height.int_val * 4
    img_data = png_decoded.ptr_val
    img_view = img_data.__dereference__(img_size)

    if COLOR_SIZE == 4:
        convert_rgba8888_to_bgra8888(img_view)
    else:
        raise lodepng_error("Error: Color mode not supported yet!")

    dsc.img_data = img_data
    return lv.RES.OK

# Above: Taken from https://github.com/lvgl/lv_binding_micropython/blob/master/driver/js/imagetools.py#L22-L94

decoder = lv.img.decoder_create()
decoder.info_cb = get_png_info
decoder.open_cb = open_png

def anim_x_cb(obj, v):
    obj.set_x(v)

def anim_y_cb(obj, v):
    obj.set_y(v)

def anim_width_cb(obj, v):
    obj.set_width(v)

def anim_height_cb(obj, v):
    obj.set_height(v)

def anim_img_zoom_cb(obj, v):
    obj.set_zoom(v)

def anim_img_rotate_cb(obj, v):
    obj.set_angle(v)

global_font_cache = {}
def test_font(font_family, font_size):
    global global_font_cache
    if font_family + str(font_size) in global_font_cache:
        return global_font_cache[font_family + str(font_size)]
    if font_size % 2:
        candidates = [
            (font_family, font_size),
            (font_family, font_size-font_size%2),
            (font_family, font_size+font_size%2),
            ("montserrat", font_size-font_size%2),
            ("montserrat", font_size+font_size%2),
            ("montserrat", 16)
        ]
    else:
        candidates = [
            (font_family, font_size),
            ("montserrat", font_size),
            ("montserrat", 16)
        ]
    for (family, size) in candidates:
        try:
            if eval(f'lv.font_{family}_{size}'):
                global_font_cache[font_family + str(font_size)] = eval(f'lv.font_{family}_{size}')
                if family != font_family or size != font_size:
                    print(f'WARNING: lv.font_{family}_{size} is used!')
                return eval(f'lv.font_{family}_{size}')
        except AttributeError:
            try:
                load_font = lv.font_load(f"Z:MicroPython/lv_font_{family}_{size}.fnt")
                global_font_cache[font_family + str(font_size)] = load_font
                return load_font
            except:
                if family == font_family and size == font_size:
                    print(f'WARNING: lv.font_{family}_{size} is NOT supported!')

global_image_cache = {}
def load_image(file):
    global global_image_cache
    if file in global_image_cache:
        return global_image_cache[file]
    try:
        with open(file,'rb') as f:
            data = f.read()
    except:
        print(f'Could not open {file}')
        sys.exit()

    img = lv.img_dsc_t({
        'data_size': len(data),
        'data': data
    })
    global_image_cache[file] = img
    return img

def calendar_event_handler(e,obj):
    code = e.get_code()

    if code == lv.EVENT.VALUE_CHANGED:
        source = e.get_current_target()
        date = lv.calendar_date_t()
        if source.get_pressed_date(date) == lv.RES.OK:
            source.set_highlighted_dates([date], 1)

def spinbox_increment_event_cb(e, obj):
    code = e.get_code()
    if code == lv.EVENT.SHORT_CLICKED or code == lv.EVENT.LONG_PRESSED_REPEAT:
        obj.increment()
def spinbox_decrement_event_cb(e, obj):
    code = e.get_code()
    if code == lv.EVENT.SHORT_CLICKED or code == lv.EVENT.LONG_PRESSED_REPEAT:
        obj.decrement()

def digital_clock_cb(timer, obj, current_time, show_second, use_ampm):
    hour = int(current_time[0])
    minute = int(current_time[1])
    second = int(current_time[2])
    ampm = current_time[3]
    second = second + 1
    if second == 60:
        second = 0
        minute = minute + 1
        if minute == 60:
            minute = 0
            hour = hour + 1
            if use_ampm:
                if hour == 12:
                    if ampm == 'AM':
                        ampm = 'PM'
                    elif ampm == 'PM':
                        ampm = 'AM'
                if hour > 12:
                    hour = hour % 12
    hour = hour % 24
    if use_ampm:
        if show_second:
            obj.set_text("%d:%02d:%02d %s" %(hour, minute, second, ampm))
        else:
            obj.set_text("%d:%02d %s" %(hour, minute, ampm))
    else:
        if show_second:
            obj.set_text("%d:%02d:%02d" %(hour, minute, second))
        else:
            obj.set_text("%d:%02d" %(hour, minute))
    current_time[0] = hour
    current_time[1] = minute
    current_time[2] = second
    current_time[3] = ampm

def analog_clock_cb(timer, obj):
    datetime = time.localtime()
    hour = datetime[3]
    if hour >= 12: hour = hour - 12
    obj.set_time(hour, datetime[4], datetime[5])

def datetext_event_handler(e, obj):
    code = e.get_code()
    target = e.get_target()
    if code == lv.EVENT.FOCUSED:
        if obj is None:
            bg = lv.layer_top()
            bg.add_flag(lv.obj.FLAG.CLICKABLE)
            obj = lv.calendar(bg)
            scr = target.get_screen()
            scr_height = scr.get_height()
            scr_width = scr.get_width()
            obj.set_size(int(scr_width * 0.8), int(scr_height * 0.8))
            datestring = target.get_text()
            year = int(datestring.split('/')[0])
            month = int(datestring.split('/')[1])
            day = int(datestring.split('/')[2])
            obj.set_showed_date(year, month)
            highlighted_days=[lv.calendar_date_t({'year':year, 'month':month, 'day':day})]
            obj.set_highlighted_dates(highlighted_days, 1)
            obj.align(lv.ALIGN.CENTER, 0, 0)
            lv.calendar_header_arrow(obj)
            obj.add_event_cb(lambda e: datetext_calendar_event_handler(e, target), lv.EVENT.ALL, None)
            scr.update_layout()

def datetext_calendar_event_handler(e, obj):
    code = e.get_code()
    target = e.get_current_target()
    if code == lv.EVENT.VALUE_CHANGED:
        date = lv.calendar_date_t()
        if target.get_pressed_date(date) == lv.RES.OK:
            obj.set_text(f"{date.year}/{date.month}/{date.day}")
            bg = lv.layer_top()
            bg.clear_flag(lv.obj.FLAG.CLICKABLE)
            bg.set_style_bg_opa(lv.OPA.TRANSP, 0)
            target.delete()

# Create screen
screen = lv.obj()
screen.set_size(320, 240)
screen.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_cont_master
screen_cont_master = lv.obj(screen)
screen_cont_master.set_pos(0, 0)
screen_cont_master.set_size(320, 240)
screen_cont_master.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_cont_master, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_cont_master.set_style_border_width(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_border_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_border_color(lv.color_hex(0x2195f6), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_border_side(lv.BORDER_SIDE.FULL, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_bg_color(lv.color_hex(0x000000), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_cont_master.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_voltage
screen_label_voltage = lv.label(screen)
screen_label_voltage.set_text("\n\n                V")
screen_label_voltage.set_long_mode(lv.label.LONG.WRAP)
screen_label_voltage.set_width(lv.pct(100))
screen_label_voltage.set_pos(5, 135)
screen_label_voltage.set_size(100, 100)
# Set style for screen_label_voltage, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_voltage.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_radius(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_text_font(test_font("montserratMedium", 18), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_bg_opa(179, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_bg_color(lv.color_hex(0x008b32), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_shadow_width(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_shadow_color(lv.color_hex(0x042a00), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_shadow_opa(188, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_shadow_spread(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_shadow_ofs_x(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_voltage.set_style_shadow_ofs_y(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Current
screen_label_Current = lv.label(screen)
screen_label_Current.set_text("\n\n                A")
screen_label_Current.set_long_mode(lv.label.LONG.WRAP)
screen_label_Current.set_width(lv.pct(100))
screen_label_Current.set_pos(110, 135)
screen_label_Current.set_size(100, 100)
# Set style for screen_label_Current, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Current.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_radius(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_text_font(test_font("montserratMedium", 18), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_bg_opa(216, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_bg_color(lv.color_hex(0xb0a20f), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_shadow_width(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_shadow_color(lv.color_hex(0x383d00), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_shadow_opa(181, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_shadow_spread(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_shadow_ofs_x(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current.set_style_shadow_ofs_y(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_power
screen_label_power = lv.label(screen)
screen_label_power.set_text("\n0.000 W")
screen_label_power.set_long_mode(lv.label.LONG.WRAP)
screen_label_power.set_width(lv.pct(100))
screen_label_power.set_pos(215, 135)
screen_label_power.set_size(100, 48)
# Set style for screen_label_power, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_power.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_radius(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_text_font(test_font("montserratMedium", 17), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_text_line_space(7, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_bg_opa(153, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_bg_color(lv.color_hex(0x05e8f2), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_shadow_width(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_shadow_color(lv.color_hex(0x213a3d), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_shadow_opa(179, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_shadow_spread(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_shadow_ofs_x(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_power.set_style_shadow_ofs_y(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_btn_Voltage
screen_btn_Voltage = lv.btn(screen)
screen_btn_Voltage_label = lv.label(screen_btn_Voltage)
screen_btn_Voltage_label.set_text("电压")
screen_btn_Voltage_label.set_long_mode(lv.label.LONG.CLIP)
screen_btn_Voltage_label.set_width(lv.pct(100))
screen_btn_Voltage_label.align(lv.ALIGN.CENTER, 0, 0)
screen_btn_Voltage.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_Voltage.set_pos(7, 137)
screen_btn_Voltage.set_size(96, 20)
# Set style for screen_btn_Voltage, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_btn_Voltage.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_bg_color(lv.color_hex(0x128505), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_radius(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_shadow_width(3, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_shadow_color(lv.color_hex(0x064501), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_shadow_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_shadow_spread(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_shadow_ofs_x(1, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_shadow_ofs_y(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_text_font(test_font("SourceHanSerifSC_Regular", 18), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Voltage.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_btn_Current
screen_btn_Current = lv.btn(screen)
screen_btn_Current_label = lv.label(screen_btn_Current)
screen_btn_Current_label.set_text("电流")
screen_btn_Current_label.set_long_mode(lv.label.LONG.CLIP)
screen_btn_Current_label.set_width(lv.pct(100))
screen_btn_Current_label.align(lv.ALIGN.CENTER, 0, 0)
screen_btn_Current.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_Current.set_pos(112, 137)
screen_btn_Current.set_size(96, 20)
# Set style for screen_btn_Current, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_btn_Current.set_style_bg_opa(198, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_bg_color(lv.color_hex(0xb7aa26), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_radius(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_shadow_width(3, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_shadow_color(lv.color_hex(0x666008), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_shadow_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_shadow_spread(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_shadow_ofs_x(1, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_shadow_ofs_y(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_text_font(test_font("SourceHanSerifSC_Regular", 18), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Current.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_btn_Power
screen_btn_Power = lv.btn(screen)
screen_btn_Power_label = lv.label(screen_btn_Power)
screen_btn_Power_label.set_text("功率")
screen_btn_Power_label.set_long_mode(lv.label.LONG.CLIP)
screen_btn_Power_label.set_width(lv.pct(100))
screen_btn_Power_label.align(lv.ALIGN.CENTER, 0, 0)
screen_btn_Power.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_Power.set_pos(217, 137)
screen_btn_Power.set_size(96, 18)
# Set style for screen_btn_Power, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_btn_Power.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_bg_color(lv.color_hex(0x069dad), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_radius(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_shadow_width(3, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_shadow_color(lv.color_hex(0x10505b), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_shadow_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_shadow_spread(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_shadow_ofs_x(1, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_shadow_ofs_y(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_text_font(test_font("SourceHanSerifSC_Regular", 18), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_Power.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Voltage_now
screen_label_Voltage_now = lv.label(screen)
screen_label_Voltage_now.set_text("0.000")
screen_label_Voltage_now.set_long_mode(lv.label.LONG.WRAP)
screen_label_Voltage_now.set_width(lv.pct(100))
screen_label_Voltage_now.set_pos(10, 166)
screen_label_Voltage_now.set_size(76, 24)
# Set style for screen_label_Voltage_now, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Voltage_now.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_text_font(test_font("montserratMedium", 26), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_now.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Current_now
screen_label_Current_now = lv.label(screen)
screen_label_Current_now.set_text("0.000")
screen_label_Current_now.set_long_mode(lv.label.LONG.WRAP)
screen_label_Current_now.set_width(lv.pct(100))
screen_label_Current_now.set_pos(115, 166)
screen_label_Current_now.set_size(76, 25)
# Set style for screen_label_Current_now, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Current_now.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_text_font(test_font("montserratMedium", 26), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_now.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Voltage_Set
screen_label_Voltage_Set = lv.label(screen)
screen_label_Voltage_Set.set_text("0.000 V")
screen_label_Voltage_Set.set_long_mode(lv.label.LONG.WRAP)
screen_label_Voltage_Set.set_width(lv.pct(100))
screen_label_Voltage_Set.set_pos(50, 198)
screen_label_Voltage_Set.set_size(56, 15)
# Set style for screen_label_Voltage_Set, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Voltage_Set.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_text_font(test_font("montserratMedium", 12), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_text_opa(243, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Voltage_Set.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Current_Set
screen_label_Current_Set = lv.label(screen)
screen_label_Current_Set.set_text("0.000 A")
screen_label_Current_Set.set_long_mode(lv.label.LONG.WRAP)
screen_label_Current_Set.set_width(lv.pct(100))
screen_label_Current_Set.set_pos(155, 198)
screen_label_Current_Set.set_size(56, 15)
# Set style for screen_label_Current_Set, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Current_Set.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_text_font(test_font("montserratMedium", 12), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Current_Set.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Timeout
screen_label_Timeout = lv.label(screen)
screen_label_Timeout.set_text("00 : 00 : 00")
screen_label_Timeout.set_long_mode(lv.label.LONG.WRAP)
screen_label_Timeout.set_width(lv.pct(100))
screen_label_Timeout.set_pos(110, 5)
screen_label_Timeout.set_size(100, 20)
# Set style for screen_label_Timeout, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Timeout.set_style_border_width(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_border_opa(51, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_border_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_border_side(lv.BORDER_SIDE.FULL, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_radius(8, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_text_font(test_font("montserratMedium", 16), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_text_opa(226, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_bg_opa(69, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_bg_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_shadow_width(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_shadow_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_shadow_opa(34, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_shadow_spread(6, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_shadow_ofs_x(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Timeout.set_style_shadow_ofs_y(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Temperature
screen_label_Temperature = lv.label(screen)
screen_label_Temperature.set_text("25.3°C")
screen_label_Temperature.set_long_mode(lv.label.LONG.WRAP)
screen_label_Temperature.set_width(lv.pct(100))
screen_label_Temperature.set_pos(215, 5)
screen_label_Temperature.set_size(55, 20)
# Set style for screen_label_Temperature, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Temperature.set_style_border_width(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_border_opa(37, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_border_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_border_side(lv.BORDER_SIDE.FULL, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_radius(8, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_text_font(test_font("montserratMedium", 14), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_text_opa(213, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_text_letter_space(1, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_bg_opa(69, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_bg_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_shadow_width(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_shadow_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_shadow_opa(34, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_shadow_spread(6, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_shadow_ofs_x(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Temperature.set_style_shadow_ofs_y(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Start
screen_label_Start = lv.label(screen)
screen_label_Start.set_text("关闭")
screen_label_Start.set_long_mode(lv.label.LONG.WRAP)
screen_label_Start.set_width(lv.pct(100))
screen_label_Start.set_pos(274, 5)
screen_label_Start.set_size(40, 20)
# Set style for screen_label_Start, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Start.set_style_border_width(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_border_opa(37, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_border_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_border_side(lv.BORDER_SIDE.FULL, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_radius(8, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_text_font(test_font("SourceHanSerifSC_Regular", 14), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_text_opa(243, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_text_line_space(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_bg_opa(69, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_bg_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_shadow_width(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_shadow_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_shadow_opa(34, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_shadow_spread(6, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_shadow_ofs_x(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Start.set_style_shadow_ofs_y(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_mode
screen_label_mode = lv.label(screen)
screen_label_mode.set_text("电压环模式")
screen_label_mode.set_long_mode(lv.label.LONG.WRAP)
screen_label_mode.set_width(lv.pct(100))
screen_label_mode.set_pos(5, 5)
screen_label_mode.set_size(100, 20)
# Set style for screen_label_mode, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_mode.set_style_border_width(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_border_opa(37, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_border_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_border_side(lv.BORDER_SIDE.FULL, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_radius(8, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_text_font(test_font("SourceHanSerifSC_Regular", 13), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_text_opa(243, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_text_line_space(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_bg_opa(69, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_bg_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_shadow_width(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_shadow_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_shadow_opa(34, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_shadow_spread(6, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_shadow_ofs_x(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_mode.set_style_shadow_ofs_y(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_main
screen_label_main = lv.label(screen)
screen_label_main.set_text("0.000")
screen_label_main.set_long_mode(lv.label.LONG.WRAP)
screen_label_main.set_width(lv.pct(100))
screen_label_main.set_pos(10, 43)
screen_label_main.set_size(216, 69)
# Set style for screen_label_main, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_main.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_text_font(test_font("montserratMedium", 73), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_main.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_select_unit
screen_label_select_unit = lv.label(screen)
screen_label_select_unit.set_text("V")
screen_label_select_unit.set_long_mode(lv.label.LONG.WRAP)
screen_label_select_unit.set_width(lv.pct(100))
screen_label_select_unit.set_pos(191, 87)
screen_label_select_unit.set_size(100, 32)
# Set style for screen_label_select_unit, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_select_unit.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_text_font(test_font("montserratMedium", 28), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_select_unit.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_in1
screen_label_in1 = lv.label(screen)
screen_label_in1.set_text("输入:")
screen_label_in1.set_long_mode(lv.label.LONG.WRAP)
screen_label_in1.set_width(lv.pct(100))
screen_label_in1.set_pos(19, 212)
screen_label_in1.set_size(56, 15)
# Set style for screen_label_in1, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_in1.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_text_font(test_font("SourceHanSerifSC_Regular", 12), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_text_opa(231, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in1.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_in2
screen_label_in2 = lv.label(screen)
screen_label_in2.set_text("输入:")
screen_label_in2.set_long_mode(lv.label.LONG.WRAP)
screen_label_in2.set_width(lv.pct(100))
screen_label_in2.set_pos(124, 212)
screen_label_in2.set_size(45, 14)
# Set style for screen_label_in2, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_in2.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_text_font(test_font("SourceHanSerifSC_Regular", 12), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in2.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_in_v
screen_label_in_v = lv.label(screen)
screen_label_in_v.set_text("0.000 V")
screen_label_in_v.set_long_mode(lv.label.LONG.WRAP)
screen_label_in_v.set_width(lv.pct(100))
screen_label_in_v.set_pos(50, 213)
screen_label_in_v.set_size(56, 15)
# Set style for screen_label_in_v, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_in_v.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_text_font(test_font("montserratMedium", 12), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_text_opa(243, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_v.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_in_a
screen_label_in_a = lv.label(screen)
screen_label_in_a.set_text("0.000 A")
screen_label_in_a.set_long_mode(lv.label.LONG.WRAP)
screen_label_in_a.set_width(lv.pct(100))
screen_label_in_a.set_pos(155, 213)
screen_label_in_a.set_size(56, 15)
# Set style for screen_label_in_a, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_in_a.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_text_font(test_font("montserratMedium", 12), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_in_a.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_set_v
screen_label_set_v = lv.label(screen)
screen_label_set_v.set_text("设置:")
screen_label_set_v.set_long_mode(lv.label.LONG.WRAP)
screen_label_set_v.set_width(lv.pct(100))
screen_label_set_v.set_pos(19, 198)
screen_label_set_v.set_size(56, 15)
# Set style for screen_label_set_v, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_set_v.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_text_font(test_font("SourceHanSerifSC_Regular", 12), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_text_opa(251, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_v.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_set_a
screen_label_set_a = lv.label(screen)
screen_label_set_a.set_text("设置:")
screen_label_set_a.set_long_mode(lv.label.LONG.WRAP)
screen_label_set_a.set_width(lv.pct(100))
screen_label_set_a.set_pos(124, 198)
screen_label_set_a.set_size(56, 15)
# Set style for screen_label_set_a, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_set_a.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_text_font(test_font("SourceHanSerifSC_Regular", 12), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_set_a.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_energy
screen_label_energy = lv.label(screen)
screen_label_energy.set_text("\n 0.00 mAh")
screen_label_energy.set_long_mode(lv.label.LONG.WRAP)
screen_label_energy.set_width(lv.pct(100))
screen_label_energy.set_pos(215, 186)
screen_label_energy.set_size(100, 48)
# Set style for screen_label_energy, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_energy.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_radius(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_text_font(test_font("montserratMedium", 17), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_text_opa(251, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_text_letter_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_text_line_space(7, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_bg_opa(194, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_bg_color(lv.color_hex(0xa40242), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_shadow_width(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_shadow_color(lv.color_hex(0x350d1f), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_shadow_opa(134, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_shadow_spread(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_shadow_ofs_x(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_energy.set_style_shadow_ofs_y(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_btn_energy
screen_btn_energy = lv.btn(screen)
screen_btn_energy_label = lv.label(screen_btn_energy)
screen_btn_energy_label.set_text("能量")
screen_btn_energy_label.set_long_mode(lv.label.LONG.CLIP)
screen_btn_energy_label.set_width(lv.pct(100))
screen_btn_energy_label.align(lv.ALIGN.CENTER, 0, 0)
screen_btn_energy.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_energy.set_pos(217, 188)
screen_btn_energy.set_size(96, 18)
# Set style for screen_btn_energy, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_btn_energy.set_style_bg_opa(183, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_bg_color(lv.color_hex(0x940140), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_bg_grad_dir(lv.GRAD_DIR.NONE, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_radius(10, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_shadow_width(3, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_shadow_color(lv.color_hex(0x24000e), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_shadow_opa(220, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_shadow_spread(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_shadow_ofs_x(1, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_shadow_ofs_y(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_text_font(test_font("SourceHanSerifSC_Regular", 16), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_text_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_energy.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)

screen.update_layout()

# content from custom.py

# Load the default screen
lv.scr_load(screen)

while SDL.check():
    time.sleep_ms(5)

