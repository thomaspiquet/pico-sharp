#include "pico_sharp.hpp"

PicoSharp::PicoSharp(int width, int height, spi_inst_t* spi_port, int clk_pin, int mosi_pin, int cs_pin, int baudrate)
{
    this->width = width;
    this->height = height;
    this->spi_port = spi_port;
    this->clk_pin = clk_pin;
    this->mosi_pin = mosi_pin;
    this->cs_pin = cs_pin;
    this->baudrate = baudrate;
}

PicoSharp::~PicoSharp()
{ 
}

void PicoSharp::Init()
{
    this->vcom_command = 0x00;
    // Build buffer with already processed line address
    const uint8_t bytesPerLine = (this->width / 8 + 2);
    this->buffer = (uint8_t *)malloc(bytesPerLine * this->height + 2);
    
    this->buffer[0] = this->vcom_command;
    this->buffer[12481] = 0x00;

    for (int iY = 0; iY < this->height; ++iY)
    {
      // Start at 1
      uint8_t lineAddr = iY + 1;
      // Bit reversal
      lineAddr = (lineAddr & 0xF0) >> 4 | (lineAddr & 0x0F) << 4;
      lineAddr = (lineAddr & 0xCC) >> 2 | (lineAddr & 0x33) << 2;
      lineAddr = (lineAddr & 0xAA) >> 1 | (lineAddr & 0x55) << 1;
      // Line Address
      this->buffer[1 + bytesPerLine * iY] = lineAddr;
      // End trail
      this->buffer[1 + (bytesPerLine -1) + bytesPerLine * iY] = 0b00000000;
      // Data
      for (int iX = 1; iX < bytesPerLine; ++iX)
      {
          this->buffer[1 + iX + bytesPerLine * iY] = 0b11111111;
      }
    }

    stdio_init_all();
    spi_init(this->spi_port, this->baudrate);

    gpio_set_function(this->clk_pin, GPIO_FUNC_SPI);
    gpio_set_function(this->mosi_pin, GPIO_FUNC_SPI);

    gpio_init(this->cs_pin);
    gpio_set_dir(this->cs_pin, GPIO_OUT);

    gpio_put(this->cs_pin, 0);
}

void PicoSharp::FullRefresh()
{
    // Prepare write command
    this->buffer[0] = this->GetWriteCommand();

    // Select display
    gpio_put(this->cs_pin, 1);

    // Load buffer in display
    spi_write_blocking(this->spi_port, this->buffer, ((this->width / 8) + 2) * this->height + 2);

    // Release display
    gpio_put(this->cs_pin, 0);
}

void PicoSharp::DrawPixel(int x, int y, int color)
{
    if (x >= 0 && x < this->width && y >= 0 && y < this->height)
    {
        if (color >= 0)
        {
            this->buffer[(2 + (int)(x / 8)) + (int)((this->width / 8) + 2) * y] ^= 
            (-color ^ this->buffer[(2 + (int)(x / 8)) + (int)((this->width / 8) + 2) * y]) & (1 << ( 7 - (x % 8)));
        }   
    }
}

void PicoSharp::ClearBuffer()
{
    const uint8_t bytesPerLine = (this->width / 8 + 2);
    for (int iY = 0; iY < this->height; ++iY)
    {
      for (int iX = 1; iX < bytesPerLine; ++iX)
      {
          this->buffer[1 + iX + bytesPerLine * iY] = 0b11111111;
      }
    }
}

inline uint8_t PicoSharp::GetWriteCommand()
{
    if (this->vcom_command == 0b10000000)
    {
        this->vcom_command = 0b11000000;
    }
    else if (this->vcom_command == 0b11000000)
    {
        this->vcom_command = 0b10000000;
    }
    else
    {
        this->vcom_command = 0b10000000;
    }
    return this->vcom_command;
}

void PicoSharp::drawSprite(std::vector<unsigned char>& spriteData, Vector2 position)
{
    for (int iY = 0; iY < spriteData[1]; iY++)
    {
      for (int iX = 0; iX < spriteData[0]; iX += 4)
      {
        this->DrawPixel(position.x + iX, position.y + iY, ((spriteData[iX / 4 + (spriteData[0] / 4) * iY + 2] & 0b00000011)) -1);
        this->DrawPixel(position.x + iX + 1, position.y + iY, ((spriteData[iX / 4 + (spriteData[0] / 4) * iY + 2] & 0b00001100) >> 2) -1);
        this->DrawPixel(position.x + iX + 2, position.y + iY, ((spriteData[iX / 4 + (spriteData[0] / 4) * iY + 2] & 0b00110000) >> 4) -1);
        this->DrawPixel(position.x + iX + 3, position.y + iY, ((spriteData[iX / 4 + (spriteData[0] / 4) * iY + 2] & 0b11000000) >> 6) -1);
      }
    }
}