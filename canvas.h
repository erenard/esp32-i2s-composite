#pragma once
#include "constants.h"

class Canvas
{ 
  private:
  unsigned char * read;
  unsigned char * write;
  unsigned char * buffer_0;
  unsigned char * buffer_1;
  short active = 0;
  short inactive = 1;

  public:
  Canvas()
  {
    buffer_0 = new unsigned char [WIDTH * HEIGHT];
    buffer_1 = new unsigned char [WIDTH * HEIGHT];
    write = buffer_0;
    read = buffer_1;
  }

  inline void begin(unsigned char color = 0)
  {
    for(int p = 0; p < WIDTH * HEIGHT; p++) {
       write[p] = color;
    }
  }

  inline void end()
  {
    unsigned char * temp = read;
    read = write;
    write = temp;
  }

  inline unsigned char* get_frame() {
    return read;
  }

  inline void set_pixel(int x, int y, unsigned char color = 0)
  {
    write[y * WIDTH + x] = color;
  }

  void line(int x1, int y1, int x2, int y2, unsigned char color)
  {
    int x, y, xe, ye;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dx1 = labs(dx);
    int dy1 = labs(dy);
    int px = 2 * dy1 - dx1;
    int py = 2 * dx1 - dy1;
    if(dy1 <= dx1)
    {
      if(dx >= 0)
      {
        x = x1;
        y = y1;
        xe = x2;
      }
      else
      {
        x = x2;
        y = y2;
        xe = x1;
      }
      set_pixel(x, y, color);
      for(int i = 0; x < xe; i++)
      {
        x = x + 1;
        if(px < 0)
        {
          px = px + 2 * dy1;
        }
        else
        {
          if((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
          {
            y = y + 1;
          }
          else
          {
            y = y - 1;
          }
          px = px + 2 *(dy1 - dx1);
        }
        set_pixel(x, y, color);
      }
    }
    else
    {
      if(dy >= 0)
      {
        x = x1;
        y = y1;
        ye = y2;
      }
      else
      {
        x = x2;
        y = y2;
        ye = y1;
      }
      set_pixel(x, y, color);
      for(int i = 0; y < ye; i++)
      {
        y = y + 1;
        if(py <= 0)
        {
          py = py + 2 * dx1;
        }
        else
        {
          if((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
          {
            x = x + 1;
          }
          else
          {
            x = x - 1;
          }
          py = py + 2 * (dx1 - dy1);
        }
        set_pixel(x, y, color);
      }
    }
  }
};
