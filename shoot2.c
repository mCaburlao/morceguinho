
/*
A shoot-em-up game.
Uses CHR RAM to draw attacker tiles at 8 different pixels,
then animates them in the nametable.
Sprites are used for the player, missiles, attackers, and stars.
*/

#include <stdlib.h>
#include <string.h>
#include <nes.h>

#include "neslib.h"

#define NES_MAPPER 2	// mapper 2 (UxROM mapper)
#define NES_CHR_BANKS 0	// CHR RAM

// APU (sound) support
#include "apu.h"
//#link "apu.c"

// BCD arithmetic support
#include "bcd.h"
//#link "bcd.c"

// VRAM update buffer
#include "vrambuf.h"
//#link "vrambuf.c"

#define COLS 32
#define ROWS 28

//#define DEBUG_FRAMERATE

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[32] = { 
  0x0F,

  0x11,0x24,0x3C, 0x00,
  0x01,0x15,0x25, 0x00,
  0x01,0x10,0x20, 0x00,
  0x06,0x16,0x26, 0x00,
  
  0x11,0x24,0x3C, 0x00,
  0x01,0x15,0x25, 0x00,
  0x31,0x35,0x3C, 0x00,
  0x26,0x02,0x31
};

#define COLOR_PLAYER		3
#define COLOR_FORMATION		1
#define COLOR_ATTACKER		1
#define COLOR_MISSILE		3
#define COLOR_BOMB		2
#define COLOR_SCORE		2
#define COLOR_EXPLOSION		3

/*{w:8,h:8,bpp:1,count:128,brev:1,np:2,pofs:8,remap:[0,1,2,4,5,6,7,8,9,10,11,12]}*/
const char TILESET[128*8*2] = {
// font (0..63)
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x7C,0x7C,0x7C,0x38,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6C,0x6C,0x48,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6C,0xFE,0x6C,0xFE,0x6C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0xFE,0xD0,0xFE,0x16,0xFE,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCE,0xDC,0x38,0x76,0xE6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x6C,0x7C,0xEC,0xEE,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x38,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x70,0x70,0x70,0x70,0x70,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x38,0x38,0x38,0x38,0x38,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6C,0x38,0x6C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x38,0xFE,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x1E,0x3C,0x78,0xF0,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x7C,0xEE,0xEE,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x78,0x38,0x38,0x38,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x0E,0x7C,0xE0,0xEE,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x0E,0x3C,0x0E,0x0E,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x7E,0xEE,0xEE,0xFE,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0xE0,0xFC,0x0E,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xE0,0xFC,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0xEE,0x1C,0x1C,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0x7C,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xEE,0x7E,0x0E,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00,0x60,0x60,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x38,0x70,0x70,0x38,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x38,0x1C,0x1C,0x38,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0x1C,0x38,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x7C,0xEE,0xEE,0xEE,0xE0,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xEE,0xEE,0xFE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0xEE,0xFC,0xEE,0xEE,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xE0,0xE0,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0xEC,0xEE,0xEE,0xEE,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0xE0,0xF0,0xE0,0xE0,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0xE0,0xF8,0xE0,0xE0,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xE0,0xEE,0xEE,0xEE,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xFE,0xEE,0xEE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x38,0x38,0x38,0x38,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x0E,0x0E,0x0E,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xFC,0xF8,0xEC,0xEE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0xE0,0xE0,0xE0,0xEE,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC6,0xEE,0xFE,0xFE,0xEE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCE,0xEE,0xFE,0xFE,0xEE,0xE6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xEE,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xFC,0xEE,0xEE,0xEE,0xFC,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xEE,0xEE,0xEE,0xEC,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0xEE,0xEE,0xEE,0xFC,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xE0,0x7C,0x0E,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x38,0x38,0x38,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xEE,0xEE,0xEE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xEE,0x6C,0x38,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xFE,0xFE,0xEE,0xC6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0x7C,0x38,0x7C,0xEE,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xEE,0xEE,0x7C,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x1C,0x38,0x70,0xE0,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6C,0x92,0x82,0x82,0x44,0x28,0x10,0x00,0x6C,0xFE,0xFE,0xFE,0x7C,0x38,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*{w:16,h:16,bpp:1,count:16,brev:1,np:2,pofs:8,remap:[5,0,1,2,4,6,7,8,9,10,11,12]}*/  
// formation enemy (64,66)
0x00,0x00,0x40,0xE0,0xB0,0x08,0x00,0x00,0x00,0x44,0xA4,0x17,0x0D,0x05,0x02,0x00,
0x00,0x18,0x38,0x38,0x28,0x00,0x00,0x00,0x18,0x24,0x44,0x47,0x45,0x45,0x02,0x00,
0x00,0x00,0x04,0x0E,0x1A,0x20,0x00,0x00,0x00,0x44,0x4A,0xD0,0x60,0x40,0x80,0x00,
0x00,0x30,0x38,0x38,0x28,0x00,0x00,0x00,0x30,0x48,0x44,0xC4,0x44,0x44,0x80,0x00,
// attackers (68)
0x00,0x00,0x00,0x00,0x00,0x04,0x4C,0x78,0x00,0x00,0x00,0x01,0x02,0x02,0x03,0x06,
0x30,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x4A,0x50,0x20,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x10,0x19,0x0F,0x00,0x00,0x00,0x40,0xA0,0xA0,0xE0,0x30,
0x06,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x29,0x05,0x02,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x40,0xE4,0x7C,0x70,0x00,0x00,0x00,0x00,0x01,0x02,0x82,0x8F,0x72,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x10,0x19,0x0F,0x00,0x00,0x00,0x40,0xA0,0xA0,0xE0,0x30,
0x06,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x29,0x05,0x02,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x60,0x30,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x41,0x42,0x3E,0x03,0x02,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x10,0x1C,0x0E,0x00,0x00,0x00,0x40,0xA0,0xA0,0xE0,0x30,
0x06,0x03,0x03,0x00,0x00,0x00,0x00,0x00,0x28,0x04,0x04,0x03,0x00,0x00,0x00,0x00,
  
0x00,0x60,0x78,0x0C,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x71,0x0A,0x06,0x03,0x02,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x10,0x09,0x07,0x00,0x00,0x00,0x40,0xA0,0xA0,0xF0,0x28,
0x07,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x05,0x02,0x00,0x00,0x00,0x00,0x00,

0x00,0x31,0x58,0x0C,0x04,0x00,0x00,0x00,0x00,0x00,0x20,0x51,0x0A,0x06,0x03,0x02,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x55,0x00,0x00,0x00,0x19,0x0F,0x06,0x00,0x00,0x00,0x40,0xA0,0xA0,0xF0,0x29,
0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x02,0x00,0x00,0x00,0x00,0x00,0x00,
  
0x00,0x80,0xC0,0x60,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x81,0x42,0x3E,0x03,0x02,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x01,0x01,0x03,0x06,0x1E,0x0C,0x00,0x00,0x00,0x00,0x40,0xA1,0xA1,0xF2,0x2C,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  
0x00,0x00,0x00,0x00,0x04,0x58,0x70,0x20,0x00,0x00,0x00,0x01,0x02,0x06,0x0B,0x52,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x10,0x0D,0x07,0x02,0x00,0x00,0x00,0x40,0xA0,0xB0,0xE8,0x25,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x10,0x10,0x03,0x07,0x00,0x00,0x00,0x00,0x03,0x0F,0x1F,0x1F,
0x24,0x40,0x00,0x00,0x07,0x63,0x41,0x00,0x3F,0x7F,0x7F,0x7F,0x78,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x08,0x08,0xC0,0xE0,0x00,0x00,0x00,0x00,0xC0,0xF0,0xF8,0xF8,
0x24,0x02,0x00,0x00,0xE0,0xC6,0x82,0x00,0xFC,0xFE,0xFE,0xFE,0x1E,0x00,0x00,0x00,
// player ship (96)
0x10,0x08,0x00,0x10,0x08,0x5A,0x3C,0x5A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x3C,0x24,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x38,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x10,0x38,0x10,0x00,0x00,0x04,
0x00,0x00,0x80,0x10,0x38,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x20,
// bullet (100)
0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
// stars (102)
0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
// explosions (112)
0x00,0x00,0x00,0x08,0x04,0x03,0x06,0x05,0x00,0x00,0x00,0x04,0x02,0x04,0x00,0x18,
0x05,0x06,0x07,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x08,0x09,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x08,0x90,0xE0,0x90,0xD0,0x00,0x00,0x00,0x10,0x20,0x10,0x00,0x0C,
0xF0,0x20,0xE0,0x80,0x00,0x00,0x00,0x00,0x0C,0x10,0x18,0x48,0x00,0x00,0x00,0x00,

0x00,0x04,0x12,0x0C,0x42,0x30,0x00,0x19,0x00,0x00,0x00,0x02,0x00,0x06,0x08,0x00,
0x11,0x04,0x0C,0x00,0x21,0x46,0x00,0x00,0x04,0x10,0x02,0x0C,0x00,0x00,0x00,0x00,
0x00,0x40,0x08,0x91,0xA2,0x04,0x80,0xCC,0x00,0x00,0x00,0x20,0x00,0x30,0x08,0x00,
0xC4,0x10,0x1A,0x80,0x20,0x10,0x00,0x00,0x10,0x04,0xA0,0x18,0x80,0x00,0x00,0x00,
 
0x61,0x20,0x00,0x04,0x00,0xC4,0x08,0x10,0x00,0x00,0x04,0x02,0x09,0x02,0x10,0x04,
0x10,0x00,0x42,0xC8,0x80,0x00,0x10,0x30,0x24,0x10,0x11,0x02,0x00,0x00,0x00,0x00,
0x86,0xC4,0x00,0x10,0x01,0x10,0x08,0x04,0x00,0x00,0x10,0x20,0x48,0x20,0x04,0x10,
0x04,0x00,0x20,0x89,0x00,0x84,0xC6,0x43,0x12,0x04,0x44,0x20,0x00,0x00,0x00,0x00,
  
0x00,0x00,0x02,0x08,0x00,0x06,0x02,0x00,0x00,0x40,0x08,0x01,0x11,0x11,0x00,0x38,
0x30,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x10,0x00,0x04,0x40,0x00,0x00,
0x00,0x00,0x20,0x08,0x00,0x30,0x20,0x00,0x02,0x00,0x08,0x40,0x44,0x44,0x00,0x0E,
0x06,0x20,0x80,0x80,0x00,0x00,0x00,0x00,0x08,0x80,0x04,0x00,0x00,0x11,0x00,0x00,  
};

#define CHAR(x) ((x)-' ')
#define BLANK 0

void clrscr() {
  vrambuf_clear();
  ppu_off();
  vram_adr(NAMETABLE_A);
  vram_fill(BLANK, 32*28);
  vram_adr(0x0);
  ppu_on_all();
}

/////

char in_rect(byte x, byte y, byte x0, byte y0, byte w, byte h) {
  return ((byte)(x-x0) < w && (byte)(y-y0) < h); // unsigned
}

void draw_bcd_word(byte col, byte row, word bcd) {
  byte j;
  static char buf[20];
  buf[27] = CHAR('0' + 43);
  buf[28] = CHAR('0' + 43);
  buf[29] = CHAR('0' + 43);
  for (j=3; j<0x80; j--) {
    buf[j] = CHAR('0'+(bcd&0xf));
    bcd >>= 4;
  }
  vrambuf_put(NTADR_A(col, row), buf, 35);
}

// GAME CODE

#define NSPRITES 8	// max number of sprites
#define NMISSILES 8	// max number of missiles
#define YOFFSCREEN 240	// offscreen y position (hidden)

// sprite indexes
#define PLYRMISSILE 7	// player missile
#define PLYRSPRITE 7	// player sprite
#define BOOMSPRITE 6	// explosion sprite

// nametable entries
#define NAME_SHIP	96
#define NAME_MISSILE	100
#define NAME_BOMB	104
#define NAME_EXPLODE	112
#define NAME_ENEMY	68

#define SSRC_FORM1 64	// name for formation source sprites
#define SDST_FORM1 128	// start of shifted formation table

typedef struct {
  byte shape;
} FormationEnemy;

// should be power of 2 length
typedef struct {
  byte findex;
  byte shape;
  word x;
  word y;
  byte dir;
  byte returning;
} AttackingEnemy;

typedef struct {
  byte xpos;
  byte ypos;
  signed char dx;
  signed char dy;
} Missile;

typedef struct {
  byte x;
  byte y;
  byte name;
  byte tag;
} Sprite;

#define ENEMIES_PER_ROW 8
#define ENEMY_ROWS 4
#define MAX_IN_FORMATION (ENEMIES_PER_ROW*ENEMY_ROWS)
#define MAX_ATTACKERS 6

FormationEnemy formation[MAX_IN_FORMATION];
AttackingEnemy attackers[MAX_ATTACKERS];
Missile missiles[NMISSILES];
Sprite vsprites[NSPRITES];

byte formation_offset_x;
signed char formation_direction;
byte current_row;
byte player_x;
byte player_y = 190;
byte player_exploding;
byte enemy_exploding;
byte enemies_left;
word player_score;

void copy_sprites() {
  byte i;
  byte oamid = 128; // so we don't clear stars...
  for (i=0; i<NSPRITES; i++) {
    Sprite* spr = &vsprites[i];
    if (spr->y != YOFFSCREEN) {
      byte y = spr->y;
      byte x = spr->x;
      byte chr = spr->name;
      byte attr = spr->tag;
      if (attr & 0x40) chr += 2; // horiz flip, swap tiles
      oamid = oam_spr(x, y, chr, attr, oamid);
      oamid = oam_spr(x+8, y, chr^2, attr, oamid);
    }
  }
  // copy all "shadow missiles" to video memory
  for (i=0; i<NMISSILES; i++) {
    Missile* mis = &missiles[i];
    if (mis->ypos != YOFFSCREEN) {
      oamid = oam_spr(mis->xpos, mis->ypos, NAME_MISSILE,
                      (i==7)?COLOR_MISSILE:COLOR_BOMB,
                      oamid);
    }
  }
  oam_hide_rest(oamid);
}

void add_score(word bcd) {
  player_score = bcd_add(player_score, bcd);
  draw_bcd_word(1, 1, player_score);
}

void clrobjs() {
  byte i;
  memset(vsprites, 0, sizeof(vsprites));
  for (i=0; i<NSPRITES; i++) {
    vsprites[i].y = YOFFSCREEN;
  }
  for (i=0; i<NMISSILES; i++) {
    missiles[i].ypos = YOFFSCREEN;
  }
}

void setup_formation() {
  byte i;
  memset(formation, 0, sizeof(formation));
  memset(attackers, 0, sizeof(attackers));
  for (i=0; i<MAX_IN_FORMATION; i++) {
    byte flagship = i < ENEMIES_PER_ROW;
    formation[i].shape = flagship ? SDST_FORM1 : SDST_FORM1;
  }
  enemies_left = MAX_IN_FORMATION;
  formation_offset_x = 8;
}

void draw_row(byte row) {
  static char buf[32];
  register byte i;
  register byte x = formation_offset_x / 8;
  byte xd = (formation_offset_x & 7) * 3;
  byte y = 3 + row * 2;
  memset(buf, BLANK, sizeof(buf));
  for (i=0; i<ENEMIES_PER_ROW; i++) {
    byte shape = formation[i + row*ENEMIES_PER_ROW].shape;
    if (shape) {
      shape += xd;
      buf[x] = shape;
      buf[x+1] = shape+1;
      buf[x+2] = shape+2;
    }
    x += 3;
  }
  vrambuf_put(NTADR_A(0, y), buf, sizeof(buf));
}

void draw_next_row() {
  draw_row(current_row);
  if (++current_row == ENEMY_ROWS) {
    current_row = 0;
    formation_offset_x += formation_direction;
    if (formation_offset_x == 63) {
      formation_direction = -1;
    }
    else if (formation_offset_x == 8) {
      formation_direction = 1;
    }
  }
}

#define FLIPX 0x40
#define FLIPY 0x80
#define FLIPXY 0xc0

const byte DIR_TO_CODE[32] = {
  0|FLIPXY, 1|FLIPXY, 2|FLIPXY, 3|FLIPXY, 4|FLIPXY, 5|FLIPXY, 6|FLIPXY, 6|FLIPXY,
  6|FLIPX, 6|FLIPX, 5|FLIPX, 4|FLIPX, 3|FLIPX, 2|FLIPX, 1|FLIPX, 0|FLIPX,
  0, 1, 2, 3, 4, 5, 6, 6,
  6|FLIPY, 6|FLIPY, 5|FLIPY, 4|FLIPY, 3|FLIPY, 2|FLIPY, 1|FLIPY, 0|FLIPY,
};

// sine table, pre-multiplied by 2
const int SINTBL2[32] = {
  0, 25*2, 49*2, 71*2, 90*2, 106*2, 117*2, 125*2,
  127*2, 125*2, 117*2, 106*2, 90*2, 71*2, 49*2, 25*2,
  0*2, -25*2, -49*2, -71*2, -90*2, -106*2, -117*2, -125*2,
  -127*2, -125*2, -117*2, -106*2, -90*2, -71*2, -49*2, -25*2,
};

#define ISIN(x) (SINTBL2[(x) & 31])
#define ICOS(x) ISIN(x+8)

#define FORMATION_X0 0
#define FORMATION_Y0 19
#define FORMATION_XSPACE 24
#define FORMATION_YSPACE 16

byte get_attacker_x(byte formation_index) {
  byte column = (formation_index % ENEMIES_PER_ROW);
  return FORMATION_XSPACE*column + FORMATION_X0 + formation_offset_x;
}

byte get_attacker_y(byte formation_index) {
  byte row = formation_index / ENEMIES_PER_ROW;
  return FORMATION_YSPACE*row + FORMATION_Y0;
}

void draw_attacker(byte i) {
  AttackingEnemy* a = &attackers[i];
  if (a->findex) {
    byte code = DIR_TO_CODE[a->dir & 31];
    vsprites[i].name = NAME_ENEMY + (code & 7)*4; // tile
    vsprites[i].tag = code & FLIPXY; // flip h/v
    vsprites[i].x = a->x >> 8;
    vsprites[i].y = a->y >> 8;
  } else {
    vsprites[i].y = YOFFSCREEN;
  }
}

void draw_attackers() {
  byte i;
  for (i=0; i<MAX_ATTACKERS; i++) {
    draw_attacker(i);
  }
}

void return_attacker(register AttackingEnemy* a) {
  byte fi = a->findex-1;
  byte destx = get_attacker_x(fi);
  byte desty = get_attacker_y(fi);
  byte ydist = desty - (a->y >> 8);
  // are we close to our formation slot?
  if (ydist == 0) {
    // convert back to formation enemy
    formation[fi].shape = a->shape;
    a->findex = 0;
  } else {
    a->dir = (ydist + 16) & 31;
    a->x = destx << 8;
    a->y += 128;
  }
}

void fly_attacker(register AttackingEnemy* a) {
  a->x += ISIN(a->dir);
  a->y += ICOS(a->dir);
  if ((a->y >> 8) == 0) {
    a->returning = 1;
  }
}

void move_attackers() {
  byte i;
  for (i=0; i<MAX_ATTACKERS; i++) {
    AttackingEnemy* a = &attackers[i];
    if (a->findex) {
      if (a->returning)
        return_attacker(a);
      else
        fly_attacker(a);
    }
  }
}

void think_attackers() {
  byte i;
  for (i=0; i<MAX_ATTACKERS; i++) {
    AttackingEnemy* a = &attackers[i];
    if (a->findex) {
      // rotate?
      byte x = a->x >> 8;
      byte y = a->y >> 8;
      // don't shoot missiles after player exploded
      if (y < 112 || player_exploding) {
        if (x < 128) {
          a->dir++;
        } else {
          a->dir--;
        }
      } else {
        // lower half of screen
        // shoot a missile?
        if (missiles[i].ypos == YOFFSCREEN) {
          missiles[i].ypos = y+16;
          missiles[i].xpos = x;
          missiles[i].dy = 2;
        }
      }
    }
  }
}

void formation_to_attacker(byte formation_index) {
  byte i;
  // out of bounds? return
  if (formation_index >= MAX_IN_FORMATION)
    return;
  // nobody in formation? return
  if (!formation[formation_index].shape)
    return;
  // find an empty attacker slot
  for (i=0; i<MAX_ATTACKERS; i++) {
    AttackingEnemy* a = &attackers[i];
    if (a->findex == 0) {
      a->x = get_attacker_x(formation_index) << 8;
      a->y = get_attacker_y(formation_index) << 8;
      a->shape = formation[formation_index].shape;
      a->findex = formation_index+1;
      a->dir = 0;
      a->returning = 0;
      formation[formation_index].shape = 0;
      break;
    }
  }
}

void draw_player() {
  vsprites[PLYRSPRITE].x = player_x;
  vsprites[PLYRSPRITE].y = player_y;
  vsprites[PLYRSPRITE].name = NAME_SHIP;
  vsprites[PLYRSPRITE].tag = COLOR_PLAYER;
}

void move_player() {
  byte joy = pad_poll(0);
  // move left/right?
  if ((joy & PAD_LEFT) && player_x > 16) player_x--;
  if ((joy & PAD_RIGHT) && player_x < 224) player_x++;
  // shoot missile?
  if ((joy & PAD_A) && missiles[PLYRMISSILE].ypos == YOFFSCREEN) {
    missiles[PLYRMISSILE].ypos = player_y-8; // must be multiple of missile speed
    missiles[PLYRMISSILE].xpos = player_x+4; // player X position
    missiles[PLYRMISSILE].dy = -4; // player missile speed
  }
  vsprites[PLYRMISSILE].x = player_x;
}

void move_missiles() {
  byte i;
  for (i=0; i<8; i++) { 
    if (missiles[i].ypos != YOFFSCREEN) {
      // hit the bottom or top?
      if ((byte)(missiles[i].ypos += missiles[i].dy) > YOFFSCREEN) {
        missiles[i].ypos = YOFFSCREEN;
      }
    }
  }
}

void blowup_at(byte x, byte y) {
  vsprites[BOOMSPRITE].tag = COLOR_EXPLOSION;
  vsprites[BOOMSPRITE].name = NAME_EXPLODE; // TODO
  vsprites[BOOMSPRITE].x = x;
  vsprites[BOOMSPRITE].y = y;
  enemy_exploding = 1;
}

void animate_enemy_explosion() {
  if (enemy_exploding) {
    // animate next frame
    if (enemy_exploding >= 8) {
      enemy_exploding = 0; // hide explosion after 4 frames
      vsprites[BOOMSPRITE].y = YOFFSCREEN;
    } else {
      vsprites[BOOMSPRITE].name = NAME_EXPLODE + (enemy_exploding += 4); // TODO
    }
  }
}

void animate_player_explosion() {
  byte z = player_exploding;
  if (z <= 3) {
    if (z == 3) {
      vsprites[PLYRSPRITE].y = YOFFSCREEN;
    } else {
      vsprites[PLYRSPRITE].name = NAME_EXPLODE + z*4;
    }
  }
}

void hide_player_missile() {
  missiles[PLYRMISSILE].ypos = YOFFSCREEN;
}

void does_player_shoot_formation() {
  byte mx = missiles[PLYRMISSILE].xpos + 4;
  byte my = missiles[PLYRMISSILE].ypos;
  signed char row = (my - FORMATION_Y0) / FORMATION_YSPACE;
  if (missiles[PLYRMISSILE].ypos == YOFFSCREEN)
    return;
  if (row >= 0 && row < ENEMY_ROWS) {
    // ok if unsigned (in fact, must be due to range)
    byte xoffset = mx - FORMATION_X0 - formation_offset_x;
    byte column = xoffset / FORMATION_XSPACE;
    byte localx = xoffset - column * FORMATION_XSPACE;
    if (column < ENEMIES_PER_ROW && localx < 16) {
      char index = column + row * ENEMIES_PER_ROW;
      if (formation[index].shape) {
        formation[index].shape = 0;
        enemies_left--;
        blowup_at(get_attacker_x(index), get_attacker_y(index));
        hide_player_missile();
        add_score(2);
      }
    }
  }
}

void does_player_shoot_attacker() {
  byte mx = missiles[PLYRMISSILE].xpos + 4;
  byte my = missiles[PLYRMISSILE].ypos;
  byte i;
  if (missiles[PLYRMISSILE].ypos == YOFFSCREEN)
    return;
  for (i=0; i<MAX_ATTACKERS; i++) {
    AttackingEnemy* a = &attackers[i];
    if (a->findex && in_rect(mx, my, a->x >> 8, a->y >> 8, 16, 16)) {
      blowup_at(a->x >> 8, a->y >> 8);
      a->findex = 0;
      enemies_left--;
      hide_player_missile();
      add_score(5);
      break;
    }
  }
}

void does_missile_hit_player() {
  byte i;
  if (player_exploding)
    return;
  for (i=0; i<MAX_ATTACKERS; i++) {
    if (missiles[i].ypos != YOFFSCREEN && 
        in_rect(missiles[i].xpos, missiles[i].ypos + 16, 
                player_x, player_y, 16, 16)) {
      player_exploding = 1;
      break;
    }
  }
}

void new_attack_wave() {
  byte i = rand();
  byte j;
  // find a random slot that has an enemy
  for (j=0; j<MAX_IN_FORMATION; j++) {
    i = (i+1) & (MAX_IN_FORMATION-1);
    // anyone there?
    if (formation[i].shape) {
      formation_to_attacker(i);
      formation_to_attacker(i+1);
      formation_to_attacker(i+ENEMIES_PER_ROW);
      formation_to_attacker(i+ENEMIES_PER_ROW+1);
      break;
    }
  }
}

void new_player_ship() {
  player_exploding = 0;
  player_x = 120;
  draw_player();
}

void set_sounds() {
  byte i;
  // these channels decay, so ok to always enable
  byte enable = ENABLE_PULSE0|ENABLE_PULSE1|ENABLE_NOISE;
  // missile fire sound
  if (missiles[PLYRMISSILE].ypos != YOFFSCREEN) {
    APU_PULSE_SUSTAIN(0, 255-missiles[PLYRMISSILE].ypos, DUTY_50, 6);
  } else {
    APU_PULSE_SET_VOLUME(0, DUTY_50, 0);
  }
  // enemy explosion sound
  if (player_exploding && player_exploding < 8) {
    APU_NOISE_DECAY(8 + player_exploding, 5, 15);
  } else if (enemy_exploding) {
    APU_NOISE_DECAY(8 + enemy_exploding, 2, 8);
  }
  // set diving sounds for spaceships
  for (i=0; i<2; i++) {
    register AttackingEnemy* a = i ? &attackers[4] : &attackers[0];
    if (a->findex && !a->returning) {
      byte y = a->y >> 8;
      APU_TRIANGLE_SUSTAIN(0x100 | y);
      enable |= ENABLE_TRIANGLE;
      break;
    }
  }
  APU_ENABLE(enable);
}

void init_stars() {
  byte oamid = 0; // 32 slots = 128 bytes
  byte i;
  for (i=0; i<32; i++) {
    oamid = oam_spr(rand(), i*8, 103+(i&3), 0|OAM_BEHIND, oamid);
  }
}
/*
void draw_stars_c() {
  byte i;
  for (i=0; i<32; i++) {
    ++OAMBUF[i].y;
  }
}
*/
void draw_stars() {
  asm("ldy #0");	// start with Y = 0
  asm("clc");		// clear carry for addition
  asm("@1: lda $200,y");// read from OAM buffer
  asm("adc #1");	// increment
  asm("sta $200,y");	// write to OAM buffer
  asm("iny");		// increment Y by 4
  asm("iny");
  asm("iny");
  asm("iny");
  asm("bpl @1");	// branch while < 128
}

void play_round() {
  register byte framecount;
  register byte t0;
  byte end_timer = 255;
  add_score(0);
  //putbytes(NTADR_A(0, 1), "PLAYER 1", 8);
  setup_formation();
  clrobjs();
  formation_direction = 1;
  framecount = 0;
  new_player_ship();
  while (end_timer) {
    if (player_exploding) {
      if ((framecount & 7) == 1) {
        animate_player_explosion();
        if (++player_exploding > 32 && enemies_left) {
          new_player_ship();
        }
      }
    } else {
      if (framecount == 0 || enemies_left < 8) {
        new_attack_wave();
      }
      move_player();
    }
    move_attackers();
    move_missiles();
    if (framecount & 1)
      does_player_shoot_formation();
    else
      does_player_shoot_attacker(); 
    draw_attackers();
    if ((framecount & 0xf) == 0) {
      think_attackers();
    } else {
      switch (framecount & 3) {
        case 1: animate_enemy_explosion(); // continue...
        case 2: does_missile_hit_player(); break;
        case 3: draw_stars(); break;
      }
      set_sounds();
      if (!enemies_left) end_timer--;
      draw_next_row();
    }
    vrambuf_flush();
    copy_sprites();
#ifdef DEBUG_FRAMERATE
    putchar(t0 & 31, 27, CHAR(' '));
    putchar(framecount & 31, 27, CHAR(' '));
#endif
    framecount++;
    t0 = nesclock();
#ifdef DEBUG_FRAMERATE
    putchar(t0 & 31, 27, CHAR('C'));
    putchar(framecount & 31, 27, CHAR('F'));
#endif
  }
}

// turn off aggressive inlining to save a few bytes
// functions after this point aren't called often
#pragma codesize(100)

void set_shifted_pattern(const byte* src, word dest, byte shift) {
  static byte buf[16*3];
  byte y;
  for (y=0; y<16; y++) {
    byte a = src[y];
    byte b = src[y+32];
    buf[y] = a>>shift;
    buf[y+16] = b>>shift | a<<(8-shift);
    buf[y+32] = b<<(8-shift);
  }
  vram_adr(dest);
  vram_write(buf, sizeof(buf));
}

void setup_graphics() {
  byte i;
  word src;
  word dest;
  // copy background
  vram_adr(0x0);
  vram_write(TILESET, sizeof(TILESET));
  // copy sprites
  vram_adr(0x1000);
  vram_write(TILESET, sizeof(TILESET));
  // write shifted versions of formation ships
  src = SSRC_FORM1*16;
  dest = SDST_FORM1*16;
  for (i=0; i<8; i++) {
    if (i==4) src += 16;
    set_shifted_pattern(&TILESET[src], dest, i);
    dest += 3*16;
  }
  // activate vram buffer
  vrambuf_clear();
  set_vram_update(updbuf);
}

void main() {  
  setup_graphics();
  apu_init();
  player_score = 0;
  while (1) {
    pal_all(PALETTE);
    oam_clear();
    oam_size(1); // 8x16 sprites
    clrscr();
    init_stars();
    play_round();
  }
}

