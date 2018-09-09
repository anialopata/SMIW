/*
 * mk_ssd1306.c
 *
 *  Modifications on: 15-03-2017
 *      Author: Miros³aw Kardaœ
 */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "mk_ssd1306.h"

TVIEWPORT viewport = {0,127,0,SSD1306_HEIGHT-1};

#if BUFFERED == 1
uint8_t ssd1306_buf[ BUF_SIZE ];
#endif

void reset_viewport( void ) {
	viewport.left = 0;
	viewport.right = 127;
	viewport.top = 0;
	viewport.bottom = SSD1306_HEIGHT-1;
}

void set_viewport( uint8_t left, uint8_t right, uint8_t top, uint8_t bottom ) {
	viewport.left = left;
	viewport.right = right;
	viewport.top = top;
	viewport.bottom = bottom;
}

void set_viewport_pages( uint8_t left, uint8_t right, uint8_t page, uint8_t count ) {
	viewport.left = left;
	viewport.right = right;
	viewport.top = page * 8;
	viewport.bottom = (count * 8) - 1;
}

void set_viewport_x( uint8_t left, uint8_t right ) {
	viewport.left = left;
	viewport.right = right;
}

void set_viewport_y( uint8_t top, uint8_t bottom ) {
	viewport.top = top;
	viewport.bottom = bottom;
}



#if USE_CS2 == 1
uint8_t display_nr;
#endif

//*************************************** SPI
#if USE_SPI_OR_I2C == 1  || USE_SPI_OR_I2C == 3
static void SPIwrite( uint8_t dat ) {

	uint8_t i;
	for (i = 0x80; i; i >>= 1) {
		SCK_LO;
		if (dat & i) MOSI_HI;
		else MOSI_LO;
		SCK_HI;
	}
}

void ssd1306_InitSpi(void) {

	MOSI_DDR |= MOSI;
	MOSI_PORT |= MOSI;

	SCK_DDR |= SCK;
	SCK_PORT |= SCK;

#if USE_RST == 1
	RST_DDR |= RST;
	RST_PORT |= RST;
#endif

	DC_DDR |= DC;

#if USE_CS == 1
	CS_DDR |= CS;
	CS_PORT |= CS;
#endif

#if USE_CS2 == 1
	CS2_DDR |= CS2;
	CS2_PORT |= CS2;
#endif



}

#endif



//***************************************** I2C
#if USE_SPI_OR_I2C == 0  || USE_SPI_OR_I2C == 3

static void i2cBitrate(uint16_t bitrateKHz) {
	uint8_t bitrate_div;

	bitrate_div = ((F_CPU/1000l)/bitrateKHz);
	if(bitrate_div >= 16)
		bitrate_div = (bitrate_div-16)/2;

	TWBR = bitrate_div;
}

static void TWI_start(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
	while (!(TWCR&(1<<TWINT)));
}

static void TWI_stop(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	while ( (TWCR&(1<<TWSTO)) );
}

static void TWI_write(uint8_t bajt) {
	TWDR = bajt;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ( !(TWCR&(1<<TWINT)));
}

static void TWI_write_buf( uint8_t SLA, uint8_t adr, uint16_t len, uint8_t *buf ) {

	TWI_start();
	TWI_write(SLA);
	TWI_write(adr);
	while (len--) TWI_write(*buf++);
	TWI_stop();
}

#endif
//************************************** koniec I2C



//***** wylœlij komendê ***********
void mk_ssd1306_cmd( uint8_t cmd ) {

#if USE_SPI_OR_I2C == 1  || USE_SPI_OR_I2C == 3

	#if USE_CS == 1
		#if USE_CS2 == 0
		CS_HI;
		#else
		if( !display_nr ) CS_HI;
		else CS2_HI;
		#endif
	#endif

	DC_LO;

	#if USE_CS == 1
		#if USE_CS2 == 0
		CS_LO;
		#else
		if( !display_nr ) CS_LO;
		else CS2_LO;
		#endif
	#endif

	SPIwrite( cmd );

	#if USE_CS == 1
		#if USE_CS2 == 0
		CS_HI;
		#else
		if( !display_nr ) CS_HI;
		else CS2_HI;
		#endif
	#endif

#endif


#if USE_SPI_OR_I2C == 0 || USE_SPI_OR_I2C == 3

    uint8_t control = 0x00;
    TWI_start();
    TWI_write( OLED_I2C_ADDRESS );
    TWI_write( control );
    TWI_write( cmd );
    TWI_stop();

#endif

}


//***********  wyœlij dane *************
void mk_ssd1306_data( uint8_t dat ) {

#if USE_SPI_OR_I2C == 1  || USE_SPI_OR_I2C == 3

#if USE_CS == 1
	#if USE_CS2 == 0
    CS_HI;
	#else
    if( !display_nr ) CS_HI;
    else CS2_HI;
	#endif
#endif

    DC_HI;

#if USE_CS == 1
	#if USE_CS2 == 0
    CS_LO;
	#else
    if( !display_nr ) CS_LO;
    else CS2_LO;
	#endif
#endif

    SPIwrite( dat );

#if USE_CS == 1
	#if USE_CS2 == 0
    CS_HI;
	#else
    if( !display_nr ) CS_HI;
    else CS2_HI;
	#endif
#endif

#endif

#if USE_SPI_OR_I2C == 0 || USE_SPI_OR_I2C == 3

    uint8_t control = 0x40;
    TWI_start();
    TWI_write( OLED_I2C_ADDRESS );
    TWI_write( control );
    TWI_write( dat );
    TWI_stop();

#endif

}


//************ wyœlij bufor RAM do sterownika ************
void mk_ssd1306_display( void ) {
#if BUFFERED == 0
	return;
#else

	if( !can_display ) return;

	mk_ssd1306_cmd( SSD1306_SETLOWCOLUMN  );
	mk_ssd1306_cmd( SSD1306_SETHIGHCOLUMN );
//	mk_ssd1306_cmd( SSD1306_SETSTARTLINE  );


#ifdef SSD1306_128_32L
	mk_ssd1306_cmd( 0xB4 );						// wa¿na poprawka do biblioteki !!!!!
#endif

#ifdef SSD1306_128_32H
	mk_ssd1306_cmd( 0xB0 );
#endif

#ifdef SSD1306_128_64
	mk_ssd1306_cmd( 0xB0 );
#endif



#if USE_SPI_OR_I2C == 1 || USE_SPI_OR_I2C == 3

#if USE_CS == 1
	#if USE_CS2 == 0
    CS_HI;
	#else
    if( !display_nr ) CS_HI;
    else CS2_HI;
	#endif
#endif
	    DC_HI;

#if USE_CS == 1
	#if USE_CS2 == 0
    CS_LO;
	#else
    if( !display_nr ) CS_LO;
    else CS2_LO;
	#endif
#endif

	    for (uint16_t i=0; i<( BUF_SIZE ); i++) {
	    	SPIwrite( ssd1306_buf[i] );
	    }

#if USE_CS == 1
	#if USE_CS2 == 0
    CS_HI;
	#else
    if( !display_nr ) CS_HI;
    else CS2_HI;
	#endif
#endif

#endif

#if USE_SPI_OR_I2C == 0 || USE_SPI_OR_I2C == 3

	    TWI_write_buf( OLED_I2C_ADDRESS, 0x40, BUF_SIZE, ssd1306_buf );

#endif

#endif

	can_display = 0;
}



//************************************************************
// podstawowa funkcja do wyœwietlania pojedynczego pixela
void mk_ssd1306_setPixel(int x, int y, uint8_t bw) {
//	if ((x < 0) || (x >= SSD1306_WIDTH) || (y < 0) || (y >= SSD1306_HEIGHT ))
//		return;

	if ((x < viewport.left) || (x >= viewport.right) || (y < viewport.top) || (y >= viewport.bottom ))
		return;

#if BUFFERED == 0
	mk_ssd1306_cmd( 0x00 | (x&0x0f) ); // m³odsza czêœæ startowego adresu kolumny - 0x00 - 0x0f
	mk_ssd1306_cmd( 0x10 | (x>>4) ); // starsza czêœæ startowego adresu kolumny - 0x10 - 0x1f
//	mk_ssd1306_cmd( SSD1306_SETSTARTLINE   );
	mk_ssd1306_cmd( 0xB4 | (y/8) );	// 0xB0 dla 128x64 oraz 0xB4 dla 128x32


	mk_ssd1306_data( 1<<(y%8) );
#else

	  if (bw) ssd1306_buf[x+ (y/8)*SSD1306_WIDTH] |= (1<<(y%8)); 	// zapal pixel
	  else ssd1306_buf[x+ (y/8)*SSD1306_WIDTH] &= ~(1<<(y%8));		// zgaœ pixel

#endif
}


// clear screen buffer
void mk_ssdD1306_cls( void ) {
#if BUFFERED == 1
  memset( ssd1306_buf, 0x00, ( BUF_SIZE ));
#else
	mk_ssd1306_cmd( SSD1306_SETLOWCOLUMN  );
	mk_ssd1306_cmd( SSD1306_SETHIGHCOLUMN );

#ifdef SSD1306_128_32L
	mk_ssd1306_cmd( 0xB4 );						// wa¿na poprawka do biblioteki !!!!!
#endif

#ifdef SSD1306_128_32H
	mk_ssd1306_cmd( 0xB0 );
#endif

#ifdef SSD1306_128_64
	mk_ssd1306_cmd( 0xB0 );
#endif


    mk_ssd1306_cmd(SSD1306_COLUMNADDR);
    mk_ssd1306_cmd(0);   // Column start address (0 = reset)
    mk_ssd1306_cmd(SSD1306_WIDTH - 1); // Column end address (127 = reset)

    mk_ssd1306_cmd(SSD1306_PAGEADDR);
    mk_ssd1306_cmd(4); // Page start address (0 = reset)

#if SSD1306_HEIGHT == 64
    mk_ssd1306_cmd(7); // Page end address
#endif
#if SSD1306_HEIGHT == 32
    mk_ssd1306_cmd(7); // Page end address
#endif

	uint16_t i;
	for(i=0; i<BUF_SIZE; i++) mk_ssd1306_data(0x00);

#endif
	can_display=1;
}


//************* INICJALIZACJA sterownika SSD1306
void mk_ssd1306_init( uint8_t vcc, uint8_t refresh, uint8_t rst_disable ) {

#if USE_SPI_OR_I2C == 1 || USE_SPI_OR_I2C == 3
	ssd1306_InitSpi();

	if( !rst_disable ) {
	#if USE_RST == 1
		RST_HI;
		_delay_ms(25);
		RST_LO;
		_delay_ms(25);
		RST_HI;
	#endif
	}

#endif

#if USE_SPI_OR_I2C == 0 || USE_SPI_OR_I2C == 3

	i2cBitrate( 400 );
	I2C_DIR |= (1<<SCL)|(1<<SDA);
	_delay_ms(50);

#endif


    mk_ssd1306_cmd( SSD1306_DISPLAYOFF );
    mk_ssd1306_cmd( SSD1306_SETDISPLAYCLOCKDIV );
    mk_ssd1306_cmd( refresh );

    mk_ssd1306_cmd( SSD1306_SETDISPLAYOFFSET );
    mk_ssd1306_cmd( 0x0);
    mk_ssd1306_cmd( SSD1306_SETSTARTLINE | 0x0 );
    mk_ssd1306_cmd( SSD1306_CHARGEPUMP );

    if (vcc == SSD1306_EXTERNALVCC ) mk_ssd1306_cmd( 0x10 );
    else  mk_ssd1306_cmd( 0x14 );

    mk_ssd1306_cmd( SSD1306_MEMORYMODE );
    mk_ssd1306_cmd( 0x10);					// PageAddressing MODE
    mk_ssd1306_cmd( SSD1306_SEGREMAP | 0x1 ); // Column 0 --> mapped to SEG0
    mk_ssd1306_cmd( SSD1306_COMSCANDEC );

//    mk_ssd1306_cmd( SSD1306_SETCONTRAST );

    if (vcc == SSD1306_EXTERNALVCC ) mk_ssd1306_cmd( 0x9F );
    else mk_ssd1306_cmd( 0xCF );

    mk_ssd1306_cmd( SSD1306_SETPRECHARGE );

    // ssd1306 - 128 x 32
#if defined SSD1306_128_32L || SSD1306_128_32H
	    mk_ssd1306_cmd( SSD1306_SETMULTIPLEX );
	    mk_ssd1306_cmd( 0x1F );

	    mk_ssd1306_cmd( SSD1306_SETCOMPINS );
	    mk_ssd1306_cmd( 0x02 );
#endif

	// ssd1306 - 128 x 64
#if defined SSD1306_128_64
	    mk_ssd1306_cmd( SSD1306_SETMULTIPLEX );
	    mk_ssd1306_cmd( 0x3F );

	    mk_ssd1306_cmd( SSD1306_SETCOMPINS );
	    mk_ssd1306_cmd( 0x12 );
#endif

	    mk_ssd1306_cmd(SSD1306_SETVCOMDETECT);
	    mk_ssd1306_cmd(0x40);

		mk_ssd1306_cmd( SSD1306_DISPLAYALLON_RESUME );
		mk_ssd1306_cmd( SSD1306_NORMALDISPLAY );

	    mk_ssd1306_cmd( SSD1306_DISPLAYON );



	    mk_ssd1306_display();

}

#if BUFFERED == 1
void mk_ssd1306_refresh_pages( uint8_t page_nr, uint8_t pages_cnt, uint8_t col_start, uint8_t col_end ) {

	uint8_t page_cnt;

	uint8_t * ram_buf_start;
#if USE_SPI_OR_I2C == 0 || USE_SPI_OR_I2C == 3
	uint8_t * ram_buf_start1;
#endif

#if USE_SPI_OR_I2C == 1 || USE_SPI_OR_I2C == 3
	uint8_t col_cnt;
#endif


	for( page_cnt=page_nr; page_cnt<(page_nr+pages_cnt); page_cnt++ ) {

		mk_ssd1306_cmd( SSD1306_SETLOWCOLUMN  | (col_start & 0x0f) );
		mk_ssd1306_cmd( SSD1306_SETHIGHCOLUMN | col_start>>4 );
		mk_ssd1306_cmd( 0xB0 + page_cnt );

		ram_buf_start =  &ssd1306_buf[ ( page_cnt*128 )+col_start ];
#if USE_SPI_OR_I2C == 0 || USE_SPI_OR_I2C == 3
		ram_buf_start1 = ram_buf_start;
#endif

#if USE_SPI_OR_I2C == 1 || USE_SPI_OR_I2C == 3
		DC_HI;

		#if USE_CS == 1
			#if USE_CS2 == 0
			CS_LO;
			#else
			if( !display_nr ) CS_LO;
			else CS2_LO;
			#endif
		#endif

		for( col_cnt=col_start; col_cnt<col_end; col_cnt++ ) {
			SPIwrite( *ram_buf_start++ );
		}
#endif

#if USE_SPI_OR_I2C == 0 || USE_SPI_OR_I2C == 3

		TWI_write_buf( OLED_I2C_ADDRESS, 0x40, col_end-col_start, ram_buf_start1 );

#endif

	}
}
#endif

//********** Ustawienie kontrastu wyœwietlacza od 0 do 255
void mk_ssd1306_contrast( uint8_t contrast ) {

    mk_ssd1306_cmd( SSD1306_SETCONTRAST );
    mk_ssd1306_cmd( contrast );

}
