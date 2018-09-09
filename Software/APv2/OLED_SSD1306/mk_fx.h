/*
 * mk_scrl.h
 *
 *  Created on: 17 mar 2017
 *      Author: admin
 */

#ifndef MK_OLED_MK_SCRLOLL_H_
#define MK_OLED_MK_SCRLOLL_H_

enum { fx_none,
	fx_L, fx_R,
	fx_LR_in, fx_RL_in,
	fx_LR_out, fx_RL_out
};

typedef struct {
	uint8_t 	type;
	uint16_t 	delay;
	uint8_t 	xp;
	int8_t 		cont;
	uint8_t 	left_x;
	uint8_t 	right_x;
	int 		txt_x;
	uint16_t	txt_len;
	uint8_t 	txt_size;
	char *		txt;
	uint8_t 	fx;
} TMKFX;




void MK_FX_EVENT( void );

void fx_init( uint8_t page, uint8_t fx_type, uint8_t left_x, uint8_t right_x, uint8_t txt_size,
		 char *txt, uint8_t speed, uint16_t fx_delay, int8_t cont);


#endif /* MK_OLED_MK_SCRLOLL_H_ */
