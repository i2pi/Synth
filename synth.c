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



int WIDTH=640;
int HEIGHT=480;



void Select_A_Mode() { wtcGo=1; usewindow_tc(1); }

int main()
{
	char	title[]="i2pi synth";

	settopic_tc(title);
	logoUpdate=Select_A_Mode; logoWidth=640; logoHeight=480;
	init_tc(WIDTH, HEIGHT);
	start_tc();
	init_font ();
	init_tick (SAMPLE_RATE, 550.0f);
	memset (tcono, 0, sizeof(unsigned long)*WIDTH*HEIGHT);
	pa_init (BUF_SIZE, SAMPLE_RATE);



	init_cu();
	init_au();

//	return(1);	
	
	

	Pa_StartStream( stream );	
	load ();
	init_tick (SAMPLE_RATE, 850.0f);
/*
	sprintf (audio_unit_root.dac.short_name, "dack");
	audio_unit_root.dac.parameters = 0;
	audio_unit_root.dac.inputs = 0;
	audio_unit_root.dac.id = get_id ();
	au_register(&audio_unit_root.dac);
*/
	DONE = 0;

	SCROLL_X = 0;	
	SCROLL_Y = 0;
	SCROLL_X=-audio_unit_root.dac.x + WIDTH/2;		
	SCROLL_Y=-audio_unit_root.dac.y + HEIGHT/2;
	CLIPX1 = 0; CLIPX2 = WIDTH;
	CLIPY1 = 0; CLIPY2 = HEIGHT;

	while (!DONE)
	{		
		draw_gui();		
		Pa_Sleep (40);
	}
	kill_tc ();
	pa_close ();
	return 1;
}

