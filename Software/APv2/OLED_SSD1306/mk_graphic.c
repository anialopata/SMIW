/*
 * mk_graphic.c
 *
 *  Created on: 21-08-2014
 *      Author: admin
 */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "mk_ssd1306.h"

int cur_x, cur_y;

uint8_t can_display;

void mk_ssd1306_drawBitmap_P( int x, int y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color ) {

	int i, j, byteWidth = (w + 7) / 8;

	  for(j=0; j<h; j++) {
	    for(i=0; i<w; i++ ) {
	      if( pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7)) ) {
	    	  mk_ssd1306_setPixel(x+i, y+j, color);
	      }
	    }
	  }
	  can_display=1;
}

void mk_ssd1306_puts( int x, int y, char * str, uint8_t txt_size, uint8_t color, uint8_t bg ) {

	cur_x = x; cur_y = y;

	while( *str ) {

		mk_ssd1306_drawChar( cur_x, cur_y, *str++, color, bg, txt_size );
		cur_x += txt_size*6;
	}
	can_display=1;

}

void mk_ssd1306_puts_P( int x, int y, const char * str, uint8_t txt_size, uint8_t color, uint8_t bg ) {

	cur_x = x; cur_y = y;

	char c;
	while( (c = pgm_read_byte( str++ )) ) {
		mk_ssd1306_drawChar( cur_x, cur_y, c, color, bg, txt_size );
		cur_x += txt_size*6;
	}
	can_display=1;

}


void mk_ssd1306_put_int( int x, int y, int data, uint8_t txt_size, uint8_t color, uint8_t bg ) {

	char buf[16];
	mk_ssd1306_puts( x, y, itoa( data, buf, 10 ), txt_size, color, bg );
	can_display=1;
}


void mk_ssd1306_drawChar( int x, int y, char c, uint8_t color, uint8_t bg, uint8_t size ) {

	if((x >= viewport.right) || (y >= SSD1306_HEIGHT) || ((x + 6 * size - 1) < viewport.left) || ((y + 8 * size - 1) < 0)) return;

	uint8_t line;

	  for (int8_t i=0; i<6; i++ ) {

	    if (i == 5)
	      line = 0x0;
	    else
	      line = pgm_read_byte(font+(c*5)+i);

	    for (int8_t j = 0; j<8; j++) {
	      if (line & 0x1) {
	        if (size == 1)
	        	mk_ssd1306_setPixel(x+i, y+j, color);
	        else {
	        	mk_ssd1306_fillRect(x+(i*size), y+(j*size), size, size, color);
	        }
	      } else if (bg != color) {
	        if (size == 1)
	        	mk_ssd1306_setPixel(x+i, y+j, bg);
	        else {
	           mk_ssd1306_fillRect(x+i*size, y+j*size, size, size, bg);
	        }
	      }
	      line >>= 1;
	    }
	  }
	  can_display=1;

}


void mk_ssd1306_fillRect( int x, int y, int w, int h, uint8_t color ) {

	  for (int16_t i=x; i<x+w; i++) {
		  mk_ssd1306_drawFastVLine(i, y, h, color);
	  }
	  can_display = 1;

}

void mk_ssd1306_drawFastVLine( int x, int y, int h, uint8_t color ) {
  	mk_ssd1306_drawLine(x, y, x, y+h-1, color);
  	can_display=1;
}

void mk_ssd1306_drawFastHLine(int x, int y, int w, uint8_t color) {
	mk_ssd1306_drawLine(x, y, x+w-1, y, color);
	can_display=1;
}


// Bresenham's algorithm - thx wikpedia
void mk_ssd1306_drawLine( int x0, int y0, int x1, int y1, uint8_t color ) {

	int steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int err = dx / 2;
	int ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			mk_ssd1306_setPixel(y0, x0, color);
		} else {
			mk_ssd1306_setPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
	can_display=1;
}


// rysuj okr¹g
void mk_ssd1306_circle(int x, int y, uint8_t r, uint8_t bw) {
	int f = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int xa = 0;
	int ya = r;

	mk_ssd1306_setPixel(x, y + r, bw);
	mk_ssd1306_setPixel(x, y - r, bw);
	mk_ssd1306_setPixel(x + r, y, bw);
	mk_ssd1306_setPixel(x - r, y, bw);

	while (xa < ya) {
		if (f >= 0) {
			ya--;
			ddF_y += 2;
			f += ddF_y;
		}
		xa++;
		ddF_x += 2;
		f += ddF_x;

		mk_ssd1306_setPixel(x + xa, y + ya, bw);
		mk_ssd1306_setPixel(x - xa, y + ya, bw);
		mk_ssd1306_setPixel(x + xa, y - ya, bw);
		mk_ssd1306_setPixel(x - xa, y - ya, bw);
		mk_ssd1306_setPixel(x + ya, y + xa, bw);
		mk_ssd1306_setPixel(x - ya, y + xa, bw);
		mk_ssd1306_setPixel(x + ya, y - xa, bw);
		mk_ssd1306_setPixel(x - ya, y - xa, bw);
  }
	can_display=1;
}

void mk_ssd1306_fillCircle( int x, int y, uint8_t r, uint8_t color ) {
	mk_ssd1306_drawFastVLine( x, y-r, 2*r+1, color);
	mk_ssd1306_fillCircleHelper( x, y, r, 3, 0, color);
	can_display=1;
}


// Used to do circles and roundrects
void mk_ssd1306_fillCircleHelper( int x0, int y0, int r, uint8_t cornername, int delta, uint8_t color ) {

	int f     = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int x     = 0;
	int y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
    	mk_ssd1306_drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
    	mk_ssd1306_drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
    	mk_ssd1306_drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
    	mk_ssd1306_drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
  can_display=1;
}


void mk_ssd1306_drawRect( int x, int y, int w, int h, uint8_t color ) {
	mk_ssd1306_drawFastHLine(x, y, w, color);
	mk_ssd1306_drawFastHLine(x, y+h-1, w, color);
	mk_ssd1306_drawFastVLine(x, y, h, color);
	mk_ssd1306_drawFastVLine(x+w-1, y, h, color);
	can_display=1;
}


void mk_ssd1306_drawRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t color) {

	mk_ssd1306_drawFastHLine(x+r  , y    , w-2*r, color);
	mk_ssd1306_drawFastHLine(x+r  , y+h-1, w-2*r, color);
	mk_ssd1306_drawFastVLine(x    , y+r  , h-2*r, color);
	mk_ssd1306_drawFastVLine(x+w-1, y+r  , h-2*r, color);

	mk_ssd1306_drawCircleHelper(x+r    , y+r    , r, 1, color);
	mk_ssd1306_drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
	mk_ssd1306_drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
	mk_ssd1306_drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
	can_display=1;
}


void mk_ssd1306_fillRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t color) {
	mk_ssd1306_fillRect(x+r, y, w-2*r, h, color);

	mk_ssd1306_fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
	mk_ssd1306_fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
	can_display=1;
}

void mk_ssd1306_drawCircleHelper( int x0, int y0, int8_t r, uint8_t cornername, uint8_t color) {
	int f     = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int x     = 0;
	int y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
    	mk_ssd1306_setPixel(x0 + x, y0 + y, color);
    	mk_ssd1306_setPixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
    	mk_ssd1306_setPixel(x0 + x, y0 - y, color);
    	mk_ssd1306_setPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
    	mk_ssd1306_setPixel(x0 - y, y0 + x, color);
    	mk_ssd1306_setPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
    	mk_ssd1306_setPixel(x0 - y, y0 - x, color);
    	mk_ssd1306_setPixel(x0 - x, y0 - y, color);
    }
  }
}


void mk_ssd1306_drawTriangle( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color) {
	mk_ssd1306_drawLine(x0, y0, x1, y1, color);
	mk_ssd1306_drawLine(x1, y1, x2, y2, color);
	mk_ssd1306_drawLine(x2, y2, x0, y0, color);
	can_display=1;
}


void mk_ssd1306_fillTriangle ( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    mk_ssd1306_drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1,
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    mk_ssd1306_drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    mk_ssd1306_drawFastHLine(a, y, b-a+1, color);
  }
  can_display=1;
}




// display.scrollright(0x00, 0x0F)
void mk_ssd1306_startscrollright( uint8_t start, uint8_t stop ) {
	mk_ssd1306_cmd( SSD1306_RIGHT_HORIZONTAL_SCROLL );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( start );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( stop );
	mk_ssd1306_cmd( 0x01 );
	mk_ssd1306_cmd( 0xFF );
	mk_ssd1306_cmd( SSD1306_ACTIVATE_SCROLL );
}

void mk_ssd1306_startscrollleft( uint8_t start, uint8_t stop ) {
	mk_ssd1306_cmd( SSD1306_LEFT_HORIZONTAL_SCROLL );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( start );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( stop );
	mk_ssd1306_cmd( 0x01 );
	mk_ssd1306_cmd( 0xFF );
	mk_ssd1306_cmd( SSD1306_ACTIVATE_SCROLL );
}

// display.scrollright(0x00, 0x0F)
void mk_ssd1306_startscrolldiagright( uint8_t start, uint8_t stop ) {
	mk_ssd1306_cmd( SSD1306_SET_VERTICAL_SCROLL_AREA );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( SSD1306_HEIGHT );
	mk_ssd1306_cmd( SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( start );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( stop );
	mk_ssd1306_cmd( 0x01 );
	mk_ssd1306_cmd( SSD1306_ACTIVATE_SCROLL );
}

// display.scrollright(0x00, 0x0F)
void mk_ssd1306_startscrolldiagleft( uint8_t start, uint8_t stop ) {
	mk_ssd1306_cmd( SSD1306_SET_VERTICAL_SCROLL_AREA );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( SSD1306_HEIGHT );
	mk_ssd1306_cmd( SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( start );
	mk_ssd1306_cmd( 0x00 );
	mk_ssd1306_cmd( stop );
	mk_ssd1306_cmd( 0x01 );
	mk_ssd1306_cmd( SSD1306_ACTIVATE_SCROLL );
}

void mk_ssd1306_stopscroll( void ) {
	mk_ssd1306_cmd( SSD1306_DEACTIVATE_SCROLL );
}


void mk_ssd1306_invertDisplay( uint8_t i ) {
  if (i) mk_ssd1306_cmd(SSD1306_INVERTDISPLAY);
  else mk_ssd1306_cmd(SSD1306_NORMALDISPLAY);
}

