#ifndef PICO_SHARP_HPP
#define PICO_SHARP_HPP
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "vector2.hpp"
#include <vector>

#include <iostream>

class PicoSharp
{
public:
    PicoSharp(int width, int height, spi_inst_t* spi_port, int CLK, int MOSI, int CS, int baudrate = 1000000);
    ~PicoSharp();
    void Init();  
    void FullRefresh();
    void DrawPixel(int x, int y, int color);
    void ClearBuffer();
    void drawSprite(std::vector<unsigned char>& spriteData, Vector2 position);

private:
    int width;
    int height;
    int clk_pin;
    int mosi_pin;
    int cs_pin;
    int baudrate;
    spi_inst_t* spi_port;
    uint8_t *buffer = NULL;   
    inline uint8_t GetWriteCommand();

    uint8_t vcom_command = 0x00;
};
#endif

