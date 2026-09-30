#pragma once
#include <array>
#include <map>
#include <stdint.h>
struct font_data_struct
{
  uint8_t width;
  uint8_t height;
  int8_t offset_x;
  int8_t offset_y;
  int8_t advance;
  const uint8_t * data;
};
extern std::map<char,font_data_struct> font_data;