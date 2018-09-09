/*
 * mk_scrl.c
 *
 *  Created on: 17 mar 2017
 *      Author: admin
 */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "mk_fx.h"

#include "mk_ssd1306.h"


TMKFX mkfx[8];


void fx_init( uint8_t page, uint8_t fx_type, uint8_t left_x, uint8_t right_x, uint8_t txt_size,
		 char *txt, uint8_t speed, uint16_t fx_delay, int8_t cont) {

	uint16_t len = strlen( txt ) * 6;
	mkfx[ page ].txt_len 	= len;
	mkfx[ page ].txt 		= txt;
	mkfx[ page ].type 		= fx_type;
	mkfx[ page ].delay 		= fx_delay;
	mkfx[ page ].left_x 	= left_x;
	mkfx[ page ].right_x 	= right_x + 1;
	mkfx[ page ].cont		= cont;
	mkfx[ page ].txt_size 	= txt_size;
	mkfx[ page ].xp			= speed;
	mkfx[ page ].fx 		= 0;

	if( (fx_type == fx_L) ) {						// fx_L
		mkfx[ page ].txt_x = right_x + 1;
	}
	else if( (fx_type == fx_R) ) {				// fx_R
		mkfx[ page ].txt_x = left_x - len * txt_size;
	}
	else if( fx_type == fx_LR_in ) {										// fx_LR_in
		if( left_x + len < right_x ) mkfx[ page ].txt_x = right_x+1;// - len * txt_size;
		else mkfx[ page ].txt_x = left_x;
	}
	else if( fx_type == fx_RL_in ) {										// fx_RL_in
		if( left_x + len < right_x ) mkfx[ page ].txt_x = left_x - len * txt_size;
		else mkfx[ page ].txt_x = right_x - len;
	}
	else if( fx_type == fx_LR_out ) mkfx[ page ].txt_x = right_x;		// fx_LR_out
	else if( fx_type == fx_RL_out ) mkfx[ page ].txt_x = left_x - len;	// fx_RL_out

}

static void ssd1306_send( char * txt, uint8_t txt_size, uint8_t color ) {
	mk_ssd1306_puts( cur_x, cur_y*8, txt, txt_size, color, 0 );
}

void MK_FX_EVENT( void ) {

	static uint16_t cnt;
	cnt++;


	for( uint8_t page = 0; page < 8; page++ ) {

		if( mkfx[ page ].fx > 3 ) {
			mkfx[ page ].fx = 0;
			mkfx[ page ].type = 0;
			mkfx[ page ].cont = 0;
		}

		if( !mkfx[ page ].type ) continue;





		if( !( cnt % mkfx[ page ].delay ) ) {

			if( !mkfx[ page ].cont ) {
				if( mkfx[ page ].type == 3 || mkfx[ page ].type == 4 ) {
					mkfx[ page ].type = fx_R;
					mkfx[ page ].cont = 1;
					mkfx[ page ].txt_x = mkfx[ page ].left_x;
					mkfx[ page ].txt = "|";
					mkfx[ page ].xp = 2;
					mkfx[ page ].fx++;
					can_display = 1;
				}
				continue;
			}

			uint16_t len = strlen( mkfx[ page ].txt ) * 6;
			mkfx[ page ].txt_len 	= len;

			switch( mkfx[ page ].type ) {

			case fx_L:
			{
				set_viewport_x( mkfx[page].left_x, mkfx[page].right_x );
				if( (int)(mkfx[ page ].txt_x + ( mkfx[ page ].txt_len * mkfx[ page ].txt_size )) > mkfx[ page ].left_x )
					cur_x = mkfx[ page ].txt_x -= mkfx[ page ].xp;
				else {
					cur_x = mkfx[ page ].txt_x = mkfx[ page ].right_x;
					if( mkfx[ page ].cont>0 ) mkfx[ page ].cont--;
				}

				cur_y = page;
				ssd1306_send( mkfx[ page ].txt, mkfx[ page ].txt_size, 1 );
				cur_y = page;
				mk_ssd1306_drawChar( cur_x, cur_y*8, ' ', 1, 0, mkfx[ page ].txt_size );
				can_display = 1;
				reset_viewport();
				break;
			}

			case fx_R:
			{
				set_viewport_x( mkfx[page].left_x, mkfx[page].right_x );
				if( mkfx[ page ].txt_x <= mkfx[ page ].right_x ) {
					cur_y = page;
					if( mkfx[ page ].txt_x >= 0 ) {
						cur_x = mkfx[ page ].txt_x - mkfx[ page ].txt_size;
//						cur_x = mkfx[ page ].txt_x - (5*mkfx[ page ].txt_size);
						if( cur_x<mkfx[ page ].xp ) cur_x=0;
						mk_ssd1306_drawChar( cur_x, cur_y*8, ' ', 1, 0, mkfx[ page ].txt_size );
						cur_y = page;
					}
					cur_x = mkfx[ page ].txt_x += mkfx[ page ].xp;
					ssd1306_send( mkfx[ page ].txt, mkfx[ page ].txt_size, 1 );
					can_display = 1;
				} else {
					cur_x = mkfx[ page ].txt_x = mkfx[ page ].left_x - ( mkfx[ page ].txt_len * mkfx[ page ].txt_size );
					if( mkfx[ page ].cont>0 ) mkfx[ page ].cont--;
				}
				reset_viewport();
				break;
			}


			case fx_LR_in:
			{
				uint8_t xc = mkfx[ page ].txt_x - mkfx[ page ].left_x;// + ( mkfx[ page ].txt_len * mkfx[ page ].txt_size ));
				if( xc>mkfx[ page ].xp ) xc=mkfx[ page ].xp;

				if( mkfx[ page ].txt_x > mkfx[ page ].left_x){// + xc ) {
					cur_x = mkfx[ page ].txt_x -= xc;//mkfx[ page ].xp;
					cur_y = page;

					ssd1306_send( mkfx[ page ].txt, mkfx[ page ].txt_size, 1 );
					mk_ssd1306_drawChar( cur_x, cur_y, ' ', 1, 0, mkfx[ page ].txt_size );
					can_display = 1;

				}
				else {
					mkfx[ page ].type = fx_RL_in;
					if( mkfx[ page ].cont>0 ) mkfx[ page ].cont--;
				}
				break;
			}

			case fx_RL_in:
			{
				uint8_t xc = mkfx[ page ].right_x - (mkfx[ page ].txt_x + ( mkfx[ page ].txt_len * mkfx[ page ].txt_size ));
				if( xc>mkfx[ page ].xp ) xc=mkfx[ page ].xp;
				if( 1+ mkfx[ page ].txt_x + ( mkfx[ page ].txt_len * mkfx[ page ].txt_size ) < mkfx[ page ].right_x ) {// - mkfx[ page ].xp ) {
					cur_y = page;
					set_viewport_x( mkfx[page].left_x, mkfx[page].right_x );
					if( mkfx[ page ].txt_x >= 0 ) {
						cur_x = mkfx[ page ].txt_x - (5*mkfx[ page ].txt_size)+xc;
						mk_ssd1306_drawChar( cur_x, cur_y*8, ' ', 1, 0, mkfx[ page ].txt_size );
						cur_y = page;
					}
					cur_x = mkfx[ page ].txt_x += xc;//mkfx[ page ].xp;
					ssd1306_send( mkfx[ page ].txt, mkfx[ page ].txt_size, 1 );

//					set_viewport_x( xl, xr );
					can_display = 1;
				} else {
//					cur_x = 0;//mkfx[ page ].right_x - mkfx[ page ].txt_len * mkfx[ page ].txt_size;
//					ssd1306_send( mkfx[ page ].txt, mkfx[ page ].txt_size, 1 );
					cur_x = mkfx[ page ].txt_x;
					mkfx[ page ].type = fx_LR_in;
					if( mkfx[ page ].cont>0 ) mkfx[ page ].cont--;
				}
//				can_display = 1;
				reset_viewport();
				break;
			}


			case fx_LR_out:
				set_viewport_x( mkfx[page].left_x, mkfx[page].right_x );
				if( mkfx[ page ].txt_x + ( mkfx[ page ].txt_len * mkfx[ page ].txt_size ) > mkfx[ page ].left_x + mkfx[ page ].xp )
					cur_x = mkfx[ page ].txt_x -= mkfx[ page ].xp;
				else {
					cur_x = mkfx[ page ].txt_x -= mkfx[ page ].xp;
					mkfx[ page ].type = fx_RL_out;
					if( mkfx[ page ].cont>0 ) mkfx[ page ].cont--;
				}
				cur_y = page;

				ssd1306_send( mkfx[ page ].txt, mkfx[ page ].txt_size, 1 );
				mk_ssd1306_drawChar( cur_x, cur_y, ' ', 1, 0, mkfx[ page ].txt_size );
				reset_viewport();
				can_display = 1;
				break;


			case fx_RL_out:
				set_viewport_x( mkfx[page].left_x, mkfx[page].right_x );
				if( mkfx[ page ].txt_x <= mkfx[ page ].right_x + mkfx[ page ].xp ) {
					cur_y = page;
					if( mkfx[ page ].txt_x >= -mkfx[ page ].xp ) {
						cur_x = mkfx[ page ].txt_x - mkfx[ page ].txt_size;
						mk_ssd1306_drawChar( cur_x, cur_y*8, ' ', 1, 0, mkfx[ page ].txt_size );
						cur_y = page;
					}
					cur_x = mkfx[ page ].txt_x += mkfx[ page ].xp;

					ssd1306_send( mkfx[ page ].txt, mkfx[ page ].txt_size, 1 );
					can_display = 1;
				} else {
					cur_x = mkfx[ page ].txt_x;
					mkfx[ page ].type = fx_LR_out;
					if( mkfx[ page ].cont>0 ) mkfx[ page ].cont--;
				}
				reset_viewport();
				break;



			}


		}

	}




}



