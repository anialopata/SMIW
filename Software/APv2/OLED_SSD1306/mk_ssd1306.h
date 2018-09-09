/*
 * mk_ssd1306.h
 *
 *  Created on: 15-07-2014
 *      Author: Miros³aw Kardaœ
 */

#ifndef MK_SSD1306_H_
#define MK_SSD1306_H_

// definicje sta³ych preprocesora

// rozdzielczoœæ LCD/OLED

   #define SSD1306_128_64

//   #define SSD1306_128_32L		// LOW Pages  (Jeœli nie dzia³a ci z tym ustawieniem to spróbuj z kolejnym)

// ****** to ustawienie rzadko bêdzie potrzebne *****************
//   #define SSD1306_128_32H 	  	// HIGH Pages (jeœli nie dzia³a ci z 32L to spróbuj odkomentowaæ to ustawienie)


#define BUFFERED 1

#define	USE_SPI_OR_I2C 	1 	// 1-SPI, 0-I2C, 3-SPI & I2C

#if USE_SPI_OR_I2C == 0  || USE_SPI_OR_I2C == 3

/* konfiguracja sprzêtowego TWI / I2C */
#define OLED_I2C_ADDRESS 	0x78		// 0x7A

#define I2C_DIR		DDRC
#define SCL 		PC0
#define SDA			PC1

#endif

#if USE_SPI_OR_I2C == 1 || USE_SPI_OR_I2C == 3
//*********** konfiguracja programowego SPI
#define USE_CS  	1			// 1-u¿ywamy pinu CS, 0-gdy nie u¿ywamy pinu CS
#define USE_CS2  	0			// 1-u¿ywamy pinu CS2, 0-gdy nie u¿ywamy pinu CS2
#define USE_RST  	1

//#define SCK (1<<7)		//   <---- (CLK) SCL
//#define MOSI (1<<6)		//   <---- (MOSI) SDA
//#define RST (1<<5)		//	 <---- RST
//#define DC (1<<4)		//	 <---- D/C
//
//#define CS  (1<<3)		//	 <---- CS


#define SCK (1<<4)		//   <---- (CLK) SCL
#define MOSI (1<<3)		//   <---- (MOSI) SDA
#define RST (1<<1)		//	 <---- RST
#define DC (1<<0)		//	 <---- D/C

#define CS  (1<<2)		//	 <---- CS



#define CS2  (1<<5)		//	 <---- CS2

#define SCK_PORT  	PORTA
#define SCK_DDR		DDRA

#define MOSI_PORT  	PORTA
#define MOSI_DDR	DDRA

#define RST_PORT  	PORTA
#define RST_DDR		DDRA

#define DC_PORT  	PORTA
#define DC_DDR		DDRA

#define CS_PORT  	PORTA
#define CS_DDR		DDRA

#define CS2_PORT  	PORTA
#define CS2_DDR		DDRA

#endif



#define RST_LO 	RST_PORT &= ~RST
#define RST_HI 	RST_PORT |= RST

#define CS_LO 	CS_PORT &= ~CS
#define CS_HI 	CS_PORT |= CS

#define CS2_LO 	CS2_PORT &= ~CS2
#define CS2_HI 	CS2_PORT |= CS2

#define DC_LO 	DC_PORT &= ~DC
#define DC_HI 	DC_PORT |= DC

#define SCK_LO 	SCK_PORT &= ~SCK
#define SCK_HI 	SCK_PORT |= SCK

#define MOSI_LO 	MOSI_PORT &= ~MOSI
#define MOSI_HI 	MOSI_PORT |= MOSI


#define SSD1306_WIDTH                  128

#if defined SSD1306_128_64
  #define SSD1306_HEIGHT                 64
#endif
#if defined SSD1306_128_32L || SSD1306_128_32H
  #define SSD1306_HEIGHT                 32
#endif


#define BUF_SIZE 	(SSD1306_WIDTH*SSD1306_HEIGHT/8)

#define REFRESH_MIN 0x80
#define REFRESH_MID 0xB0
#define REFRESH_MAX 0xF0

typedef enum {page0, page1, page2, page3, page4, page5, page6, page7} TPAGE;


#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A


typedef struct {
	uint8_t left;
	uint8_t right;
	uint8_t top;
	uint8_t bottom;
} TVIEWPORT;

#define swap(a, b) { int16_t t = a; a = b; b = t; }


// deklaracje zmiennych

#if USE_CS2 == 1
extern uint8_t display_nr;
#endif

extern const  uint8_t font[];

extern int cur_x, cur_y;

extern uint8_t can_display;

extern TVIEWPORT viewport;

//extern uint8_t ssd1306_buf[ BUF_SIZE ];



// deklaracje funkcji - sprzêtowych
void mk_ssd1306_init( uint8_t vcc, uint8_t refresh, uint8_t rst_disable );
void mk_ssd1306_cmd( uint8_t cmd );
void mk_ssd1306_data( uint8_t dat );

void mk_ssd1306_display( void );
void mk_ssd1306_contrast( uint8_t contrast );
void mk_ssd1306_refresh_pages( uint8_t page_nr, uint8_t pages_cnt, uint8_t col_start, uint8_t col_end );

void mk_ssd1306_setPixel( int x, int y, uint8_t bw );
void mk_ssdD1306_cls( void );


// funkcje graficzne
void mk_ssd1306_drawBitmap_P( int x, int y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color );
void mk_ssd1306_drawChar( int x, int y, char c, uint8_t color, uint8_t bg, uint8_t size );
void mk_ssd1306_puts( int x, int y, char * str, uint8_t txt_size, uint8_t color, uint8_t bg );
void mk_ssd1306_puts_P( int x, int y, const char * str, uint8_t txt_size, uint8_t color, uint8_t bg );
void mk_ssd1306_put_int( int x, int y, int data, uint8_t txt_size, uint8_t color, uint8_t bg );

void mk_ssd1306_fillRect( int x, int y, int w, int h, uint8_t color );
void mk_ssd1306_drawFastVLine( int x, int y, int h, uint8_t color );
void mk_ssd1306_drawFastHLine(int x, int y, int w, uint8_t color);
void mk_ssd1306_drawLine( int x0, int y0, int x1, int y1, uint8_t color );

void mk_ssd1306_fillCircle( int x, int y, uint8_t r, uint8_t color );
void mk_ssd1306_circle(int x, int y, uint8_t r, uint8_t bw);
void mk_ssd1306_fillCircleHelper( int x0, int y0, int r, uint8_t cornername, int delta, uint8_t color );

void mk_ssd1306_drawRect( int x, int y, int w, int h, uint8_t color );
void mk_ssd1306_drawRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t color);
void mk_ssd1306_fillRoundRect( int x, int y, int w, int h, uint8_t r, uint8_t color);
void mk_ssd1306_drawCircleHelper( int x0, int y0, int8_t r, uint8_t cornername, uint8_t color);

void mk_ssd1306_drawTriangle( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color );
void mk_ssd1306_fillTriangle ( int x0, int y0, int x1, int y1, int x2, int y2, uint8_t color );

// scroll
void mk_ssd1306_startscrollright( uint8_t start, uint8_t stop );
void mk_ssd1306_startscrollleft( uint8_t start, uint8_t stop );
void mk_ssd1306_startscrolldiagright( uint8_t start, uint8_t stop );
void mk_ssd1306_startscrolldiagleft( uint8_t start, uint8_t stop );
void mk_ssd1306_stopscroll( void );

void mk_ssd1306_invertDisplay( uint8_t i );

void reset_viewport( void );
void set_viewport( uint8_t left, uint8_t right, uint8_t top, uint8_t bottom );
void set_viewport_pages( uint8_t left, uint8_t right, uint8_t page, uint8_t count );
void set_viewport_x( uint8_t left, uint8_t right );
void set_viewport_y( uint8_t left, uint8_t right );


#endif /* MK_SSD1306_H_ */
