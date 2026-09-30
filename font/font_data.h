#pragma once
#include <array>
#include <map>
struct font_data_struct = 
{
  uint8_t width;
  uint8_t height;
  int8_t offset_x;
  int8_t offset_y;
  int8_t advance;
  std::array<uint32_t,32> data;
};
extern std::map<char,font_data_struct> font_data;