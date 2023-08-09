#include <Arduino.h>
#include "hmi.h"

#define TS_SDA  19
#define TS_SCL  20

Arduino_ESP32RGBPanel *panel;
Arduino_RGB_Display *gfx;
GT911<800, 480, TS_SDA, TS_SCL> *ts;

static void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t*)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t*)&color_p->full, w, h);
#endif
  lv_disp_flush_ready(disp);
}

static void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t x, y;

  if (ts->read(&x, &y, 1) == 1) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = map(x, 800, 0, 0, gfx->width() - 1);
    data->point.y = map(y, 480, 0, 0, gfx->height() - 1);
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

bool tftInit() {
  panel = new Arduino_ESP32RGBPanel(
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 0 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */,
    0 /* hsync_polarity */, 210 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 43 /* hsync_back_porch */,
    0 /* vsync_polarity */, 22 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 12 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 16000000 /* prefer_speed */);
  if (! panel)
    return false;
  gfx = new Arduino_RGB_Display(800, 480, panel);
  if (! gfx) {
    delete panel;
    return false;
  }
  if (! gfx->begin()) {
    delete gfx;
    delete panel;
    return false;
  }
  gfx->fillScreen(BLACK);
#ifdef GFX_BL
//  pinMode(GFX_BL, OUTPUT);
//  digitalWrite(GFX_BL, HIGH);
  analogWrite(GFX_BL, 128);
#endif
  return true;
}

bool tsInit() {
  ts = new GT911<800, 480, TS_SDA, TS_SCL>();
  if (! ts)
    return false;
  if (! ts->begin()) {
    delete ts;
    return false;
  }
  return true;
}

bool lvglInit() {
  static lv_disp_draw_buf_t draw_buf;
  static lv_disp_drv_t disp_drv;
  static lv_indev_drv_t indev_drv;

  lv_color_t *disp_draw_buf;
  uint16_t screenWidth, screenHeight;

  lv_init();
  screenWidth = gfx->width();
  screenHeight = gfx->height();
#ifdef ESP32
  disp_draw_buf = (lv_color_t*)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * 40, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#else
  disp_draw_buf = (lv_color_t*)malloc(sizeof(lv_color_t) * screenWidth * 40);
#endif
  if (! disp_draw_buf)
    return false;
  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * 40);
  /* Initialize the display */
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  /* Initialize the touch screen input device driver */
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);
  return true;
}
