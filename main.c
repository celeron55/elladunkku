//#define F_CPU 1000000L
#define F_CPU 128000L
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

/*

MCU: ATTINY25

FUSE SETTINGS:

lfuse=0xe4
hfuse=0xd6
//hfuse=0xdf
//hfuse=0xcf
efuse=0xff (default)

//lfuse=0x62 (default)
//hfuse=0xdf
//efuse=0xff (default)

I/O PORTS

PB5: (i) (reset)
PB4: (o) LCD RESET
PB3: (ia) buttons
PB2: (o) LCD SCLK / (sck)
PB1: (o) LCD SI / (miso)
PB0: (o) LCD D/C / (mosi)

OTHER

*/

#define LCD_DC_SET   PORTB |=  (1<<0);
#define LCD_DC_CLEAR PORTB &= ~(1<<0);
#define LCD_SI_SET   PORTB |=  (1<<1);
#define LCD_SI_CLEAR PORTB &= ~(1<<1);
#define LCD_SCLK_SET   PORTB |=  (1<<2);
#define LCD_SCLK_CLEAR PORTB &= ~(1<<2);
#define LCD_RESET_SET   PORTB |=  (1<<4);
#define LCD_RESET_CLEAR PORTB &= ~(1<<4);

///////////////////////////////////////////////////////

const uint8_t font[] PROGMEM = {
	0x1c,0x22,0x22,0x1c, //0
	0x00,0x02,0x3e,0x00, //1
	0x32,0x2a,0x2a,0x24, //2
	0x22,0x2a,0x2a,0x14, //3
	0x18,0x14,0x3e,0x10, //4
	0x2e,0x2a,0x2a,0x12, //5
	0x1c,0x2a,0x2a,0x10, //6
	0x02,0x32,0x0a,0x06, //7
	0x14,0x2a,0x2a,0x14, //8
	0x04,0x2a,0x2a,0x1c, //9
	0x22,0x14,0x1c,0x7f,0x1c,0x14,0x22,
	/*0x3c,0x12,0x12,0x3c, //A 40
	0x3e,0x2a,0x2a,0x14, //B
	0x1c,0x22,0x22,0x00, //C
	0x3e,0x22,0x22,0x1c, //D
	0x00,0x3e,0x2a,0x2a, //E
	0x00,0x3e,0x0a,0x0a, //F
	0x1c,0x22,0x2a,0x3a, //G
	0x3e,0x08,0x08,0x3e, //H
	0x00,0x22,0x3e,0x22, //I
	0x10,0x20,0x22,0x1e, //J
	0x3e,0x08,0x14,0x22, //K
	0x00,0x3e,0x20,0x20, //L
	0x3e,0x04,0x08,0x04, //M
	0x3e,0x04,0x08,0x3e, //N
	0x1c,0x22,0x22,0x1c, //O
	0x3e,0x12,0x12,0x0c, //P
	0x1c,0x22,0x22,0x5c, //Q
	0x3e,0x12,0x12,0x2c, //R
	0x24,0x2a,0x2a,0x12, //S
	0x00,0x02,0x3e,0x02, //T
	0x1e,0x20,0x20,0x1e, //U
	0x1e,0x20,0x18,0x06, //V
	0x1e,0x20,0x1c,0x20, //W
	0x36,0x08,0x08,0x36, //X
	0x06,0x28,0x28,0x1e, //Y
	0x00,0x32,0x2a,0x26, //Z*/
};

__attribute__((optimize(3)))
void lcd_byte(uint8_t c, uint8_t data_command)
{
	if(data_command) LCD_DC_SET
	else LCD_DC_CLEAR
	for(uint8_t i=0; i<8; i++){
		LCD_SI_SET
		if(!(c & (1<<(7-i)))) LCD_SI_CLEAR
		LCD_SCLK_SET
		LCD_SCLK_CLEAR
	}
}

void lcd_locate(uint8_t x, uint8_t y)
{
	lcd_byte(0x80+x, 0);
	lcd_byte(0x40+y, 0);
}

void lcd_locate8(uint8_t pos)
{
	lcd_byte(0x80+0x02+((pos&0x0f)<<3), 0);
	lcd_byte(0x40+(pos>>4), 0);
	//lcd_byte(0x40+0x01+(pos>>4), 0);
}

void lcd_print_font(uint8_t start, uint8_t end)
{
	uint8_t i;
	for(i=start; i<end; i++){
		lcd_byte(pgm_read_byte(&(font[i])), 1);
	}
}

void lcd_print(uint8_t c)
{
	/*if(c==' '){
		for(c=0; c<5; c++) lcd_byte(0x00, 1);
	}
	else*/ //if(c>='0' && c<='9'){
		lcd_print_font((c-'0')*4, (c-'0'+1)*4);
		lcd_byte(0x00, 1);
	//}
	/*else if(c>='A' && c<='Z'){
		lcd_print_font((c-'A')*4+40, (c-'A'+1)*4+40);
		if(c=='M') lcd_byte(0x3e, 1);
		else if(c=='W') lcd_byte(0x1e, 1);
		else lcd_byte(0x00, 1);
	}*/
}

/*void lcd_printstrP(PGM_P c)
{
	while(pgm_read_byte(c)){ lcd_print((uint8_t)pgm_read_byte(c)); c++; }
}*/

/*void lcd_put3digit(uint8_t i)
{
	lcd_print('0'+(i/100));
	lcd_print('0'+((i/10)%10));
	lcd_print('0'+(i%10));
}*/

void lcd_put5digit(uint16_t i)
{
	/*uint16_t j;
	uint16_t c=10000;
	do{
		j = i/c;
		lcd_print('0'+j);
		i -= c*j;
		c /= 10;
	}while(c != 10);
	lcd_print('0'+i);*/
	/*j = i/10000;
	lcd_print('0'+j);
	i -= 10000*j;
	j = i/1000;
	lcd_print('0'+j);
	i -= 1000*j;
	j = i/100;
	lcd_print('0'+j);
	i -= 100*j;
	j = i/10;
	lcd_print('0'+j);
	i -= 10*j;
	lcd_print('0'+i);*/
	lcd_print('0'+(i/10000));
	lcd_print('0'+((i/1000)%10));
	lcd_print('0'+((i/100)%10));
	lcd_print('0'+((i/10)%10));
	lcd_print('0'+(i%10));
}

void lcd_cls(void)
{
	/*uint8_t c;
	for(c=0;c<6;c++){
		lcd_clrrow(c);
	}
	lcd_locate(0,0);*/
	uint8_t c;
	for(c=0;c<84*3;c++){
		lcd_byte(0x00, 1);
		lcd_byte(0x00, 1);
	}
}

void lcd_init(void)
{
	LCD_RESET_CLEAR
	//_delay_ms(10);
	LCD_RESET_SET
	//_delay_ms(10);

	lcd_byte(0x21, 0);//Horizontal addressing, extended instruction set
	lcd_byte(0x80+0x48, 0);//LCD Vop
	lcd_byte(0x04+2, 0);//Temp coefficient
	lcd_byte(0x10+3, 0);//LCD bias system
	lcd_byte(0x20, 0);//Horizontal addressing, basic instruction set
	//lcd_byte(0x08+4, 0);//Normal mode
	//lcd_cls();
}

void lcd_powerdown(void)
{
	lcd_byte(0x21, 0);//Horizontal addressing, extended instruction set
	lcd_byte(0x80+0, 0);//LCD Vop 0
	lcd_byte(0x24, 0);//Horizontal addressing, basic instruction set, powerdown
}

////////////////////////////////////////////////////////

uint8_t read_adc(void)
{
	//   (Vcc) left-adj.
	ADMUX = (1<<ADLAR) | 3;
	//      2.56V                     left-adj.    PB3
	//ADMUX = (1<<REFS2) | (1<<REFS1) | (1<<ADLAR) | (1<<MUX1) | (1<<MUX0);
	//      1.1V           left-adj.    PB3
	//ADMUX = (1<<REFS1) | (1<<ADLAR) | (1<<MUX1) | (1<<MUX0);
	//     Vcc  left-adj.    PB2
	//ADMUX = 0 | (1<<ADLAR) | (1<<MUX0);
	ADCSRA |= (1<<ADSC);

	while(!(ADCSRA & (1<<ADIF)));
	uint8_t c = ADCH;
	ADCSRA |= (1<<ADIF); //clear flag
	return c;
}

enum{
	DIR_NONE = 0, 
	DIR_UP = -1, 
	DIR_DOWN = 1,
	DIR_LEFT = -2, 
	DIR_RIGHT = 2
};

int8_t getkey(void)
{
	uint8_t adcv;
	//uint8_t c = 0;
	for(;;){
		//if(c > 10) return DIR_NONE;
		//c++;
		adcv = read_adc();
		if(adcv < (uint8_t)(256.0*0.5/3.035)) return DIR_UP;
		//if(read_adc() != adcv) continue;
		//if(adcv == 0) adcv = 1;
		//else if(adcv == 255) adcv = 254;
		_delay_ms(1);
		uint8_t adcv2;
		adcv2 = read_adc();
		//if(adcv2 >= adcv - 1 || adcv2 <= adcv + 1) break;
		if(adcv2 == adcv) break;
	}

	//if(adcv < (uint8_t)(256.0*0.6/3.035)) return DIR_UP;
	if(adcv > (uint8_t)(256.0*(1.518-0.2)/3.035) && adcv < (uint8_t)(256.0*(1.518+0.2)/3.035)) return DIR_LEFT;
	if(adcv > (uint8_t)(256.0*(0.976-0.2)/3.035) && adcv < (uint8_t)(256.0*(0.976+0.2)/3.035)) return DIR_RIGHT;
	if(adcv > (uint8_t)(256.0*(2.011-0.2)/3.035) && adcv < (uint8_t)(256.0*(2.011+0.2)/3.035)) return DIR_DOWN;
	return DIR_NONE;
}

#define SNAKE_MAXLEN (5*10)
#define SNAKE_STARTLEN 4

//uint8_t g_highscore;
uint16_t g_highscore;
//4 MSB's = y, 4 LSB's = x
uint8_t g_snake[SNAKE_MAXLEN];
uint8_t g_snake_start;
uint8_t g_snake_end;
int8_t g_snake_dir;
uint8_t g_point_pos;
//uint8_t g_points;
uint16_t g_points;
uint8_t g_free_pos_count;
uint16_t g_random;
uint8_t g_day;

void point_newplace(void);
void draw_block(uint8_t pos, uint8_t b);
void snake_draw(uint8_t b);
void points_draw(void);

void initsnake(void)
{
	lcd_byte(0x08+4, 0);//Normal mode
	g_day = 1;
	uint8_t i;
	for(i=0; i<SNAKE_STARTLEN; i++){
		g_snake[i] = ((2)<<4)+(0+i);
	}
	g_snake_start = 0;
	g_snake_end = 0 + SNAKE_STARTLEN - 1;
	g_snake_dir = DIR_RIGHT;
	//g_free_pos_count = 1;
	g_free_pos_count = 5*10-SNAKE_STARTLEN;
	lcd_cls();
	for(i=0; i<6; i++){
		lcd_locate(83,i);
		lcd_byte(0xff, 1);
		lcd_byte(0xff, 1);
	}
	point_newplace();
	snake_draw(0xff);
	lcd_locate(0,0);
	lcd_put5digit(g_highscore);
	lcd_locate(29,0);
	lcd_put5digit(g_points);
	g_points = 0;
	points_draw();
}

uint8_t random(void)
{
	g_random = ((g_random * 9384)^0x2736);
	return (uint8_t)(g_random>>8);
}

void points_draw(void)
{
	//lcd_locate8((0<<4)+8);
	//lcd_locate(69,0);
	//lcd_put3digit(g_points);
	lcd_locate(59,0);
	lcd_put5digit(g_points);
}

enum {POS_EMPTY, POS_DIE, POS_POINT};

uint8_t snake_posinfo(uint8_t pos)
{
	if(g_point_pos != 0xff){
		if(pos == g_point_pos) return POS_POINT;
	}
	uint8_t i = g_snake_start;
	if(g_snake_end < g_snake_start){
		for(; i < SNAKE_MAXLEN; i++){
			if(g_snake[i] == pos) return POS_DIE;
		}
		i = 0;
	}
	for(; i <= g_snake_end; i++){
		if(g_snake[i] == pos) return POS_DIE;
	}
	return POS_EMPTY;
}

void point_newplace(void)
{
	uint8_t x=0;
	uint8_t y=1;
	uint8_t i = 0;
	uint8_t c = random()%g_free_pos_count;
	for(;;){
		if(snake_posinfo((y<<4)+x) == POS_EMPTY){
			if(i == c){
				g_point_pos = (y<<4)+x;
				break;
			}
			//if(i == 10*5) return;
			i++;
		}
		x++;
		if(x>=10){ y++; x=0; }
		if(y>=6) y = 1;
	}
	lcd_locate8(g_point_pos);
	lcd_print_font(40, 40+7);
	/*lcd_byte(0x22, 1);
	lcd_byte(0x14, 1);
	lcd_byte(0x1c, 1);
	lcd_byte(0x7f, 1);
	lcd_byte(0x1c, 1);
	lcd_byte(0x14, 1);
	lcd_byte(0x22, 1);*/
}

void draw_block(uint8_t pos, uint8_t b)
{
	lcd_locate8(pos);
	uint8_t i;
	for(i=0; i<8; i++) lcd_byte(b, 1);
}

void snake_draw(uint8_t b)
{
	uint8_t i = g_snake_start;
	if(g_snake_end < g_snake_start){
		for(; i < SNAKE_MAXLEN; i++){
			draw_block(g_snake[i], b);
		}
		i = 0;
	}
	for(; i <= g_snake_end; i++){
		draw_block(g_snake[i], b);
	}
}

//returns 1 if dead
uint8_t snake_move_and_draw(void)
{
	static uint8_t dying = 0;
	uint8_t y = g_snake[g_snake_end] >> 4;
	uint8_t x = g_snake[g_snake_end] & 0x0f;
	uint8_t info = POS_EMPTY;
	switch(g_snake_dir){
	case DIR_UP:
		if(y == 1) info = POS_DIE;
		y--;
		break;
	case DIR_DOWN:
		if(y == 5) info = POS_DIE;
		y++;
		break;
	case DIR_LEFT:
		if(x == 0) info = POS_DIE;
		x--;
		break;
	case DIR_RIGHT:
		if(x == 9) info = POS_DIE;
		x++;
		break;
	default:
		return 0; //no move
	}
	uint8_t pos_new = (y<<4) + x;
	if(info == POS_EMPTY)
		info = snake_posinfo(pos_new);
	if(info == POS_DIE){
		if(dying == 0){
			dying = 1;
			return 0;
		}
		return 1;
	}

	dying = 0;

	draw_block(pos_new, 0xff);

	if(info == POS_POINT){
		g_points++;
		points_draw();
	}

	g_snake_end += 1;
	if(g_snake_end == SNAKE_MAXLEN) g_snake_end = 0;

	if(info != POS_POINT){
		draw_block(g_snake[g_snake_start], 0x00);
		g_snake_start++;
		if(g_snake_start == SNAKE_MAXLEN) g_snake_start = 0;
	}
	else g_free_pos_count--;

	g_snake[g_snake_end] = pos_new;
	
	if(g_free_pos_count == 0){
		snake_draw(0x00);
		g_snake_start = g_snake_end;
		g_free_pos_count = 10*5-1;
		snake_draw(0xff);
		g_snake_dir = DIR_NONE;
		g_day = !g_day;
		if(g_day) lcd_byte(0x08+4, 0);//Normal mode
		else      lcd_byte(0x08+5, 0);//Inverse mode
	}

	if(info == POS_POINT){
		point_newplace(); //has to be done after extending snake's head to current position
	}
	//lcd_locate(83,5);
	lcd_locate8((0<<4)+4);
	return 0;
}

volatile uint8_t g_counter0 = 0;

ISR(TIM0_COMPB_vect)
{
	TCNT0 = 0;
	g_counter0++;
}

//uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

/*void get_mcusr(void) \
  __attribute__((naked)) \
  __attribute__((section(".init3")));
void get_mcusr(void)
{
  //mcusr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}*/

int main(void)
{
	//wdt_enable(WDTO_1S);

	// I/O ports

	DDRB = 0x17;
	PORTB = 0x00 | (1<<5);
	//input logic disable on adc3
	DIDR0 = (1<<ADC3D);

	//timer0
	
	//set clock divider to 1024
	TCCR0B |= (1<<CS02) | (1<<CS00);
	//compare value 49 -> 20Hz
	//OCR0B = 49;
	OCR0B = 3;
	//enable compare match B interrupt
	TIMSK |= (1<<OCIE0B);

	//timer1

	//      CK/64
	//TCCR1 = (1<<CS12) | (1<<CS11) | (1<<CS10);
	//overflow interrupt
	//TIMSK |= (1<<TOIE1);

	//external interrupts
	
	//enable pin change interrupt
	//GIMSK = (1<<PCIE);
	//enable only on PB3
	//PCMSK = (1<<3);
	
	//adc
	
	//       enable      clk/16
	//ADCSRA = (1<<ADEN) | (1<<ADPS2);
	//       enable      clk/2
	ADCSRA = (1<<ADEN) | (1<<ADPS0);

	/*//if it's a watchdog reset
	if(mcusr_mirror & (1<<WDRF)){
		if(getkey() == DIR_NONE){
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_mode();
		}
	}*/

	//wdt_enable(WDTO_4S);

	//g_highscore = eeprom_read_byte(0);
	//if(g_highscore == 255) g_highscore = 0;
	g_highscore = eeprom_read_word(0);
	if(g_highscore == 0xffff) g_highscore = 0;

start:
	
	lcd_init();

	//lcd_locate(10,2);
	//lcd_locate8((2<<4)+1);
	//lcd_printstrP(PSTR("C55 PRESENTS"));
	//_delay_ms(1000);

	sei();

	for(;;){

		initsnake();

		//main loop
		
		g_counter0 = 0;
		//TCNT0 = 0;
		for(;;){
			int8_t lastdir_inv = -g_snake_dir;
			while(g_counter0 < 6){
			//while(TCNT0 < 6*3){
				int8_t key = getkey();
				if(key == DIR_NONE) continue;
				if(key == lastdir_inv) continue;
				g_snake_dir = key;
				g_random += g_counter0;
			}
			TCNT0 = 0;
			g_counter0 = 0;
			uint8_t ret = snake_move_and_draw();
			if(ret == 1){
				snake_draw(0xaa);
				draw_block(g_snake[g_snake_end], 0xff);
				//lcd_locate(83,5);
				lcd_locate8((0<<4)+4);
				if(g_points > g_highscore){
					g_highscore = g_points;
					//eeprom_write_byte(0, g_highscore);
					eeprom_write_word(0, g_highscore);
				}
				_delay_ms(200);
				break;
			}
		}
		
		while(getkey() != DIR_NONE);
		//TCNT0 = 0;
		g_counter0 = 0;
		while(getkey() == DIR_NONE){
			if(g_counter0 >= 200){
				cli();
				
				/*lcd_cls();
				lcd_locate8((1<<4)+0);
				lcd_printstrP(PSTR("GREETINGS  ELLA  TEJEEZ  3210"));
				_delay_ms(2000);*/
				/*lcd_locate8((1<<4)+1);
				lcd_printstrP(PSTR("GREETINGS ELLA TEJEEZ"));
				lcd_locate8((2<<4)+1);
				lcd_printstrP(PSTR("ELLA"));
				lcd_locate8((3<<4)+1);
				lcd_printstrP(PSTR("TEJEEZ"));*/
				/*lcd_cls();
				lcd_locate8((1<<4)+3);
				lcd_put5digit(2048);
				lcd_locate8((2<<4)+3);
				lcd_put5digit(128);
				lcd_locate8((3<<4)+3);
				lcd_put5digit(55);
				_delay_ms(2000);*/
				
				lcd_cls();
				lcd_powerdown();

				ADCSRA = 0 | (1<<ADPS0);
				DIDR0 = 0;
				CLKPR = 0x80;
				CLKPR = (1<<CLKPS3); //256
				while(PINB & (1<<3));
				//while(getkey() == DIR_NONE);
				CLKPR = 0x80;
				CLKPR = 0;
				DIDR0 = (1<<ADC3D);
				ADCSRA = (1<<ADEN) | (1<<ADPS0);

				goto start;
			}
		}
	}

	return 0;
}

