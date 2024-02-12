/**
 * Colours for LED Strip Library for ESP-IDF
 * 
 * @author Gabriel Thien 2024
*/
#pragma once

#include <stdint.h>

#define MAX_COLOURS 6
#define CHANNELS 3

typedef enum colour // Colours
{
    RED,     // 255, 0, 0
    GREEN,   // 0, 255, 0
    BLUE,    // 0, 0, 255
    YELLOW,  // 255, 255, 0
    AQUA,    // 0, 255, 255
    MAGENTA, // 255, 0, 255
} colour_t;

const uint8_t palette[MAX_COLOURS][CHANNELS] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 0},
    {0, 255, 255},
    {255, 0, 255},
};
