#pragma once

#include <Wire.h>

template<const uint16_t TP_WIDTH = 800, const uint16_t TP_HEIGHT = 480, const int8_t TP_SDA = -1, const int8_t TP_SCL = -1, const uint8_t TP_ADDR = 0x5D, TwoWire &WIRE = Wire>
class GT911 {
public:
  bool begin();
  int8_t read(uint16_t *x, uint16_t *y, uint8_t count);
protected:
  int16_t readReg(uint16_t reg);
  bool readRegs(uint16_t reg, uint8_t *data, uint8_t count);
  bool writeReg(uint16_t reg, uint8_t value);
  bool writeRegs(uint16_t reg, const uint8_t *data, uint8_t count);
};

template<const uint16_t TP_WIDTH, const uint16_t TP_HEIGHT, const int8_t TP_SDA, const int8_t TP_SCL, const uint8_t TP_ADDR, TwoWire &WIRE>
bool GT911<TP_WIDTH, TP_HEIGHT, TP_SDA, TP_SCL, TP_ADDR, WIRE>::begin() {
  uint8_t cfg[186];
  uint8_t crc = 0;

  if (! WIRE.begin(TP_SDA, TP_SCL))
    return false;
  delay(50);
  if (! readRegs(0x8047, cfg, sizeof(cfg)))
    return false;
  cfg[1] = (uint8_t)TP_WIDTH;
  cfg[2] = TP_WIDTH >> 8;
  cfg[3] = (uint8_t)TP_HEIGHT;
  cfg[4] = TP_HEIGHT >> 8;
  for (uint8_t i = 0; i < sizeof(cfg) - 2; ++i) {
    crc += cfg[i];
  }
  crc = ~crc + 1;
  cfg[sizeof(cfg) - 2] = crc;
  cfg[sizeof(cfg) - 1] = 1;
  return writeRegs(0x8047, cfg, sizeof(cfg));
}

template<const uint16_t TP_WIDTH, const uint16_t TP_HEIGHT, const int8_t TP_SDA, const int8_t TP_SCL, const uint8_t TP_ADDR, TwoWire &WIRE>
int8_t GT911<TP_WIDTH, TP_HEIGHT, TP_SDA, TP_SCL, TP_ADDR, WIRE>::read(uint16_t *x, uint16_t *y, uint8_t count) {
  int16_t info;
  int8_t result = 0;

  if ((info = readReg(0x814E)) == -1)
    return -1;
  if ((info & 0x8F) > 0x80) {
    for (uint8_t i = 0; (i < (info & 0x0F)) && (i < count); ++i) {
      uint8_t data[7];

      if (! readRegs(0x814F + i * 8, data, sizeof(data)))
        break;
      x[i] = TP_WIDTH - (data[1] + (data[2] << 8));
      y[i] = TP_HEIGHT - (data[3] + (data[4] << 8));
      ++result;
    }
  }
  if (info)
    writeReg(0x814E, 0);
  return result;
}

template<const uint16_t TP_WIDTH, const uint16_t TP_HEIGHT, const int8_t TP_SDA, const int8_t TP_SCL, const uint8_t TP_ADDR, TwoWire &WIRE>
int16_t GT911<TP_WIDTH, TP_HEIGHT, TP_SDA, TP_SCL, TP_ADDR, WIRE>::readReg(uint16_t reg) {
  WIRE.beginTransmission(TP_ADDR);
  WIRE.write((uint8_t)(reg >> 8));
  WIRE.write((uint8_t)reg);
  if (WIRE.endTransmission())
    return -1;
  if (WIRE.requestFrom(TP_ADDR, (uint8_t)1) != 1)
    return -1;
  return WIRE.read();
}

template<const uint16_t TP_WIDTH, const uint16_t TP_HEIGHT, const int8_t TP_SDA, const int8_t TP_SCL, const uint8_t TP_ADDR, TwoWire &WIRE>
bool GT911<TP_WIDTH, TP_HEIGHT, TP_SDA, TP_SCL, TP_ADDR, WIRE>::readRegs(uint16_t reg, uint8_t *data, uint8_t count) {
  WIRE.beginTransmission(TP_ADDR);
  WIRE.write((uint8_t)(reg >> 8));
  WIRE.write((uint8_t)reg);
  if (WIRE.endTransmission())
    return false;
  if ((WIRE.requestFrom(TP_ADDR, count) != count) || (WIRE.readBytes(data, count) != count))
    return false;
  return true;
}

template<const uint16_t TP_WIDTH, const uint16_t TP_HEIGHT, const int8_t TP_SDA, const int8_t TP_SCL, const uint8_t TP_ADDR, TwoWire &WIRE>
bool GT911<TP_WIDTH, TP_HEIGHT, TP_SDA, TP_SCL, TP_ADDR, WIRE>::writeReg(uint16_t reg, uint8_t value) {
  WIRE.beginTransmission(TP_ADDR);
  WIRE.write((uint8_t)(reg >> 8));
  WIRE.write((uint8_t)reg);
  WIRE.write(value);
  return WIRE.endTransmission() == 0;
}

template<const uint16_t TP_WIDTH, const uint16_t TP_HEIGHT, const int8_t TP_SDA, const int8_t TP_SCL, const uint8_t TP_ADDR, TwoWire &WIRE>
bool GT911<TP_WIDTH, TP_HEIGHT, TP_SDA, TP_SCL, TP_ADDR, WIRE>::writeRegs(uint16_t reg, const uint8_t *data, uint8_t count) {
  WIRE.beginTransmission(TP_ADDR);
  WIRE.write((uint8_t)(reg >> 8));
  WIRE.write((uint8_t)reg);
  WIRE.write(data, count);
  return WIRE.endTransmission() == 0;
}
