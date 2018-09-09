/*
 * APv2.c
 *
 * Created: 09.05.2018 02:22:55
 * Author : anna.lopata
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "OLED_SSD1306/mk_ssd1306.h"
#include "OLED_SSD1306/mk_fx.h"

#include "RTC_DS1307/rtc.h"

#include "DISPLAY_MAX7219/max7219.h"

volatile rtc_t rtc;
volatile uint8_t current_time[3];
volatile uint8_t buffered_time[3];
volatile uint8_t current_choice = 0;

volatile uint8_t keys = 0x00;
volatile uint8_t keysIndicator[4] = { 0x00, 0x00, 0x00, 0x00 };
	
volatile uint8_t displayFlag = 0x00;
volatile uint8_t alarmFlash = 0x00;
volatile uint8_t minusOrPlus = 0;


volatile uint8_t alarm_on = 0x00; 
volatile uint8_t alarm_time[3];
volatile uint8_t hours_to_alarm = 0xff;
volatile uint8_t show_time = 0x00;
volatile uint8_t sleep_mode = 0x00;

uint8_t playerFiles[3][10] = {{0x7E,0xFF,0x06,0x03,0x00,0x00,0x01,0xFE,0xF7,0xEF},
							  {0x7E,0xFF,0x06,0x03,0x00,0x00,0x02,0xFE,0xF6,0xEF},
							  {0x7E,0xFF,0x06,0x03,0x00,0x00,0x03,0xFE,0xF5,0xEF}};
volatile uint8_t song = 0; 
	
enum Options
{
	UNDEFINED,
	SET,
	CHECK,
	QTY_OF_OPTIONS
};
volatile enum Options option = UNDEFINED;

enum States
{
	NORMAL, // wyswietlanie godziny
	SETTING, // wyswietlanie ustawien budzika 
	SETTING_CONFIRMATION, //stan, w ktorym mam wyswietlony ekran potwierdzenia ustawionej godziny
	ALARM_ON, 
	SHOW_ALARM,
};
volatile enum States state = NORMAL;

volatile uint8_t change = 0x00;
volatile uint8_t event = 0x00;

volatile uint16_t timerHelper = 0;

void trigger_player(uint8_t file)
{
	for(uint8_t i=0;i<10;i++){
		while (!( UCSRA & (1<<UDRE)));
		UDR = playerFiles[file][i];
	}
}
void stop_player()
{
	uint8_t stop[10] = {0x7E,0xFF,0x06,0x0C,0x00,0x00,0x00,0xFE,0xEF,0xEF};
	for(uint8_t i=0;i<10;i++){
		while (!( UCSRA & (1<<UDRE)));
		UDR = stop[i];
	}
}
uint8_t incrdecrementBCD(uint8_t data, uint8_t value, uint8_t plus, uint8_t limit)
{
	uint8_t data_dec = ((data&0xF0)>>4)*10 + (data&0x0F);
	if(!plus){
		if(data_dec>0) data_dec -= value;
		else data_dec = limit;
		}else{
		if(data_dec<limit) data_dec += value;
		else data_dec = 0;
	}
	return (((data_dec/10)<<4)&0xF0) | ((data_dec%10)&0x0F);
}
void settingAlarm(uint8_t direction)
{
	if(current_choice == 0) buffered_time[current_choice] = incrdecrementBCD(buffered_time[current_choice],1,direction,24); //data, valueIncrement, 1-plus/0-minus,limit
	else buffered_time[current_choice] = incrdecrementBCD(buffered_time[current_choice],1,direction,60);
}
uint8_t bcd_to_normal(uint8_t data)
{
	return ((data&0xF0)>>4)*10 + (data&0x0F);
}

ISR (TIMER0_OVF_vect)
{
	TCNT0 += 6;
	
	if(timerHelper >= 300){
		timerHelper = 0;
		RTC_GetDateTime(&rtc);
		
		current_time[0] = rtc.hour;
		current_time[1] = rtc.min;
		current_time[2] = rtc.sec;
	}else
	{
		timerHelper++;
	}
	
	if(alarm_on == 0x01){
		if( (current_time[0] == alarm_time[0])&&(current_time[1] == alarm_time[1]) ){
			trigger_player(song);
			state = ALARM_ON;
			alarm_on = 0x02;
			event = 0x01;
			sleep_mode = 0x00;
		}
		
		//time to alarm
		if(current_time[0] <= alarm_time[0]){
			hours_to_alarm = bcd_to_normal(alarm_time[0]) - bcd_to_normal(current_time[0]);
		}else
			hours_to_alarm = 24 - bcd_to_normal(current_time[0]) + bcd_to_normal(alarm_time[0]);
	}
}

volatile uint16_t num[10] ={0b0111101101101111,  //0
							0b0010010010010010,  //1
							0b0111001111100111,  //2
							0b0111100110100111,  //3
							0b0100100111101101,  //4
							0b0111100111001111,  //5
							0b0111101111001111,  //6
							0b0100100100100111,  //7
							0b0111101111101111,  //8
							0b0111100111101111}; //9

void set_display(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t indicator) //indicator > 0 -> show
{
	uint16_t number[6];
	
	//ustawienie godziny
	number[1] = num[hours&0x0F];
    if((hours&0xF0) > 0) number[0] = num[(hours&0xF0)>>4];
	else number[0] = num[0];	
	
	//ustawienie minut
	number[3] = num[minutes&0x0F];
	if((minutes&0xF0) > 0) number[2] = num[(minutes&0xF0)>>4];
	else number[2] = num[0];
	
	//ustawienie sekund
	number[5] = num[seconds&0x0F];
	if((seconds&0xF0) > 0) number[4] = num[(seconds&0xF0)>>4];
	else number[4] = num[0];
	
	uint8_t display_buffer[4][8] = {{0x00}};
	
	uint8_t i;
	uint8_t index;
	
	//nadpisywanie 1, dla wygl¹du cyfr
	// i - indeks bajta
	// index - pozycja w tablicy dla cyfry
	for(i=5, index=0; i>=1; i--) {
		display_buffer[0][i] = 0b00001110&((number[0]>>index)<<1);
		display_buffer[0][i] |= 0b11100000&((number[1]>>index)<<5); // aby nie nadpisaæ 0, nie zepsuæ cyfry, która stoi obok
		display_buffer[1][i] |= 0b01110000&((number[2]>>index)<<4);
		display_buffer[2][i] |= 0b00000111&((number[3]>>index)<<0);
		if(seconds != 0xff){ 
			display_buffer[3][i] |= 0b00000111&((number[4]>>index)<<0);
			display_buffer[3][i] |= 0b01110000&((number[5]>>index)<<4);
		}
		index += 3;
	}

	// rysowanie kropek
	display_buffer[1][2] |= 0x02;
	display_buffer[1][4] |= 0x02;
	if(seconds != 0xff){ 
		display_buffer[2][2] |= 0x20;
		display_buffer[2][4] |= 0x20;
	}
	
	if(indicator == 1){ // przy ustawianiu godziny alarmu
		display_buffer[0][7] |= 0xee; // 11101110 --- ---
		display_buffer[0][6] |= 0x10; // _
	}else if(indicator == 2){
		display_buffer[1][7] |= 0x70;
		display_buffer[2][7] |= 0x07;
		display_buffer[1][6] |= 0x80;
	}
	
	//time to alarm
	if((hours_to_alarm != 0xff) && (state == NORMAL)){
		display_buffer[0][7] |= 0xFF;
		display_buffer[1][7] |= 0xFF;
		display_buffer[2][7] |= 0xFF;
		display_buffer[3][7] |= 0xFF;
		// w zaleznosci od ilosci czasu do alarmu wstawiamy 0 od prawej do lewej
		for(uint8_t i=0;i<hours_to_alarm;i++){
			if(i<4){
				display_buffer[3][7] &= ~(8>>i);
			}
			if((i>=4) && (i<12)){
				display_buffer[2][7] &= ~(128>>(i-4));
			}
			if((i>=12) && (i<20)){
				display_buffer[1][7] &= ~(128>>(i-12));
			}
			if((i>=20) && (i<24)){
				display_buffer[0][7] &= ~(128>>(i-20));
			}
		}
	}
	
	if(alarm_on == 0x01){ // wyswietlanie 4 kropek
		display_buffer[0][6] |= 0x01;
		display_buffer[0][0] |= 0x01;
		display_buffer[3][6] |= 0x80;
		display_buffer[3][0] |= 0x80;
	}else if((alarm_on == 0x02) && (alarmFlash < 30)){
		// ic - wyswietlacz
		// i - bajt na wyswietlaczu
		for(uint8_t ic=0; ic<MAX7219_ICNUMBER; ic++) {
			for(i=0; i<8; i++) {
				display_buffer[ic][i] ^= 0xff; 
			}
		}
	}
	
	if(sleep_mode == 0x01){
		for(uint8_t ic=0; ic<MAX7219_ICNUMBER; ic++) {
			for(i=0; i<8; i++) {
				display_buffer[ic][i] = 0x00;
			}
		}
	}
	
	for(uint8_t ic=0; ic<MAX7219_ICNUMBER; ic++) {
		for(i=0; i<8; i++) {
				max7219_digit(ic, i, display_buffer[ic][i]); 
		}
	}
}


ISR(TIMER1_OVF_vect) //30 times per second = 16Mhz/65536/8
{
	if(keysIndicator[0] > 3) //so 6 times per second (waiting 160-200ms)
	{
		keysIndicator[0] = 0;
		if( (PINC&0b00010000) == 0 ) keys |= (1<<0);
	}
	if(keysIndicator[1] > 3)
	{
		keysIndicator[1] = 0;
		if( (PINC&0b00100000) == 0 ) keys |= (1<<1);
	}
	if(keysIndicator[2] > 3)
	{
		keysIndicator[2] = 0;
		if( (PINC&0b01000000) == 0 ) keys |= (1<<2);
	}
	if(keysIndicator[3] > 3)
	{
		keysIndicator[3] = 0;
		if( (PINC&0b10000000) == 0 ) keys |= (1<<3);
	}
	
	if( (keysIndicator[0] == 0) && (PINC&0b00010000) == 0 )
	{
		keysIndicator[0] = 1;
	}
	if( (keysIndicator[1] == 0) && (PINC&0b00100000) == 0 )
	{
		keysIndicator[1] = 1;
	}
	if( (keysIndicator[2] == 0) && (PINC&0b01000000) == 0 )
	{
		keysIndicator[2] = 1;
	}
	if( (keysIndicator[3] == 0) && (PINC&0b10000000) == 0 )
	{
		keysIndicator[3] = 1;
	}
	
	if( (keys&0b0001) == 0b0001 && (keysIndicator[0] == 0) && (PINC&0b00010000) == 0b00010000 ) keys &= ~(1<<0);
	if( (keys&0b0010) == 0b0010 && (keysIndicator[1] == 0) && (PINC&0b00100000) == 0b00100000 ) keys &= ~(1<<1);
	if( (keys&0b0100) == 0b0100 && (keysIndicator[2] == 0) && (PINC&0b01000000) == 0b01000000 ) keys &= ~(1<<2);
	if( (keys&0b1000) == 0b1000 && (keysIndicator[3] == 0) && (PINC&0b10000000) == 0b10000000 ) keys &= ~(1<<3);
	
	if(keysIndicator[0] > 0) keysIndicator[0]++;
	if(keysIndicator[1] > 0) keysIndicator[1]++;
	if(keysIndicator[2] > 0) keysIndicator[2]++;
	if(keysIndicator[3] > 0) keysIndicator[3]++;
	
	displayFlag ^= 1;
	if(alarmFlash < 60)
	{
		alarmFlash++;
	}else
		alarmFlash = 0;
	
	if(state == SHOW_ALARM){	
		if(show_time < 90){
			show_time++;
		}else{
			state = NORMAL;
			show_time = 0;
		}
	}
}

void set_button()
{
	if((option == SET) && (state != ALARM_ON)){
		if(state == NORMAL)
		{
			buffered_time[0] = current_time[0];
			buffered_time[1] = current_time[1];
			buffered_time[2] = current_time[2];
			state = SETTING;
			}else if(state == SETTING){
			state = SETTING_CONFIRMATION;
			}else if(state == SETTING_CONFIRMATION){
			alarm_time[0] = buffered_time[0];
			alarm_time[1] = buffered_time[1];
			alarm_time[2] = buffered_time[2];
			alarm_on = 0x01;
			state = NORMAL;
		}
		}else if((option == CHECK) && (state != ALARM_ON) && (alarm_on>0)){
		state = SHOW_ALARM;
	}
	
	if(state == ALARM_ON){
		alarm_on = 0x00;
		stop_player();
		state = NORMAL;
		hours_to_alarm = 0xff;
	}
	event = 0x01;
}
void plus_button()
{
	if(state == NORMAL)
	{
		if(option < 2) option = option + 1;
		event = 0x01;
	}else if(state == SETTING){
		minusOrPlus = 1;
		settingAlarm(minusOrPlus);
	}else if(state == SETTING_CONFIRMATION){
		alarm_on = 0x00;
		stop_player();
		state = NORMAL;
		hours_to_alarm = 0xff;
		event = 0x01;
	}
}
void minus_button()
{
	if(state == NORMAL)
	{
		if(option > 1) option = option - 1;
		event = 0x01;
	}
	else if(state == SETTING){
		minusOrPlus = 0;
		settingAlarm(minusOrPlus);
	}else if(state == SETTING_CONFIRMATION){
		alarm_on = 0x00;
		stop_player();
		state = NORMAL;
		hours_to_alarm = 0xff;
		event = 0x01;
	}
}
void back_button()
{
	if(state == SETTING){
		if(current_choice > 0){
			current_choice = 0;
		}else
		current_choice++;
	}else if(state == SETTING_CONFIRMATION)
	{
		state = SETTING;
	}else if(state == ALARM_ON) 
	{
		alarm_on = 0x00;
		stop_player();
		state = NORMAL;
		hours_to_alarm = 0xff;
	}
	if(state == NORMAL){
		sleep_mode = 0x01;
		mk_ssdD1306_cls();
	}else
		event = 0x01;
}

void service_buttons()
{	
// keys - stan przycisków (wcisniete/puszczone)
	if( (keys&0b0010) == 0b0010) // czy 2 przycisk jest wcisniety
	{
		// change - stan przyciskow (czy przytrzymane po wcisnieciu)
		if( (change&0b0010) == 0 )
		{
			change |= (1<<1); // |= 0b0010 wykrycie przytrzymania przycisku 
			if(!sleep_mode) set_button();
			else{
				sleep_mode = 0x00;
				event = 0x01;
			}	
		}
	}else{
		change &= ~(1<<1);
	}
	if( (keys&0b0001) == 0b0001 )
	{
		if( (change&0b0001) == 0 )
		{
			change |= (1<<0);
			if(!sleep_mode) plus_button();
			else{
				sleep_mode = 0x00;
				event = 0x01;
			}
		}
	}else{
		change &= ~(1<<0);
	}
	if( (keys&0b1000) == 0b1000 )
	{
		if( (change&0b1000) == 0 )
		{
			change |= (1<<3);
			if(!sleep_mode) minus_button();
			else{
				sleep_mode = 0x00;
				event = 0x01;	
			}
		}
	}else{
		change &= ~(1<<3);
	}
	if( (keys&0b0100) == 0b0100)
	{
		if( (change&0b0100) == 0 )
		{
			change |= (1<<2);
			if(!sleep_mode) back_button();
			else{
				sleep_mode = 0x00;
				event = 0x01;
			}	
		}
	}else{
		change &= ~(1<<2);
	}
}
int main(void)
{	
	UBRRH = 0; //data bits: 8 //baud:  9600 as for 16Mhz clock
	UBRRL = 103; //stop bits:  1 //parity:  No
	UCSRC |= (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRB |= (1<<TXEN)|(1<<RXEN)|(1<<RXCIE);
	
	PORTA = 0x00;
	DDRA = 0xff;
	DDRB = 0xff;
	PORTB = 0x02;
	PORTC = 0xf0;
	DDRC = 0x0f;		
	PORTD = 0x00;
	DDRD = 0xfe;
	
	max7219_init();
	uint8_t ic = 0;
	for(ic=0; ic<MAX7219_ICNUMBER; ic++) {
		max7219_shutdown(ic, 0);
	}
	for(ic=0; ic<MAX7219_ICNUMBER; ic++) {
		max7219_shutdown(ic, 1);
		max7219_decode(ic, 0);
		max7219_intensity(ic, 0);
		max7219_scanlimit(ic, 7);
	}
	for(ic=0; ic<MAX7219_ICNUMBER; ic++) {
		max7219_test(ic, 1);
		_delay_ms(10);
		max7219_test(ic, 0);
	}
	for(ic=0; ic<MAX7219_ICNUMBER; ic++) {
		for(uint8_t i=0; i<8; i++) {
			max7219_digit(ic, i, 0);
		}
	}
	
	mk_ssd1306_init( SSD1306_SWITCHCAPVCC, REFRESH_MAX, 0 );
	mk_ssd1306_contrast(200);
	
	//T0
	TIMSK |= (1 << TOIE0);
	TCCR0 |= (1 << CS01) | (1 << CS00);	
	//T1
	TCCR1B |= (1 << CS11);
	TIMSK |= (1 << TOIE1);
	
	RTC_Init();
	
	
	//uncomment for time setting
	/*
    rtc.hour = 0x21;
    rtc.min =  0x24;
    rtc.sec =  0x55;

    rtc.date = 0x27;
    rtc.month = 0x05;
    rtc.year = 0x18;
    rtc.weekDay = 7;

    RTC_SetDateTime(&rtc);*/
	
	option = SET;
	
	//fx_init( 3, fx_LR_in, 0, 127, 1, "fruu", 1, 1, -1 );
	mk_ssdD1306_cls();
	mk_ssd1306_puts_P( 12,0, PSTR("a³alarm"), 2, 1, 0);
	mk_ssd1306_puts( 10, 20, ">> SET <<", 2, 1, 0);
	mk_ssd1306_puts( 10, 36, "  CHECK  ", 2, 1, 0);
	
	sei();
    while (1) 
    {	
		if(event == 0x01)
		{
			if(state == NORMAL){
				mk_ssdD1306_cls();
				mk_ssd1306_puts_P( 12,0, PSTR("a³alarm"), 2, 1, 0);
				if(option == SET) mk_ssd1306_puts( 10, 20, ">> SET <<", 2, 1, 0);
				else mk_ssd1306_puts( 10, 20, "   SET   ", 2, 1, 0);
				
				if(option == CHECK) mk_ssd1306_puts( 10, 36, ">>CHECK<<", 2, 1, 0);
				else mk_ssd1306_puts( 10, 36, "  CHECK  ", 2, 1, 0);
				
				/*if(option == QUIT) mk_ssd1306_puts( 10, 52, ">> QUIT <<", 2, 1, 0);
				else mk_ssd1306_puts( 10, 52, "   QUIT   ", 2, 1, 0);*/
			}else if(state == SETTING)
			{
				mk_ssdD1306_cls();
				mk_ssd1306_puts_P( 12,0, PSTR("a³alarm"), 2, 1, 0);
				mk_ssd1306_puts( 0, 24, "SET TIME", 2, 1, 0);
			}else if(state == SETTING_CONFIRMATION)
			{
				mk_ssdD1306_cls();
				mk_ssd1306_puts_P( 12,0, PSTR("a³alarm"), 2, 1, 0);
				mk_ssd1306_puts( 0, 16, " SET  set  ", 2, 1, 0);
				mk_ssd1306_puts( 0, 32, "BACK  back ", 2, 1, 0);
				mk_ssd1306_puts( 0, 48, " +/-  exit ", 2, 1, 0);
			}
			else if(state == ALARM_ON)
			{
				mk_ssdD1306_cls();
				mk_ssd1306_puts_P( 12,0, PSTR("a³alarm"), 2, 1, 0);
				mk_ssd1306_puts( 0, 24, " Wake up!", 2, 1, 0);
				mk_ssd1306_puts( 0, 42, "BACK - OK", 2, 1, 0);
			}
			event = 0x00;
		}
		
		MK_FX_EVENT();
		mk_ssd1306_display();
		
		service_buttons();
		
		if(displayFlag){
			if((state == NORMAL) || (state == ALARM_ON)){
				set_display(current_time[0],current_time[1],current_time[2],0);
				}else if(state == SETTING){
				set_display(buffered_time[0],buffered_time[1],0xff,current_choice+1);
				}else if(state == SHOW_ALARM){
				set_display(buffered_time[0],buffered_time[1],0xff,0);
			}
		}
    }
}

