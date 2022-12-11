#pragma once

#include <cstdint>

struct RGBA {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a = 255;
};

struct RGBA_f {
    float r;
    float g;
    float b;
    float a = 255;
};
