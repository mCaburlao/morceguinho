#include <stdlib.h>
#include <string.h>
#include <nes.h>

#include "neslib.h"

#include "apu.h"
//#link "apu.c"

// BCD arithmetic support
#include "bcd.h"
//#link "bcd.c"

// VRAM update buffer
#include "vrambuf.h"
//#link "vrambuf.c"

#include "utils.h"

#define CHAR(x) ((x)-' ')

void draw_bcd_heart(byte col, byte row, byte life) { // pos 27
  static char buf[3]; 
  buf[0] = (life >= 4 ) ? CHAR('0' + 43) : CHAR('0' + 44);
  buf[1] = (life >= 3 ) ? CHAR('0' + 43) : CHAR('0' + 44);
  buf[2] = (life >= 2 ) ? CHAR('0' + 43) : CHAR('0' + 44);
 
  vrambuf_put(NTADR_A(col, row), buf, 3);
}

void draw_ufabc(byte col, byte row) { // pos 27
  static char buf[5]; 
  buf[0] =CHAR('U'); 
  buf[1] =CHAR('F'); 
  buf[2] =CHAR('A'); 
  buf[3] =CHAR('B'); 
  buf[4] =CHAR('C');
 
  vrambuf_put(NTADR_A(col, row), buf, 5);
}