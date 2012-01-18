
#include "tick.h"
#include "portaudio.h"
#include "sound.h"

tickT	tick_data;

void	init_tick (float samples_per_second, float bpm)
{
	tick_data.samples_per_second = samples_per_second;
	tick_data.bpm = bpm;
	tick_data.samples = Pa_StreamTime (stream);
	tick_data.seconds_per_beat = 60.0 / bpm;
	tick_data.seconds_since_beat = 0;
	tick_data.seconds = tick_data.samples / tick_data.samples_per_second;
}

void	tick_delete_counter (int c)
{
	int	k;
	for (k=c; k<tick_data.counters-1; k++)
	{
		tick_data.counter[k] = tick_data.counter[k+1];
	}
	tick_data.counters--;
}

void	tick_buffer (double outTime)
{
	int		c;
	double	old_seconds;

	old_seconds = tick_data.seconds;

	tick_data.samples = outTime ;//Pa_StreamTime (stream);
	tick_data.seconds = tick_data.samples / tick_data.samples_per_second;
	tick_data.seconds_since_beat += (tick_data.seconds - old_seconds);

	if (tick_data.seconds_since_beat >= tick_data.seconds_per_beat)
	{
		tick_data.seconds_since_beat = 0;
		tick_data.beats++;

		for (c=0; c<tick_data.counters; c++)
		if (tick_data.beats % tick_data.counter[c].beat_div == 0)
		{
			tick_data.counter[c].value++;
			if (tick_data.counter[c].value >= tick_data.counter[c].loop_value)
			{
				tick_data.counter[c].value=0;
			}
			if (tick_data.counter[c].hit[tick_data.counter[c].value])
			{
				*tick_data.counter[c].au->parameter[tick_data.counter[c].p].trigger_value = 1;
			}
		}
	}
}

void	tick_add_counter (int init_value, int loop_value, audio_unitT *au, int p)
{
	int	c, i;
	
	if (tick_data.counters == MAX_COUNTERS) return;
	c = tick_data.counters++;
	tick_data.counter[c].beat_div = 1;
	for (i=0; i<loop_value; i++)
	{		
		if (i % 3 == 1)
			tick_data.counter[c].hit[i] = 1;
		else
			tick_data.counter[c].hit[i] = 0;
	}
	
	tick_data.counter[c].value = init_value;
	tick_data.counter[c].loop_value = loop_value;
	tick_data.counter[c].au = au;
	tick_data.counter[c].p = p;
	
}