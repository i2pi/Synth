#ifndef _RGB_H_
#define _RGB_H_

extern int WIDTH, HEIGHT;
extern int CLIPX1, CLIPY1, CLIPX2, CLIPY2;
extern int SCROLL_X, SCROLL_Y;

unsigned char	r_ (unsigned long c);
unsigned char	g_ (unsigned long c);
unsigned char 	b_ (unsigned long c);
unsigned long	rgb (unsigned char r, unsigned char g, unsigned char b);
unsigned long	mix (unsigned long a, unsigned long b, float r);
void			pixel (int x, int y, unsigned long c);
unsigned long	get_pixel (int x, int y);
void			fade_pixel (int x, int y, unsigned long c, float m);
void			fade_screen (float f);
void			wu_line (int x1, int y1, int x2, int y2, unsigned long c);
void			line (int x, int y, int x1, int y1, unsigned long c);
void			box (int ax, int ay, int W, int H, unsigned long c);
void			fade_box (int ax, int ay, int W, int H, unsigned long c, float m);

#endif