#pragma once

#include <array>
#include <stdint.h>
#include <type_traits>

typedef uint8_t CRC8Value;
typedef uint16_t CRC16Value;
typedef uint32_t CRC32Value;

/* https://en.wikipedia.org/wiki/Cyclic_redundancy_check */
template<typename T>
struct CRCPolynomial; // deliberately undefined. 
template<>
struct CRCPolynomial<CRC8Value>
{
    static constexpr CRC8Value value = 0xD5;
};
template<>
struct CRCPolynomial<CRC16Value>
{
    static constexpr CRC16Value value = 0x1021;
};
template<>
struct CRCPolynomial<CRC32Value>
{
    static constexpr CRC32Value value = 0x04C11DB7;
};

//
// a naive implementation of a CRC.
// this class template can be used to generate 8, 16 and 32 bit CRCs
//
// example:
//    using CRC = TCRC<CRC8Value>;
//    CRC crc;
//    int data = 314;
//    crc.Update(data);
//    int result = crc;
//
template<typename CRCValue>
class TCRC final
{
private:
  static constexpr uint32_t width = 8 * sizeof(CRCValue);
  static constexpr uint32_t topbit = CRCValue{1} << (width - 1);;

  static constexpr CRCValue initialization_value = 42;
  static constexpr CRCValue finalization_value = 69;

  class CRCTable
  {
  public:
    CRCTable()
    {
      constexpr CRCValue polynomial = CRCPolynomial<CRCValue>::value;
      CRCValue remainder;

      // Compute the remainder of each possible dividend.
      for (uint32_t dividend = 0; dividend < 256; ++dividend)
      {
        remainder = dividend << (width - 8);
        for (int bit = 8; bit > 0; --bit)
        {
          if (remainder & topbit)
          {
              remainder = (remainder << 1) ^ polynomial;
          }
          else
          {
              remainder = (remainder << 1);
          }
        }
        table[dividend] = remainder;
      }
    }
    CRCValue operator[](uint8_t index) const
    {
      return table[index];
    }
  private:
    std::array<CRCValue,256> table;
  };

  inline static const CRCTable crcTable{};

public:
  TCRC()
    : remainder(initialization_value)
  {}
  operator CRCValue () const
  {
    return Final();
  }
  CRCValue Update(void const * input, size_t len)
  {
    uint8_t data;
    for (int byte = 0; byte < len; ++byte)
    {
        data = ((uint8_t const *)input)[byte] ^ (remainder >> (width - 8));
        remainder = crcTable[data] ^ (remainder << 8);
    }
    return Final();
  }
  template<typename T>
  CRCValue Update(const T input)
  {
    static_assert(std::is_trivial<T>::value && std::is_standard_layout<T>::value, "T must be POD");
    Update(&input, sizeof(input));
  }
private:
  CRCValue Final() const
  {
    return remainder ^ finalization_value;
  }
  CRCValue remainder;
};
