/**
 * elladunkku
 * Use makefile for avr
 *
 * For SDL simulation:
 * gcc main.c -std=c99 -Os -DSDL -lSDL -o elladunkku
 */

//#define F_CPU 1000000L
#define F_CPU 128000L

#if !SDL
# include <avr/interrupt.h>
# include <util/delay.h>
# include <avr/io.h>
# include <avr/pgmspace.h>
# include <avr/wdt.h>
# include <avr/sleep.h>
# include <avr/eeprom.h>
# include <stdbool.h>
# include <string.h>
#else
# define _DEFAULT_SOURCE
# include <stdbool.h>
# include <string.h>
# include <stdint.h>
# include <unistd.h>
# include <SDL/SDL.h>
# define PROGMEM
# define pgm_read_byte(x) *x
# define sei()
# define _delay_ms(x) SDL_Delay(x)
static SDL_Surface *screen;
#endif

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
	// 0
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
	// 40
	// 0
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	// 1
	0b00000000,
	0b11100000,
	0b11110000,
	0b11110000,
	0b11110000,
	0b11100000,
	0b00000000,
	0b00000000,
	// 2
	0b00000000,
	0b00000000,
	0b01100000,
	0b10010000,
	0b10010000,
	0b01100000,
	0b00000000,
	0b00000000,
	// 3
	0b11000000,
	0b01000000,
	0b01110000,
	0b00010000,
	0b00011100,
	0b00000100,
	0b00000100,
	0b00000000,
	// 4
	0b00000000,
	0b10010000,
	0b11110100,
	0b11111110,
	0b11110100,
	0b10010000,
	0b00000000,
	0b00000000,
	// 5
	0b00001000,
	0b01000100,
	0b00100010,
	0b00010100,
	0b00001000,
	0b00000100,
	0b00001000,
	0b00010000,
	// 6
	0b00000000,
	0b00111100,
	0b00100010,
	0b11111010,
	0b01000110,
	0b00111100,
	0b00000000,
	0b00000000,
	// 7
	0b00000000,
	0b00000000,
	0b00100000,
	0b00000100,
	0b01000000,
	0b00010000,
	0b00000000,
	0b00000000,
	// 8
	0b00000000,
	0b00000110,
	0b01000110,
	0b10100100,
	0b10100100,
	0b00011000,
	0b00000000,
	0b00000000,
	// 9
	0b00010000,
	0b00100001,
	0b10101110,
	0b01111100,
	0b10101110,
	0b00100001,
	0b00010000,
	0b00000000,
	// 10
	0b00000000,
	0b10000000,
	0b11111000,
	0b10000100,
	0b11111000,
	0b10000000,
	0b00000000,
	0b00000000,
	// 11
	0b00001010,
	0b00010100,
	0b00101010,
	0b10110011,
	0b01111110,
	0b10001000,
	0b00100100,
	0b00001010,
	// 12: player
	0b01000000,
	0b00100000,
	0b10101100,
	0b01111100,
	0b10101100,
	0b00100000,
	0b01000000,
	0b00000000,
	/*0x3c,0x12,0x12,0x3c, //A
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
static void lcd_byte(uint8_t c, uint8_t data_command)
{
#if !SDL
	if(data_command) LCD_DC_SET
	else LCD_DC_CLEAR
	for(uint8_t i=0; i<8; i++){
		LCD_SI_SET
		if(!(c & (1<<(7-i)))) LCD_SI_CLEAR
		LCD_SCLK_SET
		LCD_SCLK_CLEAR
	}
#else
	static uint8_t x = 0, y = 0;
	if (data_command == 0) {
		if (c >= 0x80)
			x = (c - 0x80) % 80;
		else if (c >= 0x40)
			y = ((c - 0x40) * 8) % 48;
	} else if (data_command == 1) {
		for (uint8_t i = 0; i < 8; ++i) {
			if (!(c & (1 << i)))
				continue;

			uint32_t c = SDL_MapRGB(screen->format, 0, 0, 0);
			uint32_t *p32 = (uint32_t*)screen->pixels + ((y + i) * 80) + x;
			*p32 = c;
		}
		x = (x + 1) % 80;
	}
#endif
}

static void lcd_locate(uint8_t x, uint8_t y)
{
	lcd_byte(0x80+x, 0);
	lcd_byte(0x40+y, 0);
}

/*static void lcd_locate8(uint8_t pos)
{
	lcd_byte(0x80+0x02+((pos&0x0f)<<3), 0);
	//lcd_byte(0x40+(pos>>4), 0);
	lcd_byte(0x40+0x01+(pos>>4), 0);
}*/

static void lcd_print_font(uint8_t start, uint8_t end)
{
	uint8_t i;
	for(i=start; i<end; i++){
		lcd_byte(pgm_read_byte(&(font[i])), 1);
	}
}

static void lcd_print_sprite(uint8_t sprite)
{
	uint8_t sprite8 = sprite<<3;
	lcd_print_font(40 + sprite8, 48 + sprite8);
}

static void lcd_print(uint8_t c)
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

/*static void lcd_printstrP(PGM_P c)
{
	while(pgm_read_byte(c)){ lcd_print((uint8_t)pgm_read_byte(c)); c++; }
}*/

/*static void lcd_put3digit(uint8_t i)
{
	lcd_print('0'+(i/100));
	lcd_print('0'+((i/10)%10));
	lcd_print('0'+(i%10));
}*/

static void lcd_put5digit(uint16_t i)
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
	//lcd_print('0'+(i/10000));
	lcd_print('0'+((i/1000)%10));
	lcd_print('0'+((i/100)%10));
	lcd_print('0'+((i/10)%10));
	lcd_print('0'+(i%10));
}

static void lcd_cls(void)
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

static void lcd_init(void)
{
#if !SDL
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
#endif
}

static void lcd_powerdown(void)
{
	lcd_byte(0x21, 0);//Horizontal addressing, extended instruction set
	lcd_byte(0x80+0, 0);//LCD Vop 0
	lcd_byte(0x24, 0);//Horizontal addressing, basic instruction set, powerdown
}

////////////////////////////////////////////////////////

enum{
	DIR_NONE = 0,
	DIR_UP = -1,
	DIR_DOWN = 1,
	DIR_LEFT = -2,
	DIR_RIGHT = 2
};

#if !SDL

static uint8_t read_adc(void)
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
		//if(adcv2 >= adcv - 2 || adcv2 <= adcv + 2) break;
		//if(adcv2 >= adcv - 1 || adcv2 <= adcv + 1) break;
		if(adcv2 == adcv) break;
	}

	//if(adcv < (uint8_t)(256.0*0.6/3.035)) return DIR_UP;
	if(adcv > (uint8_t)(256.0*(1.518-0.2)/3.035) && adcv < (uint8_t)(256.0*(1.518+0.2)/3.035)) return DIR_LEFT;
	if(adcv > (uint8_t)(256.0*(0.976-0.2)/3.035) && adcv < (uint8_t)(256.0*(0.976+0.2)/3.035)) return DIR_RIGHT;
	if(adcv > (uint8_t)(256.0*(2.011-0.2)/3.035) && adcv < (uint8_t)(256.0*(2.011+0.2)/3.035)) return DIR_DOWN;
	return DIR_NONE;
}

#else

SDL_Event g_event;

int8_t getkey(void)
{
	switch(g_event.type){
		case SDL_KEYDOWN:
			switch(g_event.key.keysym.sym) {
				case SDLK_LEFT: return DIR_LEFT;
				case SDLK_RIGHT: return DIR_RIGHT;
				case SDLK_UP: return DIR_UP;
				case SDLK_DOWN: return DIR_DOWN;
				default:break;
			}
		default:break;
	}
	return DIR_NONE;
}

#endif

/* =============== GAME STUFF ============== */

#define MAP_W 10
#define MAP_H 5
#define MAP_SIZE (MAP_W*MAP_H)
#define MAX_TILES 16
#define GROUND_TILES 3
#define ENEMY_TILES 4

enum {
	EMPTY,
	BUSH,
	STONE,
	STAIRS,
	TREE, //4
	MOUNTAIN,
	DOOR,
	BERRY,
	SNAKE, // 8
	GOBLIN,
	ELLA,
	DRAGON,
};

volatile uint8_t g_counter0 = 0;

uint8_t g_map[MAP_W*MAP_H];
int8_t g_next_dir;
uint8_t g_player_position_i = 0;
uint8_t g_level = 1;
uint16_t g_seed = 0;
int8_t g_hp;

static uint8_t
get_pos(uint8_t x, uint8_t y)
{
   return (x + (MAP_W * y));
}

static void
to_pos(uint8_t p, uint8_t *ox, uint8_t *oy)
{
   *oy = p / MAP_W;
   *ox = p - (MAP_W * *oy);
}

static bool
valid_rooms(const uint8_t rooms[2])
{
   return (rooms[0] + 1 != rooms[1] && rooms[0] - 1 != rooms[1] && rooms[0] != rooms[1]);
}

static void
generate_dungeon(uint8_t tiles[MAP_SIZE], uint16_t seed, uint8_t level, uint8_t *out_pos)
{
   memset(tiles, 0, MAP_SIZE);

   uint8_t hash = 0;
   for (uint8_t t = 0; t < MAP_SIZE; t += MAP_SIZE / 8) {
      hash += ((hash << 5) + hash) + ((t & seed) ^ seed);
      tiles[t] = (hash ^ 8) % (STONE + 1);
   }

   for (uint8_t t = 0; t < 2 + (level < 32 ? level % 4 : level / 4); ++t) {
      const uint8_t types = 1 + (level / 8 > 3 ? 3 : level / 8);
      const uint8_t rand = ((hash ^ t) + (seed ^ level));
      tiles[rand % MAP_SIZE] = SNAKE + (rand % types);
   }

   tiles[(hash + seed) % MAP_SIZE] = BERRY;

   const uint8_t rooms[2] = { 1 + (hash ^ seed) % (MAP_W - 2), 1 + ((hash | seed)) % (MAP_W - 2) };
   for (uint8_t r = 0; r < 1 + valid_rooms(rooms); ++r) {
      for (uint8_t y = 0; y < MAP_H; ++y)
         tiles[get_pos(rooms[r], y)] = (((hash ^ r) % MAP_W) > r ? TREE : MOUNTAIN);
      tiles[get_pos(rooms[r], (hash ^ r) % MAP_H)] = DOOR;
   }

   const uint8_t s = hash % MAP_SIZE, y = ((hash & seed) % MAP_SIZE);
   const uint8_t g = (s != y ? y : (s >= MAP_SIZE ? 0 : s + 1));

   tiles[g] = STAIRS;
   *out_pos = s;
}

static void draw_stats(void)
{
	lcd_locate(0, 0);
	lcd_put5digit(g_level);
	lcd_locate(28, 0);
	lcd_put5digit(g_hp);
	/*lcd_locate(59, 0);
	lcd_put5digit(g_seed);*/
}

static void draw_game(void);

static void make_current_dungeon(void)
{
	generate_dungeon(g_map, g_seed, g_level, &g_player_position_i);
}

static void next_level(bool init_game)
{
	if(init_game){
		g_level = 1;
		g_hp = 20;
		lcd_byte(0x08+4, 0);//Normal mode
		lcd_cls();
	} else {
		g_level++;
		g_seed += g_counter0;
	}
	make_current_dungeon();
	draw_game();
}

static void draw_game(void)
{
#if SDL
	SDL_Rect r;
	r.x = r.y = 0;
	r.w = screen->w;
	r.h = screen->h;
	uint32_t c = SDL_MapRGB(screen->format, 128, 255, 128);
	SDL_FillRect(screen, &r, c);
#endif

	uint8_t i=0;
	for(uint8_t y=0; y<MAP_H; y++){
		lcd_locate(2, y+1);
		for(uint8_t x=0; x<MAP_W; x++){
			if(get_pos(x, y) == g_player_position_i){
				lcd_print_sprite(12);
			} else {
				uint8_t t = g_map[i];
				lcd_print_sprite(t);
			}
			i++;
		}
	}
	draw_stats();

#if SDL
	SDL_Flip(screen);
#endif
}

// Returns true if move is valid
static uint8_t move_player(int8_t key)
{
	uint8_t x, y;
	to_pos(g_player_position_i, &x, &y);
	/*uint8_t old_x = x;
	uint8_t old_y = y;*/
	switch(key){
	case DIR_UP:
		y--;
		break;
	case DIR_DOWN:
		y++;
		break;
	case DIR_LEFT:
		x--;
		break;
	case DIR_RIGHT:
		x++;
		break;
	default:
		return 0; //no move
	}
	g_seed += (x<<4) + y;

	if(y == 255 || x == 255 || y == MAP_H || x == MAP_W)
		return 0; // Map boundaries

	uint8_t i = get_pos(x, y);
	uint8_t t = g_map[i];

	if(t == MOUNTAIN || t == TREE){
		return 0; // Can't walk on these tiles
	}
	if(t >= SNAKE){
		// TODO: Require hitting enemy more than once
		g_map[i] = 0;
		return 1;
	}

	// TODO: Enable this
	/*// Immediately erase player
	lcd_locate(2+old_x*8, old_y+1);
	lcd_print_sprite(0);
	
	// Immediately draw player
	lcd_locate(2+x*8, y+1);
	lcd_print_sprite(12);*/

	g_player_position_i = get_pos(x, y);

	if(t == STAIRS){
		next_level(false);
		return 0;
	}
	if(t == BERRY){
		g_hp += 3;
		g_map[i] = EMPTY;
		return 1;
	}
	if(t == STONE || t == BUSH)
		return 2;
	return 1;
}

const int8_t possible_moves[] PROGMEM = {-10, -1, 1, 10};

// Returns new position, can have side effects
static int8_t ai_action(int8_t current_i, uint8_t t)
{
	uint8_t current_x, current_y;
	to_pos(current_i, &current_x, &current_y);
	uint8_t player_x, player_y;
	to_pos(g_player_position_i, &player_x, &player_y);
	int8_t pdx = player_x - current_x;
	int8_t pdy = player_y - current_y;
	uint8_t start_off = 0;
	if(pdx < -1)
		start_off = 1;
	else if(pdx > 1)
		start_off = 2;
	else if(pdy > 1)
		start_off = 3;
	else if(pdy > -1){
		g_hp -= t - SNAKE + 1;
		return current_i;
	}
	for(uint8_t i=start_off; i<start_off+4; i++){
		uint8_t actual_i = i % 4;
		int8_t try_i = current_i + pgm_read_byte(&(possible_moves[actual_i]));
		if(try_i < 0 || try_i >= MAP_SIZE)
			continue;
		if(g_map[try_i] != EMPTY)
			continue;
		return try_i;
	}
	return current_i;
}

static void step_enemies(void)
{
	uint8_t acted[4] = {255, 255, 255, 255};
	uint8_t num_acted = 0;

	for(int8_t i=0; i<MAP_SIZE; i++){
		uint8_t t = g_map[i];
		if(t < SNAKE)
			continue;
		for(uint8_t acted_i=0; acted_i<sizeof acted; acted_i++){
			if(acted[acted_i] == i)
				goto skip_act;
		}
		int8_t new_i = ai_action(i, t);
		if(new_i != i){
			uint8_t t2 = g_map[new_i];
			g_map[new_i] = t;
			g_map[i] = t2;
		}
		if(num_acted < sizeof acted)
			acted[num_acted++] = new_i;
skip_act:
		continue;
	}
}

#if !SDL
ISR(TIM0_COMPB_vect)
{
	TCNT0 = 0;
	g_counter0++;
}
#endif

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
#if !SDL
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

#else

	SDL_Init(SDL_INIT_VIDEO);
	if (!(screen = SDL_SetVideoMode(80, 48, 32, SDL_SWSURFACE)))
		return EXIT_FAILURE;

	SDL_WM_SetCaption("elladunkku", NULL);

#endif

	/*g_highscore = eeprom_read_byte(0);
	if(g_highscore == 0xff) g_highscore = 0;*/
	/*g_highscore = eeprom_read_word(0);
	if(g_highscore == 0xffff) g_highscore = 0;*/

start:

	lcd_init();

	//lcd_locate(10,2);
	//lcd_locate8((2<<4)+1);
	//lcd_printstrP(PSTR("C55 PRESENTS"));
	//_delay_ms(1000);

	sei();

	for(;;){

		next_level(true);

		//main loop

		g_counter0 = 0;
		//TCNT0 = 0;
		for(;;){
#if SDL
			++g_counter0;
			SDL_WaitEvent(&g_event);
			if (g_event.type == SDL_QUIT)
				goto quit;
#endif

			// TODO: If nothing happens for a long time, go to sleep

			int8_t key = getkey();
			if(key == DIR_NONE)
				continue;

			uint8_t num_enemy_moves = move_player(key);
			for(uint8_t i=0; i<num_enemy_moves; i++){
				step_enemies();
				draw_game();
			}

			if(g_hp <= 0){
				_delay_ms(5000);
				while(getkey() == DIR_NONE);
				next_level(true); // Reset game
			}

			/*int8_t lastdir_inv = -g_next_dir;
			while(g_counter0 < 6){
			//while(TCNT0 < 6*3){
				int8_t key = getkey();
				if(key == DIR_NONE) continue;
				if(key == lastdir_inv) continue;
				g_next_dir = key;
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
			}*/
		}

#if !SDL
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
#endif
	}

#if SDL
quit:
	SDL_Quit();
#endif
	return 0;
}

