#ifndef _GUI_H_
#define _GUI_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <tclib.h>
#include <math.h>

#include "portaudio.h"
#include "sound.h"
#include "Font.h"
#include "mathmat.h"
#include "rgb.h"
#include "tick.h"
#include "audio_units.h"
#include "gui.h"


#define BR_RED	0xff8080
#define DK_RED	0xa05050
#define FC_RED	0xff0000
#define BR_BLUE	0x8080ff
#define DK_BLUE	0x5050a0
#define FC_BLUE	0x0000ff


int	DONE;


void	layout_units (void);
void	draw_gui (void);

#endif