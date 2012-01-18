#include <math.h>
#include <stdio.h>
#include "rgb.h"
#include "tclib.h"

int	CLIPX1, CLIPY1, CLIPX2, CLIPY2, SCROLL_X, SCROLL_Y;

unsigned char	r_ (unsigned long c)
{
	return ((unsigned char)((c >> 16) & 0xFF));
}

unsigned char	g_ (unsigned long c)
{
	return ((unsigned char)((c >> 8) & 0xFF));
}

unsigned char 	b_ (unsigned long c)
{
	return ((unsigned char) (c & 0xFF));
}

unsigned long rgb (unsigned char r, unsigned char g, unsigned char b)
{
	return (r << 16 | g << 8 | b);
}

unsigned long mix (unsigned long a, unsigned long b, float r)
{
	return ( rgb (
					(unsigned char) (r_(a)*r + r_(b)*(1.0f-r)),
					(unsigned char) (g_(a)*r + g_(b)*(1.0f-r)),
					(unsigned char) (b_(a)*r + b_(b)*(1.0f-r))
				 ));
}

unsigned long	get_pixel (int x, int y)
{
	x += SCROLL_X; y += SCROLL_Y;
	if ((y > CLIPY1) && (y < CLIPY2) && (x > CLIPX1) && (x < CLIPX2))
		return (tcono[y*WIDTH+x]);
	return (0);
}

void pixel (int x, int y, unsigned long c)
{	
	x += SCROLL_X; y += SCROLL_Y;
	if ((y > CLIPY1) && (y < CLIPY2) && (x > CLIPX1) && (x < CLIPX2))
	{
		tcono[y*WIDTH+x] = c;	
	} 
}

void fade_pixel (int x, int y, unsigned long c, float m)
{	
	pixel (x,y, mix (get_pixel(x,y), c, m));
}

void	fade_screen (float f)
{
	int	x, y;
	unsigned long	r,g,b;

	for (x=0; x<WIDTH; x++)
	for (y=0; y<HEIGHT; y++)
	{
		r = r_(tcono[y*WIDTH+x]); 
		g = g_(tcono[y*WIDTH+x]); 
		b = b_(tcono[y*WIDTH+x]); 
		tcono[y*WIDTH+x] = rgb ((unsigned char)(r*f),
								(unsigned char)(g*f),
								(unsigned char)(b*f));
	}
}

/*
** http://freespace.virgin.net/hugo.elias/graphics/x_wuline.htm
*/

/*
** Fixed Point functions required by WuLines:
*/
/*
    function trunc(x)
	return integer part of x
    end of function

    function frac(x)
	return fractional part of x
    end of function
*/

float	trunc(float x)
{
	int	t;
	t = (int) x;
	return ((float)t);
}

float	frac(float x)
{
	return (x - trunc(x));
}

float	invfrac (float x)
{
	return (1.0f - frac(x));
}

void	swap (int a, int b)
{
	int	c;
	c = a;
	a = b;
	b = c;
}

void	DrawPixel (int x, int y, unsigned long c, float m)
{	
/*
	if ((m > 1.0) || (m < 0.0))
	{
		printf ("** %6.4f\n", m);
	}
*/
	if ((y > CLIPY1) && (y < CLIPY2) && (x > CLIPX1) && (x < CLIPX2))
	tcono[y*WIDTH+x] = mix (tcono[y*WIDTH+x], c, 1.0f - m);
}

void	dot (int x, int y)
{
	int	i,j;

	for (j=y-3; j<y+3; j++)
	for (i=x-3; i<x+3; i++)
	{
		pixel (i,j, 0x00ff00);
	}
}


void	line (int x, int y, int x1, int y1, unsigned long c)
{
	float	m;
	int		X,Y;

	if (abs(x-x1) > abs(y-y1))	
	{
		m = (y1 - y) / (float)(x1 - x);

		if (x < x1)
		{
			for (X=x; X<x1; X++)
			{
				Y = (int) (y + m * (X - x));
				fade_pixel (X, Y, c, 0.3f);
			}
		} else
		{
			for (X=x1; X<x; X++)
			{
				Y = (int) (y + m * (X - x));
				fade_pixel (X, Y, c, 0.3f);
			}
		}
	} else
	{
		m = (x1 - x) / (float)(y1 - y);

		if (y < y1)
		{
			for (Y=y; Y<y1; Y++)
			{
				X = (int) (x + m * (Y - y));
				fade_pixel (X, Y, c, 0.3f);
			}
		} else
		{
			for (Y=y1; Y<y; Y++)
			{
				X = (int) (x + m * (Y - y));
				fade_pixel (X, Y, c, 0.3f);
			}
		}
	}
}

void	box (int ax, int ay, int W, int H, unsigned long c)
{	
	int	x, y;

	for (x=ax-W; x<ax+W; x++)
	{
		y = ay - H - 1;
		fade_pixel (x, y, c, 0.5f);
		y = ay + H;
		fade_pixel (x, y, c, 0.5f);
	}

	for (y=ay-H; y<ay+H; y++)
	{
		x = ax - W - 1;
		fade_pixel (x, y, c, 0.5f);
		x = ax + W;
		fade_pixel (x, y, c, 0.5f);
	}

	for (x=ax-W; x<ax+W; x++)
	for (y=ay-H; y<ay+H; y++)
	{
		pixel (x, y, c);
	}
}

void	fade_box (int ax, int ay, int W, int H, unsigned long c, float m)
{	
	int	x, y;

	for (x=ax-W; x<ax+W; x++)
	{
		y = ay - H - 1;
		fade_pixel (x, y, c, 0.5f*m);
		y = ay + H;
		fade_pixel (x, y, c, 0.5f*m);
	}

	for (y=ay-H; y<ay+H; y++)
	{
		x = ax - W - 1;
		fade_pixel (x, y, c, 0.5f*m);
		x = ax + W;
		fade_pixel (x, y, c, 0.5f*m);
	}

	for (x=ax-W; x<ax+W; x++)
	for (y=ay-H; y<ay+H; y++)
	{
		fade_pixel (x, y, c, 0.5f*m);
	}
}



void wu_line (int x1, int y1, int x2, int y2, unsigned long c)
{
 	float			grad, xd, yd;
	float			xgap, ygap, xend, yend, xf, yf;
	float			brightness1, brightness2;
	int				x, y, ix1, ix2, iy1, iy2;
	
	
	
	fade_box (x1, y1, 1, 1, c, 0.7f);
	fade_box (x2, y2, 1, 1, c, 0.7f);

//	line (x1, y1, x2, y2, c); return;

	x1 += SCROLL_X;
	y1 += SCROLL_Y;
	x2 += SCROLL_X;
	y2 += SCROLL_Y;
	
	
	xd = (float)(x2-x1);
	yd = (float)(y2-y1);

	if (x1 < CLIPX1)
	{
		y1 = (int)((yd/xd) * (CLIPX1-x1) + y1);
		x1 = CLIPX1;
	} 
	if (x1 > CLIPX2)
	{
		y1 = (int)((yd/xd) * (CLIPX2-x1) + y1);
		x1 = CLIPX2;
	}
	if (x2 < CLIPX1)
	{
		y2 = (int)((yd/xd) * (CLIPX1-x2) + y2);
		x2 = CLIPX1;
	}
	if (x2 > CLIPX2)
	{
		y2 = (int)((yd/xd) * (CLIPX2-x2) + y2);
		x2 = CLIPX2;
	}


	if (y1 < CLIPY1)
	{
		x1 = (int)((xd/yd) * (CLIPY1-y1) + x1);
		y1 = CLIPY1;
	}
	if (y1 > CLIPY2)
	{
		x1 = (int)((xd/yd) * (CLIPY2-y1) + x1);
		y1 = CLIPY2;
	}
	if (y2 < CLIPY1)
	{
		x2 = (int)((xd/yd) * (CLIPY1-y2) + x2);
		y2 = CLIPY1;
	}
	if (y2 > CLIPY2)
	{
		x2 = (int)((xd/yd) * (CLIPY2-y2) + x2);
		y2 = CLIPY2;
	}

	xd = (float)(x2-x1);
	yd = (float)(y2-y1);
	
	if (fabs(xd) > fabs(yd))
	{
		if (x1 > x2)
		{
		    swap (x1, x2);
		    swap (y1, y2);
	        xd = (float)(x2-x1);
	        yd = (float)(y2-y1);
		}

	 	grad = yd/(float)xd;		

		xend = trunc(x1+0.5f); //                      find nearest integer X-coordinate
		yend = y1 + grad*(xend-x1); //               and corresponding Y value
		
		xgap = invfrac(x1+0.5f); //                    distance i
		
		ix1  = (int)(xend);   //                      calc screen coordinates
		iy1  = (int)(yend);
	
		brightness1 = invfrac(yend) * xgap; //      calc the intensity of the other 
		brightness2 =    frac(yend) * xgap; //      end point pixel pair.
		
		DrawPixel (ix1,iy1, c, brightness1);	//			 draw the pair of pixels
		DrawPixel (ix1,iy1+1, c, brightness2);

		yf = yend+grad; //                           calc first Y-intersection for
                          //                               main loop

		//End Point 2
		//-----------

		xend = trunc(x2+0.5f);     //                 find nearest integer X-coordinate
		yend = y2 + grad*(xend-x2);  //             and corresponding Y value
		
		xgap = invfrac(x2-0.5f); //                   distance i
		
		ix2  = (int)(xend);//                         calc screen coordinates
		iy2  = (int)(yend);
	
		brightness1 = invfrac(yend) * xgap;//       calc the intensity of the first 
		brightness2 =    frac(yend) * xgap;//       end point pixel pair.
		
		DrawPixel(ix2,iy2, c, brightness1); //			draw the pair of pixels
		DrawPixel(ix2,iy2+1, c, brightness2);

		//MAIN LOOP
		if (ix1+1 < ix2-1)
		{
			for (x=ix1+1; x<ix2-1; x++)
			{
				brightness1 = invfrac(yf);
				brightness2 =    frac(yf);

				DrawPixel(x,(int)(yf), c, brightness1);
 				DrawPixel(x,(int)(yf)+1, c, brightness2);

				yf = yf + grad;
			}	
		} else
		{	
			yf = (float) y2; 
			for (x=ix2-1; x<ix1+1; x++)
			{
				brightness1 = invfrac(yf);
				brightness2 =    frac(yf);

				DrawPixel(x,(int)(yf), c, brightness1);
 				DrawPixel(x,(int)(yf)+1, c, brightness2);

				yf = yf + grad;
			}
		}
	}  else
	{
		if (y1 > y2)
		{
		    swap (y1, y2);
		    swap (x1, x2);
	        yd = (float)(y2-y1);
	        xd = (float)(x2-x1);
		}
		
		grad = xd/(float)yd;
		
		yend = trunc(y1+0.5f); //                      find nearest integer y-coordinate
		xend = x1 + grad*(yend-y1); //               and corresponding x value
		
		ygap = invfrac(y1+0.5f); //                    distance i
		
		iy1  = (int)(yend);   //                      calc screen coordinates
		ix1  = (int)(xend);
	
		brightness1 = invfrac(xend) * ygap; //      calc the intensitx of the other 
		brightness2 =    frac(xend) * ygap; //      end point Pixel pair.
		
		DrawPixel (ix1,iy1, c, brightness1);	//			 draw the pair of Pixels
		DrawPixel (ix1,iy1+1, c, brightness2);
		
		xf = xend+grad; //                           calc first x-intersection for
                          //                               main loop
		//End Point 2
		//-----------

		yend = trunc(y2+0.5f);     //                 find nearest integer y-coordinate
		xend = x2 + grad*(yend-y2);  //             and corresponding x value
		
		ygap = invfrac(y2-0.5f); //                   distance i
		
		iy2  = (int)(yend);//                         calc screen coordinates
		ix2  = (int)(xend);
	
		brightness1 = invfrac(xend) * ygap;//       calc the intensitx of the first 
		brightness2 =    frac(xend) * ygap;//       end point Pixel pair.
		
		DrawPixel(ix2,iy2, c, brightness1); //			draw the pair of Pixels
		DrawPixel(ix2,iy2+1, c, brightness2);

		//MAIN LOOP
	
		if (iy1+1 < iy2-1)
		{
			for (y=iy1+1; y<iy2-1; y++)
			{
				brightness1 = invfrac(xf);
				brightness2 =    frac(xf);

				DrawPixel((int)(xf),y, c, brightness1);
 				DrawPixel((int)(xf+1),y, c, brightness2);

				xf = xf + grad;
			}
		} else
		{
			xf = (float)x2;
			for (y=iy2-1; y<iy1+1; y++)
			{
				brightness1 = invfrac(xf);
				brightness2 =    frac(xf);

				DrawPixel((int)(xf),y, c, brightness1);
 				DrawPixel((int)(xf+1),y, c, brightness2);

				xf = xf + grad;
			}
		}
	}
}


