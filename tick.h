#ifndef _TICK_H_
#define _TICK_H_

#include "sound.h"

#define MAX_COUNTERS	32
#define MAX_LOOP		32
typedef struct
{
	int			beat_div;
	int			value;
	int			loop_value;
	int			hit[MAX_LOOP];
	audio_unitT	*au;
	int			p;
} counterT;

typedef struct
{
	double			samples_per_second;
	double			bpm;
	
	double			samples;
	double			seconds;
	unsigned long	beats;

	counterT		counter[MAX_COUNTERS];
	int				counters;

	double			seconds_per_beat;
	double			seconds_since_beat;
} tickT;


extern tickT	tick_data;

void	init_tick (float samples_per_second, float bpm);
void	tick_buffer (double outTime);
void	tick_add_counter (int init_value, int loop_value, audio_unitT *au, int p);
void	tick_delete_counter (int c);

#endif