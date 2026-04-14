/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)
/----------------------------------------------------------------------------*/
#pragma once

#include "lgfx/v1/panel/Panel_HasBuffer.hpp"
#include "lgfx/v1/misc/range.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_EPD_ED2208 : public Panel_HasBuffer
  {
    Panel_EPD_ED2208(void);

    bool init(bool use_reset) override;

    color_depth_t setColorDepth(color_depth_t depth) override;

    void setInvert(bool invert) override;
    void setSleep(bool flg) override;
    void setPowerSave(bool flg) override;

    void waitDisplay(void) override;
    bool displayBusy(void) override;
    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
    void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override;

    uint32_t readCommand(uint_fast16_t, uint_fast8_t, uint_fast8_t) override { return 0; }
    uint32_t readData(uint_fast8_t, uint_fast8_t) override { return 0; }

    void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override;

    // EPD color indices
    static constexpr uint8_t EPD_BLACK  = 0x0;
    static constexpr uint8_t EPD_WHITE  = 0x1;
    static constexpr uint8_t EPD_YELLOW = 0x2;
    static constexpr uint8_t EPD_RED    = 0x3;
    static constexpr uint8_t EPD_BLUE   = 0x5;
    static constexpr uint8_t EPD_GREEN  = 0x6;

  private:

    static constexpr unsigned long _refresh_msec = 1000;

    range_rect_t _range_old;
    unsigned long _send_msec = 0;

    size_t _get_buffer_length(void) const override;

    bool _wait_busy(uint32_t timeout = 60000);
    void _draw_pixel(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor);
    uint8_t _read_pixel(uint_fast16_t x, uint_fast16_t y);
    void _update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye);
    void _exec_transfer(void);
    void _turn_on_display(void);
    void _send_command(uint8_t cmd);
    void _send_data(uint8_t data);
    void _init_sequence(void);
    void _after_wake(void);

    static uint8_t _rgb_to_epd_color(int32_t r, int32_t g, int32_t b);

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
      static constexpr uint8_t list0[] = {
        0xAA,  6, 0x49, 0x55, 0x20, 0x08, 0x09, 0x18,  // CMDH
        0x01,  1, 0x3F,
        0x00,  2, 0x5F, 0x69,
        0x05,  4, 0x40, 0x1F, 0x1F, 0x2C,
        0x08,  4, 0x6F, 0x1F, 0x1F, 0x22,
        0x06,  4, 0x6F, 0x1F, 0x17, 0x17,
        0x03,  4, 0x03, 0x54, 0x00, 0x44,

        0x60,  2, 0x02, 0x00,
        0x30,  1, 0x08,
        0x50,  1, 0x3F,

        0xE3,  1, 0x2F,
        0x84,  1, 0x01,
        0xFF, 0xFF, // end
      };
      switch (listno) {
      case 0: return list0;
      default: return nullptr;
      }
    }
  };

//----------------------------------------------------------------------------
 }
}
