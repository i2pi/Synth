#ifndef _FONT_H_
#define _FONT_H_

unsigned char font [38][14][16];
unsigned char font_width[38];

void	init_font (void);
void	print (char *s, int X, int Y);

#endif