#pragma once

#include <array>
#include <string>

class MAC final
{
public:
    MAC() = default;  // all 0 mac, invalid

    MAC(const MAC & source);
    MAC(const uint8_t* source); // 6 bytes input
    MAC(const std::string & source); // 17 chars input, 01:02:03:04:05:06

    MAC& operator=(const MAC & source);
    MAC& operator=(const uint8_t* source);
    MAC& operator=(const std::string & source);

    bool operator==(const MAC& other) const;
    bool operator!=(const MAC& other) const;

    std::string ToString() const;

    const uint8_t *Data() const;
          uint8_t *Data();

    operator const uint8_t *() const { return Data(); };
    operator       uint8_t *()       { return Data(); };

    bool IsValid() const;

    struct HashFunction
    {
        size_t operator()(const MAC& mac) const;
    };

    static const MAC BroadCast;
private:
    std::array<uint8_t, 6> mac{};
};

template<> struct std::hash<MAC> 
{
    std::size_t operator()(MAC const& mac) const noexcept 
    {
        return MAC::HashFunction()(mac);
    }
};

inline std::string ToString(const MAC & mac)
{
    return mac.ToString();
}
