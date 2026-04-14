/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)
/----------------------------------------------------------------------------*/
#include "Panel_EPD_ED2208.hpp"
#include "lgfx/v1/Bus.hpp"
#include "lgfx/v1/platforms/common.hpp"
#include "lgfx/v1/misc/pixelcopy.hpp"
#include "lgfx/v1/misc/colortype.hpp"

#ifdef min
#undef min
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  // EPD color palette in RGB888
  static constexpr struct { uint8_t r, g, b, idx; } epd_palette[] = {
    {   0,   0,   0, Panel_EPD_ED2208::EPD_BLACK  },
    { 255, 255, 255, Panel_EPD_ED2208::EPD_WHITE  },
    { 255, 255,   0, Panel_EPD_ED2208::EPD_YELLOW },
    { 255,   0,   0, Panel_EPD_ED2208::EPD_RED    },
    {   0,   0, 255, Panel_EPD_ED2208::EPD_BLUE   },
    {   0, 255,   0, Panel_EPD_ED2208::EPD_GREEN  },
  };

  // Reverse mapping: EPD color index -> RGB888
  static rgb888_t epd_color_to_rgb888(uint8_t epd_color)
  {
    switch (epd_color) {
      case Panel_EPD_ED2208::EPD_BLACK:  return 0x000000;
      case Panel_EPD_ED2208::EPD_WHITE:  return 0xFFFFFF;
      case Panel_EPD_ED2208::EPD_YELLOW: return 0xFFFF00;
      case Panel_EPD_ED2208::EPD_RED:    return 0xFF0000;
      case Panel_EPD_ED2208::EPD_BLUE:   return 0x0000FF;
      case Panel_EPD_ED2208::EPD_GREEN:  return 0x00FF00;
      default:                           return 0xFFFFFF;
    }
  }


  Panel_EPD_ED2208::Panel_EPD_ED2208(void)
  {
    _cfg.dummy_read_bits = 0;
    _epd_mode = epd_mode_t::epd_quality;
  }

  color_depth_t Panel_EPD_ED2208::setColorDepth(color_depth_t depth)
  {
    (void)depth;
    _write_depth = color_depth_t::rgb888_nonswapped;
    _read_depth = color_depth_t::rgb888_nonswapped;
    return color_depth_t::rgb888_nonswapped;
  }

  size_t Panel_EPD_ED2208::_get_buffer_length(void) const
  {
    // 4 bits per pixel, 2 pixels per byte
    // width/2 bytes per row * height rows
    return ((_cfg.panel_width+1)>>1) * _cfg.panel_height;
  }

  uint8_t Panel_EPD_ED2208::_rgb_to_epd_color(int32_t r, int32_t g, int32_t b)
  {
    uint32_t min_dist = UINT32_MAX;
    uint8_t best = EPD_WHITE;
    for (const auto& p : epd_palette) {
      int32_t dr = r - (int32_t)p.r;
      int32_t dg = g - (int32_t)p.g;
      int32_t db = b - (int32_t)p.b;
      uint32_t dist = dr * dr + dg * dg + db * db;
      if (dist < min_dist) {
        min_dist = dist;
        best = p.idx;
      }
    }
    return best;
  }

  void Panel_EPD_ED2208::_send_command(uint8_t cmd)
  {
    _bus->writeCommand(cmd, 8);
  }

  void Panel_EPD_ED2208::_send_data(uint8_t data)
  {
    _bus->writeData(data, 8);
  }

  bool Panel_EPD_ED2208::_wait_busy(uint32_t timeout)
  {
    _bus->wait();
    // EPD_4in0e: LOW=busy, HIGH=idle (inverted from GDEW panels)
    if (_cfg.pin_busy >= 0 && !gpio_in(_cfg.pin_busy))
    {
      uint32_t start_time = millis();
      do
      {
        if (millis() - start_time > timeout) {
          return false;
        }
        lgfx::delay(10);
      } while (!gpio_in(_cfg.pin_busy));
      lgfx::delay(200);
    }
    return true;
  }

  void Panel_EPD_ED2208::_init_sequence(void)
  {
    // Hardware reset
    rst_control(true);
    lgfx::delay(20);
    rst_control(false);
    lgfx::delay(2);
    rst_control(true);
    lgfx::delay(20);

    for (uint8_t i = 0; auto cmds = getInitCommands(i); i++)
    {
      _wait_busy();
      command_list(cmds);
    }
  
    _wait_busy();
    _send_command(0x61);    // Resolution
    _send_data((_cfg.panel_width >> 8) & 0xFF);
    _send_data(_cfg.panel_width & 0xFF);
    _send_data((_cfg.panel_height >> 8) & 0xFF);
    _send_data(_cfg.panel_height & 0xFF);
  }

  void Panel_EPD_ED2208::_turn_on_display(void)
  {
    _send_command(0x04);    // POWER_ON
    _wait_busy();
    lgfx::delay(200);

    // Booster setting (second stage)
    _send_command(0x06);
    _send_data(0x6F);
    _send_data(0x1F);
    _send_data(0x17);
    _send_data(0x27);
    lgfx::delay(200);

    _send_command(0x12);    // DISPLAY_REFRESH
    _send_data(0x00);
    _wait_busy();

    _send_command(0x02);    // POWER_OFF
    _send_data(0x00);
    _wait_busy();
    lgfx::delay(200);
  }

  bool Panel_EPD_ED2208::init(bool use_reset)
  {
    pinMode(_cfg.pin_busy, pin_mode_t::input_pullup);

    if (!Panel_HasBuffer::init(false))  // skip default reset, we do our own
    {
      return false;
    }

    // Fill buffer with white
    memset(_buf, (EPD_WHITE << 4) | EPD_WHITE, _get_buffer_length());
    _after_wake();

    return true;
  }

  void Panel_EPD_ED2208::_after_wake(void)
  {
    startWrite(true);
    _init_sequence();

    setRotation(_rotation);

    _range_old.top = 0;
    _range_old.left = 0;
    _range_old.right = _width - 1;
    _range_old.bottom = _height - 1;
    _range_mod.top    = INT16_MAX;
    _range_mod.left   = INT16_MAX;
    _range_mod.right  = 0;
    _range_mod.bottom = 0;

    endWrite();
  }

  void Panel_EPD_ED2208::waitDisplay(void)
  {
    _wait_busy();
  }

  bool Panel_EPD_ED2208::displayBusy(void)
  {
    // EPD_4in0e: LOW=busy
    return _cfg.pin_busy >= 0 && !gpio_in(_cfg.pin_busy);
  }

  void Panel_EPD_ED2208::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    if (0 < w && 0 < h)
    {
      _range_mod.left   = std::min<int16_t>(_range_mod.left  , x        );
      _range_mod.right  = std::max<int16_t>(_range_mod.right , x + w - 1);
      _range_mod.top    = std::min<int16_t>(_range_mod.top   , y        );
      _range_mod.bottom = std::max<int16_t>(_range_mod.bottom, y + h - 1);
    }
    if (_range_mod.empty()) { return; }

    _exec_transfer();
    _turn_on_display();
    _send_msec = millis();

    _range_old = _range_mod;
    _range_mod.top    = INT16_MAX;
    _range_mod.left   = INT16_MAX;
    _range_mod.right  = 0;
    _range_mod.bottom = 0;
  }

  void Panel_EPD_ED2208::_exec_transfer(void)
  {
    // EPD_4in0e always sends full frame (no partial addressing)
    uint32_t buf_len = _get_buffer_length();

    _send_command(0x10);    // Data start transmission
    _bus->writeBytes(_buf, buf_len, true, true);
  }

  void Panel_EPD_ED2208::setInvert(bool invert)
  {
    _invert = invert;
  }

  void Panel_EPD_ED2208::setSleep(bool flg)
  {
    if (flg)
    {
      startWrite();
      _send_command(0x07);  // DEEP_SLEEP
      _send_data(0xA5);
      endWrite();
    }
    else
    {
      _after_wake();
    }
  }

  void Panel_EPD_ED2208::setPowerSave(bool flg)
  {
    if (flg) {
      setSleep(true);
    }
  }
#if 1
  void Panel_EPD_ED2208::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _xs = xs;
    _ys = ys;
    _xe = xe;
    _ye = ye;
    _update_transferred_rect(xs, ys, xe, ye);

    rgb888_t color = { rawcolor };
    uint8_t epd_color[16];

    static constexpr int8_t tile_r[] = {-30,   2, -22,  10,  18, -14,  26, - 6, -18,  14, -26,   6,  30, - 2,  22, -10};
    static constexpr int8_t tile_g[] = { 26, - 6, -18,  14, -26,   6,  30, - 2,  22, -10, -30,   2, -22,  10,  18, -14};
    static constexpr int8_t tile_b[] = { 30, - 2,  22, -10, -30,   2, -22,  10,  18, -14,  26, - 6, -18,  14, -26,   6};

    for (int i = 0; i < 16; ++i) {
      int r = color.R8() + (tile_r[i] * 3);
      int g = color.G8() + (tile_g[i] * 3);
      int b = color.B8() + (tile_b[i] * 3);
      epd_color[i] = _rgb_to_epd_color(r, g, b);
    }

    uint32_t row_bytes = _cfg.panel_width / 2;

    y = ys;
    do
    {
      auto colors = &epd_color[(y&3)<<2];
      x = xs;
      do
      {
        uint32_t byte_idx = y * row_bytes + (x >> 1);
        auto base = _buf[byte_idx];
        switch (x & 3) {
        case 0: base = (base & 0x0F) | colors[0] << 4; break;
        case 1: base = (base & 0xF0) | colors[1]; break;
        case 2: base = (base & 0x0F) | colors[2] << 4; break;
        case 3: base = (base & 0xF0) | colors[3]; break;
        default: break;
        }
        _buf[byte_idx] = base;
      } while (++x <= xe);
    } while (++y <= ye);
  }
#else
  void Panel_EPD_ED2208::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _xs = xs;
    _ys = ys;
    _xe = xe;
    _ye = ye;
    _update_transferred_rect(xs, ys, xe, ye);

    rgb888_t color = { rawcolor };
    uint8_t epd_color = _rgb_to_epd_color(color.R8(), color.G8(), color.B8());

    uint32_t row_bytes = _cfg.panel_width / 2;

    y = ys;
    do
    {
      x = xs;
      do
      {
        uint32_t byte_idx = y * row_bytes + (x >> 1);
        if (x & 1) {
          _buf[byte_idx] = (_buf[byte_idx] & 0xF0) | epd_color;
        } else {
          _buf[byte_idx] = (_buf[byte_idx] & 0x0F) | (epd_color << 4);
        }
      } while (++x <= xe);
    } while (++y <= ye);
  }
#endif

  void Panel_EPD_ED2208::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _update_transferred_rect(xs, ys, xe, ye);

    auto readbuf = (rgb888_t*)alloca(w * sizeof(rgb888_t));
    auto sx = param->src_x32;
    h += y;
    do
    {
      uint32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          do
          {
            _draw_pixel(x + prev_pos, y, readbuf[prev_pos].get());
          } while (new_pos != ++prev_pos);
        }
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
    } while (++y < h);
  }

  void Panel_EPD_ED2208::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    {
      uint_fast16_t xs = _xs;
      uint_fast16_t xe = _xe;
      uint_fast16_t ys = _ys;
      uint_fast16_t ye = _ye;
      _update_transferred_rect(xs, ys, xe, ye);
    }
    uint_fast16_t xs   = _xs  ;
    uint_fast16_t ys   = _ys  ;
    uint_fast16_t xe   = _xe  ;
    uint_fast16_t ye   = _ye  ;
    uint_fast16_t xpos = _xpos;
    uint_fast16_t ypos = _ypos;

    static constexpr uint32_t buflen = 16;
    rgb888_t colors[buflen];
    int bufpos = buflen;
    do
    {
      if (bufpos == buflen) {
        param->fp_copy(colors, 0, std::min(length, buflen), param);
        bufpos = 0;
      }
      _draw_pixel(xpos, ypos, colors[bufpos++].get());
      if (++xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye)
        {
          ypos = ys;
        }
      }
    } while (--length);
    _xpos = xpos;
    _ypos = ypos;
  }

  void Panel_EPD_ED2208::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    auto readbuf = (rgb888_t*)alloca(w * sizeof(rgb888_t));
    param->src_data = readbuf;
    int32_t readpos = 0;
    h += y;
    do
    {
      uint32_t idx = 0;
      do
      {
        uint8_t epd_val = _read_pixel(x + idx, y);
        readbuf[idx] = epd_color_to_rgb888(epd_val);
      } while (++idx != w);
      param->src_x32 = 0;
      readpos = param->fp_copy(dst, readpos, readpos + w, param);
    } while (++y < h);
  }

  void Panel_EPD_ED2208::_draw_pixel(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    _rotate_pos(x, y);

    rgb888_t color = { rawcolor };
    uint8_t epd_color = _rgb_to_epd_color(color.R8(), color.G8(), color.B8());

    uint32_t row_bytes = _cfg.panel_width / 2;
    uint32_t byte_idx = y * row_bytes + (x >> 1);
    if (x & 1) {
      _buf[byte_idx] = (_buf[byte_idx] & 0xF0) | epd_color;
    } else {
      _buf[byte_idx] = (_buf[byte_idx] & 0x0F) | (epd_color << 4);
    }
  }

  uint8_t Panel_EPD_ED2208::_read_pixel(uint_fast16_t x, uint_fast16_t y)
  {
    _rotate_pos(x, y);

    uint32_t row_bytes = _cfg.panel_width / 2;
    uint32_t byte_idx = y * row_bytes + (x >> 1);
    if (x & 1) {
      return _buf[byte_idx] & 0x0F;
    } else {
      return (_buf[byte_idx] >> 4) & 0x0F;
    }
  }

  void Panel_EPD_ED2208::_update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye)
  {
    _rotate_pos(xs, ys, xe, ye);

    if (xe >= _cfg.panel_width) xe = _cfg.panel_width - 1;
    if (ye >= _cfg.panel_height) ye = _cfg.panel_height - 1;

    _range_mod.left   = std::min<int32_t>(xs, _range_mod.left);
    _range_mod.right  = std::max<int32_t>(xe, _range_mod.right);
    _range_mod.top    = std::min<int32_t>(ys, _range_mod.top);
    _range_mod.bottom = std::max<int32_t>(ye, _range_mod.bottom);
  }

//----------------------------------------------------------------------------
 }
}
