#include <Arduino.h>
#include "hmi.h"
//#include "ui.h"

static void halt(const char *msg) {
  Serial.println(msg);
  Serial.flush();
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);

  if (! tftInit())
    halt("TFT init fail!");
  if (! tsInit())
    halt("Touch init fail!");
  if (! lvglInit())
    halt("LVGL init fail!");

//  ui_init();
}

void loop() {
//  lv_timer_handler();
  delay(5);
}
