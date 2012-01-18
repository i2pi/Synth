#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <math.h>

#include "portaudio.h"
#include "sound.h"
#include "Font.h"
#include "mathmat.h"
#include "rgb.h"
#include "tick.h"
#include "audio_units.h"
#include "gui.h"
#include "tclib.h"
#include "control_unit.h"
#include "file.h"	


int			menu_x, menu_y;
audio_unitT	*current, *focus;
control_unitT *c_current, *c_focus;
int			INPUT_LINK=0, OUTPUT_LINK=0, DELETE_LINK=0, CONT_AUDIO_LINK=0,
			CONT_AUDIO_DELETE_LINK=0;
int			PARAM_MENU=0, UNIT_MENU=0, CUNIT_MENU=0, CPARAM_MENU=0, SCROLL=0;
int			menu_item;
int			counter_i, counter_j, counter_div, parameter_i;
int			mousex, mousey;
int			l_click, r_click;

void	layout_unit (audio_unitT *au)
{	
	int		W, H;

	W = (WIDTH-200)/2;
	H = (HEIGHT-100)/2;
	au->x = (int) (Sin(rand()) * W + W + 50);
	au->y = (int) (Sin(rand()) * H + H + 50);
}

void	layout_units (void)
{
	int		i;

	current = focus = NULL;

	for (i=0; i<MAX_UNITS; i++)
	if (audio_unit[i])
	{
		layout_unit (audio_unit[i]);
	}
}

void	draw_cunits (void)
{
	int				i, j, x, y, W, H;
	control_unitT	*cu;
	unsigned long	c;

	W = 12;
	H = 12;

	x = mousex;
	y = mousey;
	
	c_focus = NULL;

	for (i=0; i<MAX_CONTROLS; i++)
	if (control_unit[i])
	{	
		c = DK_BLUE;	
		cu = control_unit[i];
		if (((abs (x-cu->x) < W) && (abs(y-cu->y) < H)) || (cu == c_current))
		{
			if (!CONT_AUDIO_LINK && !CONT_AUDIO_DELETE_LINK) current = NULL;
			focus = NULL;
			if (!INPUT_LINK && !OUTPUT_LINK && !DELETE_LINK && !PARAM_MENU && 
				!UNIT_MENU && !CUNIT_MENU)
			{
				c_current = cu;		
			} else 
			{		
				if (INPUT_LINK)
					wu_line (cu->x, cu->y-H, mousex, mousey, BR_BLUE);
				if (OUTPUT_LINK)
					wu_line (cu->x, cu->y+H, mousex, mousey, BR_BLUE);
				if (CONT_AUDIO_LINK)
					wu_line (cu->x, cu->y+H, mousex, mousey, 0xff80ff);
			}
			if (((abs (x-cu->x) < W) && (abs(y-cu->y) < H)) && !PARAM_MENU && 
				!UNIT_MENU && !CUNIT_MENU) 
			{
				c_focus = cu;
			}				
		}		
		
		if (c_focus == cu) c = FC_BLUE; else
		if (c_current == cu) c = BR_BLUE;

		for (j=0; j<cu->inputs; j++)
		{
			wu_line (cu->x, cu->y-H, cu->input[j]->x, cu->input[j]->y+H, BR_BLUE);
		}
		fade_box (cu->x, cu->y, W, H, c, 0.3f);
		print (cu->short_name, cu->x-10, cu->y-9);
	}
	
}


void	draw_unit (audio_unitT *au, int h, int i)
{	
	int				x,y, W, H;
	unsigned long	c;
	
	W = 20;
	H = 10;
	
	fade_box (au->x, au->y, 25, 8, mix(0xff00ff, 0xffffff, 0.5f*(au->status+1.0f)), 0.7f);

	x = mousex;
	y = mousey;
	c = DK_RED;	

	if (((abs (x-au->x) < W) && (abs(y-au->y) < H)) || (au == current))
	{
		if (c_current || c_focus)
		{
			if (OUTPUT_LINK || CONT_AUDIO_LINK)
			{
				PARAM_MENU = 1;
				CONT_AUDIO_LINK = 1;
			} else
			if (DELETE_LINK)
			{
				CONT_AUDIO_DELETE_LINK = 1;
			}
			INPUT_LINK = OUTPUT_LINK = DELETE_LINK = 0;
		}
		if (!CONT_AUDIO_LINK && !CONT_AUDIO_DELETE_LINK)
		{
			c_current = NULL;	
		}
		c_focus = NULL;
		if  (CONT_AUDIO_LINK || CONT_AUDIO_DELETE_LINK)
		{
			current = au;
		} 
		if (!INPUT_LINK && !OUTPUT_LINK && !DELETE_LINK && !PARAM_MENU && 
			!UNIT_MENU && !CUNIT_MENU)
		{
			current = au;		
		} else 
		{		
			if (INPUT_LINK)
					wu_line (au->x, au->y-H, mousex, mousey, BR_RED);
			if (OUTPUT_LINK)
					wu_line (au->x, au->y+H, mousex, mousey, BR_RED);
		}
		if (((abs (x-au->x) < W) && (abs(y-au->y) < H)) && !PARAM_MENU && 
			!UNIT_MENU && !CUNIT_MENU) 
		{
			focus = au;
		}
	}

	if (focus == au) c = FC_RED; else
	if (current == au) c = BR_RED; 

	for (x=0; x<au->inputs; x++)
	{
		wu_line (au->x, au->y-H, au->input[x]->x, au->input[x]->y+H, BR_RED);
	}

	if ((!PARAM_MENU) || (PARAM_MENU && (current != au)))
	{
		for (x=0; x<au->parameters; x++)
		if (au->parameter[x].control)
		{
			wu_line (au->x+W+5, au->y, 
					au->parameter[x].control->x, au->parameter[x].control->y+12, 0xff80ff);
		}
	}
	fade_box (au->x, au->y, 20, 10, c, 0.2f);

	print (au->short_name, au->x -W+4, au->y-9);

}

void	draw_units (void)
{
	int		i;

	for (i=0; i<MAX_UNITS; i++)
	if (audio_unit[i])
	{
		draw_unit (audio_unit[i], 1, i);
	}
}

void	unit_menu (void)
{
	int		i, x, y, mx, my, W, H;

	W = 60;
	H = 8*audio_unit_inss;
	x = menu_x;
	y = menu_y;

	fade_box (x, y, W, H, 0xf08080, 0.3f);
	
	menu_item = -1;
	mx = mousex;	
	my = mousey;
	for (i=0; i<audio_unit_inss; i++)
	{
		if ((mx > x-W) && (mx < x+W) && (my > y - H + 5 + i*15) 
			&& (my < y-H+5+(i+1)*15))
		{
			menu_item = i;
			fade_box (x, y-H+10+i*15, W-2, 8, 0xf09090, 0.3f);
		}
	
		print (audio_unit_ins[i].name, x - W + 5, y - H + i * 15);	
	}
}


void	parameter_menu (void)
{
	int		i, x, y, W, H, mx, my, Y;

	if (!current) return;
	if (!current->parameters) return;

	W = 60;
	H = 8*current->parameters;
	x = current->x + W;
	y = current->y + H;
	mx = mousex;
	my = mousey;
	
	fade_box (x, y, W, H, 0xf08080, 0.3f);

	menu_item = -1;
	
	for (i=0; i<current->parameters; i++)
	{
		float	min, v, max, s;
		char	str[128];

		min = 0.0f;
		max = 1.0f;
		v = 0.5f;
		
		if (current->parameter[i].p_type == TRIGGER)
		{
			min = 0.0f;
			v = (float) (*current->parameter[i].trigger_value);
			sprintf (str, "%d", (int)v);
			max = 1.0f;
		} else
		if (current->parameter[i].p_type == FLOAT)
		{
			min = current->parameter[i].float_min;
			v = *current->parameter[i].float_value;
			max = current->parameter[i].float_max;
			sprintf (str, "%+6.4f", v);
		} else
		if (current->parameter[i].p_type == INT)
		{
			min = (float) current->parameter[i].int_min;
			v =   (float) (*current->parameter[i].int_value);
			max = (float) current->parameter[i].int_max;
			sprintf (str, "%d", (int)v);
		}
		s = (v - min) / max;
		box (x + W - 6, y - H + 10 + i * 15, 4, 4, 
					mix (0xff0000, 0xf08080, s));
		if (current->parameter[i].control)
		{
			wu_line (x+W, y - H + 10 + i * 15, 
				current->parameter[i].control->x, 
				current->parameter[i].control->y+12, 0xff80ff);
		}
		if (CONT_AUDIO_LINK && (mx > x-W) && (mx < x+W) && (my > y - H + 5 + i*15) 
			&& (my < y-H+5+(i+1)*15))
		{
			fade_box (x, y-H+9+i*15, W, 8, 0xf09090, 0.3f);
			parameter_i = i;
		} else
		if ((mx > x-W) && (mx < x+W-10) && (my > y - H + 5 + i*15) 
			&& (my < y-H+5+(i+1)*15))
		{
			fade_box (x, y-H+9+i*15, W, 8, 0xf09090, 0.3f);
			for (Y = y-H+3+i*15; Y<y-H+3+(i+1)*15; Y++)
			{
				pixel (mx, Y, 0xff0000);	
			}
			v = ((mx - (x-W-10)) / (float)(W*2.1f)) * (max - min) + min;
			parameter_set (&current->parameter[i], v);
		}
		print (current->parameter[i].name, x - W + 10, y - H + i * 15);	
	}
}

void	cparameter_menu (void)
{
	int		i, x, y, W, H, mx, my, Y;

	if (!c_current) return;
	if (!c_current->parameters) return;

	W = 60;
	H = 8*c_current->parameters;
	x = c_current->x + W;
	y = c_current->y + H;
	mx = mousex;
	my = mousey;
	
	fade_box (x, y, W, H, BR_BLUE, 0.3f);

	menu_item = -1;
	
	for (i=0; i<c_current->parameters; i++)
	{
		float	min, v, max, s;
		char	str[128];

		min = 0.0f;
		max = 1.0f;
		v = 0.5f;
		
		if (c_current->parameter[i].p_type == TRIGGER)
		{
			min = 0.0f;
			v = (float) (*c_current->parameter[i].trigger_value);
			sprintf (str, "%d", (int)v);
			max = 1.0f;
		} else
		if (c_current->parameter[i].p_type == FLOAT)
		{
			min = c_current->parameter[i].float_min;
			v = *c_current->parameter[i].float_value;
			max = c_current->parameter[i].float_max;
			sprintf (str, "%+6.4f", v);
		} else
		if (c_current->parameter[i].p_type == INT)
		{
			min = (float) c_current->parameter[i].int_min;
			v =   (float) (*c_current->parameter[i].int_value);
			max = (float) c_current->parameter[i].int_max;
			sprintf (str, "%d", (int)v);
		}
		s = (v - min) / max;
		box (x + W - 6, y - H + 10 + i * 15, 4, 4, 
					mix (0x0000ff, 0x8080f0, s));
		if (c_current->parameter[i].control)
		{
			wu_line (x+W, y - H + 10 + i * 15, 
				c_current->parameter[i].control->x, 
				c_current->parameter[i].control->y+12, 0xff80ff);
		}
		if (CONT_AUDIO_LINK && (mx > x-W) && (mx < x+W) && (my > y - H + 5 + i*15) 
			&& (my < y-H+5+(i+1)*15))
		{
			fade_box (x, y-H+9+i*15, W, 8, 0x9090f0, 0.3f);
			parameter_i = i;
		} else
		if ((mx > x-W) && (mx < x+W-10) && (my > y - H + 5 + i*15) 
			&& (my < y-H+5+(i+1)*15))
		{
			fade_box (x, y-H+9+i*15, W, 8, 0x9090f0, 0.3f);
			for (Y = y-H+3+i*15; Y<y-H+3+(i+1)*15; Y++)
			{
				pixel (mx, Y, 0x0000ff);	
			}
			v = ((mx - (x-W-10)) / (float)(W*2.1f)) * (max - min) + min;
			parameter_set (&c_current->parameter[i], v);
		}
		print (c_current->parameter[i].name, x - W + 10, y - H + i * 15);	
	}
}



void	trigger (audio_unitT *au)
{
	int	i;

	for (i=0; i<au->parameters; i++)
	{
		if (au->parameter[i].p_type == TRIGGER)
		{
			*au->parameter[i].trigger_value = 1;
			printf ("Triggering %s %s\n", au->long_name, au->parameter[i].name);
		}
	}
}



void	draw_counters (void)
{
	int				i, j, x, y, mx, my;
	unsigned long	c;
	char			str[5];

	counter_i = -1;	
	counter_j = -1;

	mx = mousex;
	my = mousey;

	for (i=0; i<tick_data.counters; i++)
	{
		counterT	*cnt;
		cnt = &tick_data.counter[i];
		sprintf (str, "%d", cnt->beat_div);
		print (str, 1 - SCROLL_X, 31+i*10 - SCROLL_Y);
		x = 18 - SCROLL_X;	
		y = 40 + i*10 - SCROLL_Y;
		box (x + (cnt->loop_value*7)/2 + 5, y, cnt->loop_value*7/2 + 8, 4, 0xf0fff0);	
		for (j=0; j<cnt->loop_value; j++)
		{
			if (cnt->hit[j])
				c = 0xa0f0a0;
			else
				c = 0xa0a0a0;
			
			x = 20+j*7 - SCROLL_X;	
			y = 40 + i*10 - SCROLL_Y;
			box (x, y, 3, 3, c);
			if (abs(y-my) < 4)
			{
				counter_i = i;
				counter_div = 0;
				if (abs(x-mx) < 4)
				{
					counter_j = j;	
					fade_box (x, y, 2, 2, 0xa0ffa0, 0.5);
				}
				if (mx < 18) counter_div = 1;
			}
		}
		wu_line (24+cnt->loop_value*7- SCROLL_X, 40 + i*10-SCROLL_Y,
					 cnt->au->x-25, cnt->au->y, 0x80ff80);
		x = 20+cnt->value*7 - SCROLL_X;	
		y = 40 + i*10 - SCROLL_Y;
		box (x, y, 2, 2, 0xffffff);
			
	}
}

void	give_au_tick (audio_unitT *au)
{
	int	p;

	for (p=0; p<au->parameters; p++)
	if (au->parameter[p].p_type == TRIGGER)
	{	
		tick_add_counter (0, 16, au, p); 
	}
}

void	cunit_menu (void)
{
	int		i, x, y, mx, my, W, H;

	W = 60;
	H = 8*control_unit_inss;
	x = menu_x;
	y = menu_y;

	fade_box (x, y, W, H, 0x8080f0, 0.3f);
	
	menu_item = -1;
	mx = mousex;	
	my = mousey;
	for (i=0; i<control_unit_inss; i++)
	{
		if ((mx > x-W) && (mx < x+W) && (my > y - H + 5 + i*15) 
			&& (my < y-H+5+(i+1)*15))
		{
			menu_item = i;
			fade_box (x, y-H+10+i*15, W-2, 8, 0x9090f0, 0.3f);
		}
	
		print (control_unit_ins[i].name, x - W + 5, y - H + i * 15);	
	}
}

void	delete_au (audio_unitT *au)
{
	int		i;

	if (au == &audio_unit_root.dac) return;

	i=0; 
	while ((i<tick_data.counters) && (tick_data.counter[i].au != au)) i++;
	if (tick_data.counter[i].au == au)
	{
		tick_delete_counter (i);
	}
	for (i=0; i<MAX_UNITS; i++)	
	if (audio_unit[i])
	{
		if (audio_unit[i] == au)
		{
			audio_unit[i] = NULL;
		} else
		{
			au_delete_link (au, audio_unit[i]);
		}
	}
	current = NULL;
	focus = NULL;
}


void	delete_cu (control_unitT *cu)
{
	int	i, j;

	for (i=0; i<MAX_CONTROLS; i++)	
	if (control_unit[i])
	{
		if (control_unit[i] == cu)
		{
			control_unit[i] = NULL;
		} else
		{
			cu_delete_link (cu, control_unit[i]);
		}
	}
	for (i=0; i<MAX_UNITS; i++)
	if (audio_unit[i])
	{
		for (j=0; j<audio_unit[i]->parameters; j++)
		if (audio_unit[i]->parameter[j].control == cu)
		{
			delete_cu_au_link (audio_unit[i], cu);			
		}
	}
	c_current = NULL;
	c_focus = NULL;
}

void clear_options (void)
{	
	INPUT_LINK = 0;
	OUTPUT_LINK = 0;
	DELETE_LINK = 0;	
	CONT_AUDIO_LINK = 0;
	PARAM_MENU = 0;
	UNIT_MENU = 0;
	CPARAM_MENU = 0;
	CUNIT_MENU = 0;	
	SCROLL = 0;
}

void deselect (void)
{
	focus = NULL;
	current = NULL;
	c_focus = NULL;
	c_current = NULL;
	clear_options ();
}

void	draw_scroll (int *sx, int *sy)
{
	int	b;

	b = mouseb_tc();

	
	fade_box (WIDTH/2, HEIGHT-8, WIDTH/2 - 3, 6, 0xf0f080, 0.9f);
	fade_box (WIDTH/2 - *sx / 15,  HEIGHT-8, 5, 6, 0xf0f080, 0.1f);
	
	fade_box (WIDTH-8, HEIGHT/2 + 8, 6,  HEIGHT/2 - 25, 0xf0f080, 0.9f);
	fade_box (WIDTH-8, HEIGHT/2 + 8 - *sy/15, 6,  5, 0xf0f080, 0.1f);

	if (l_click)
	{
		if ((mousex > WIDTH - 15) && (mousey > 40) && (mousey < HEIGHT-30))	SCROLL=1;		
		if ((mousey > HEIGHT-15) && (mousex > 10) && (mousex < WIDTH-10)) SCROLL=1;
	}
	
	if (SCROLL && (b==1))
	{
		if ((mousex > WIDTH - 15) && (mousey > 40) && (mousey < HEIGHT-30))	
		{
			*sy = (-mousey + (HEIGHT/2 + 8)) * 15;		
			deselect ();
			SCROLL=1;
		} else
		if ((mousey > HEIGHT-15) && (mousex > 10) && (mousex < WIDTH-10))
		{	
			*sx = (-mousex + (WIDTH/2)) * 15;
			deselect ();		
			SCROLL=1;
		}
	} else
	{
		SCROLL = 0;
	}
}


void	draw_gui (void)
{
	char			str[512];
	static int		b = 0, ob = 0;
	static float	cpu_load = 0.0;
	int				key, sx, sy;
	
	sx = SCROLL_X;
	sy = SCROLL_Y;
	SCROLL_X = 0;
	SCROLL_Y = 0;
	CLIPX1 = 0; CLIPX2 = WIDTH;	CLIPY1 = 0; CLIPY2 = HEIGHT;
	
	mousex = mousex_tc();
	mousey = mousey_tc();
		
	memset (tcono, 0, sizeof(unsigned long)*WIDTH*30);
	memset (&tcono[WIDTH*30], 0x80, sizeof(unsigned long)*WIDTH*(HEIGHT-30));
	sprintf (str, "%06.2f", tick_data.seconds);
	print(str, 30, 10);
	sprintf (str, "%08.2f", tick_data.samples);
	print(str, 100, 10);

	cpu_load = (float) (cpu_load*0.8f + Pa_GetCPULoad(stream)*0.2f);
	if (cpu_load > 0.69f) DONE = 1;
	sprintf (str, "cpu load %3.2f", cpu_load * 100.0) ;
	print(str, 450, 10);
	print("i2pi", 250, 10);

	draw_scroll (&sx, &sy);

	SCROLL_X = sx;
	SCROLL_Y = sy;
	CLIPX1 = 0; CLIPX2 = WIDTH-15;
	CLIPY1 = 30; CLIPY2 = HEIGHT-15;
	mousex = mousex_tc() - SCROLL_X;
	mousey = mousey_tc() - SCROLL_Y;
	
	focus = NULL;
	c_focus = NULL;

	draw_units ();
	draw_cunits ();

	ob = b;
	b = mouseb_tc ();
	if ((ob == 0) && (b == 1)) l_click = 1; else l_click = 0;
	if ((ob == 0) && (b == 2)) r_click = 1; else r_click = 0;

/*	printf ("PM%d UM%d CUM%d CPM%d AL%d IL%d OL%d DL%d CL%d CC%p CF%p C%p F%p\n", 
		PARAM_MENU, UNIT_MENU, CUNIT_MENU, CPARAM_MENU,
		CONT_AUDIO_LINK, INPUT_LINK, OUTPUT_LINK, DELETE_LINK,
		CONT_AUDIO_DELETE_LINK,
		c_current, c_focus, current, focus);
*/
	if (kbhit_tc())
	{
		key = toupper (getch_tc());	
	} else
	{
		key = 0;
	}

	switch (key)
	{
		case 'S': save(); break;
		case 'H':	
			SCROLL_X=-audio_unit_root.dac.x + WIDTH/2;		
			SCROLL_Y=-audio_unit_root.dac.y + HEIGHT/2;
			break;
		case 27: deselect (); break;
		case 'Q': DONE = 1; break;
		case  8:
		{
			if (current) delete_au (current); 
			if (c_current) delete_cu (c_current);
			deselect ();
		}
		case '=': 
		case '+': init_tick ((float)SAMPLE_RATE, (float)(tick_data.bpm += 5.0f)); break;
		case '-': 
		case '_': init_tick ((float)SAMPLE_RATE, (float)(tick_data.bpm -= 5.0f)); break;
		default: if (key) printf ("[%03d] %c\n", key, key);
	}
	mousex = mousex_tc() - SCROLL_X;
	mousey = mousey_tc() - SCROLL_Y;
	
	if (r_click)
	{
		if (focus)
		{
			PARAM_MENU = PARAM_MENU ? 0 : 1;
		} else
		if (c_focus)
		{
			CPARAM_MENU = CPARAM_MENU ? 0 : 1;
		} else
		if (counter_i >= 0)
		{
			int bd;

			if (counter_div)
			{	
				bd = tick_data.counter[counter_i].beat_div;	
				bd++;
				tick_data.counter[counter_i].beat_div = bd;
			} else
			{
				bd = tick_data.counter[counter_i].loop_value;
				bd ++;
				if (bd < 1) bd = 1;
				if (bd >= MAX_LOOP) bd = MAX_LOOP-1;
				tick_data.counter[counter_i].loop_value = bd;
			}
		} else
		{
			if (CPARAM_MENU)
			{
				CPARAM_MENU = 0;
			} else
			if (PARAM_MENU)
			{
				PARAM_MENU = 0;
			} else
			{
				if (CUNIT_MENU)
				{	
					CUNIT_MENU = UNIT_MENU = 0;
				} else
				{
					UNIT_MENU = UNIT_MENU ? 0 : 1;

					if (UNIT_MENU)
					{		
						menu_x = mousex;
						menu_y = mousey;
					} else
					{
						CUNIT_MENU = CUNIT_MENU ? 0 : 1;
						if (CUNIT_MENU)
						{		
							menu_x = mousex;
							menu_y = mousey;
						}
					}
				}
			}
		}
	} else
	if (l_click)
	{
		if (CONT_AUDIO_LINK)
		{
			if (current && (parameter_i < current->parameters) && (parameter_i >= 0))
			{
				current->parameter[parameter_i].control = c_current;
				printf ("adding controller to para %d\n", parameter_i);
			}
			CONT_AUDIO_LINK = 0;
			PARAM_MENU = 0;
		} else
		if (CONT_AUDIO_DELETE_LINK)
		{
			DELETE_LINK = 0;
			CONT_AUDIO_DELETE_LINK = 0;
			if (current && c_current)
			{
				delete_cu_au_link (current, c_current);
			}
		} else
		if (UNIT_MENU)
		{
			UNIT_MENU = 0;
			if (menu_item >= 0)
			{
				audio_unitT	*au;
				au = au_register (audio_unit_ins[menu_item].new_method());
				give_au_tick (au);
				au->x = mousex;
				au->y = mousey;
			}
		} else 
		if (CUNIT_MENU)	
		{
			CUNIT_MENU = 0;
			if (menu_item >= 0)
			{
				control_unitT	*cu;
				cu = cu_register (control_unit_ins[menu_item].new_method());
				cu->x = mousex;
				cu->y = mousey;
			}
		} else
		if (counter_i >= 0)
		{
			if (counter_j >= 0)
			{
				tick_data.counter[counter_i].hit[counter_j] = 
					tick_data.counter[counter_i].hit[counter_j] ? 0 : 1;
			} else
			if (counter_div)
			{
				int bd;

				bd = tick_data.counter[counter_i].beat_div;
				if (l_click)
				{
					bd --;
					if (bd < 1) bd = 1;
				} 
				tick_data.counter[counter_i].beat_div = bd;
			} else
			{
				int bd;
				bd = tick_data.counter[counter_i].loop_value;
				bd --;
				if (bd < 1) bd = 1;
				if (bd >= MAX_LOOP) bd = MAX_LOOP-1;
				tick_data.counter[counter_i].loop_value = bd;
			}
		} 
	} else
	if (current && (b == 1) && (counter_i < 0))
	{
		int W, H;
		if (!INPUT_LINK && !OUTPUT_LINK && !DELETE_LINK && !PARAM_MENU && !UNIT_MENU)
		{
			if (current)
			{
				/*
				** Move current unit
				*/
				current->x = mousex;
				current->y = mousey;
				W = 30;
				H = 20;
			} 
		} else
		{			
			/*
			** Link units 
			*/
			if (focus && (focus != current)) 
			{
				if (INPUT_LINK)	au_add_input (current, focus);	
				if (OUTPUT_LINK) au_add_input (focus, current);
				if (DELETE_LINK) au_delete_link (current, focus);
			}
			deselect ();
		}
	} else
	if (c_current && (b == 1) && (counter_i < 0))
	{
		int W, H;
		if (!INPUT_LINK && !OUTPUT_LINK && !DELETE_LINK && !PARAM_MENU && !UNIT_MENU)
		{
			if (c_focus)
			{
				/*
				** Move current unit
				*/
				c_current->x = mousex;
				c_current->y = mousey;
				W = 30;
				H = 20;
			}
		} else
		{			
			/*
			** Link units 
			*/
			if (c_focus && (c_focus != c_current)) 
			{
				if (INPUT_LINK)	cu_add_input (c_current, c_focus);	
				if (OUTPUT_LINK) cu_add_input (c_focus, c_current);
				if (DELETE_LINK) cu_delete_link (c_current, c_focus);
			}
			deselect ();
		}
	} 

	if (CUNIT_MENU) cunit_menu();
	if (UNIT_MENU) unit_menu();
	if (PARAM_MENU) parameter_menu ();
	if (CPARAM_MENU) cparameter_menu ();


	if ((current || c_current) && key)
	{
		switch (key)
		{
			case 'I': clear_options(); INPUT_LINK = 1;  break;
			case 'O': clear_options(); OUTPUT_LINK = 1; break;
			case 'D': clear_options(); DELETE_LINK = 1; break;
			case 'R': clear_buffer_data (&current->out_buf); break;
			case ' ': trigger(current); break;
		}	
	}
	

	draw_counters ();

	blit_tc();
}