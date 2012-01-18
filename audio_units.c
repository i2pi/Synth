#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <math.h>
#include "sound.h"
#include "mathmat.h"
#include "audio_units.h"
#include "control_unit.h"


float	den_id = 1.0E-25f;

audio_unitT	*au_new (char *short_name, char *long_name)
{
	audio_unitT		*au;

	au = (audio_unitT *) malloc (sizeof (audio_unitT));
	sprintf (au->short_name, short_name);
	sprintf (au->long_name,  long_name);
	au->inputs = 0;
	au->id = get_id();
	au->parameters = 0;
	init_buffer_data (BUF_SIZE, &au->out_buf);
	return (au);
}


float	nicer (float x)
{	
	
	/*
	** Limit to [-1, +1]
	*/

//	x = (float) tanh (x);

	/*
	** Denormal
	*/
	
//	x += den_id;
//	den_id = -den_id;

	return (x);
}

/*
** RING MOD
*/


void	au_ring_proc (struct audio_unitT *au)
{
	int		j, i;

	if (!au_proc_time_check (au)) return; 

	for (i=0; i<BUF_SIZE; i++)
	{			
		au->out_buf.l_buffer[i] = 1.0f;
		au->out_buf.r_buffer[i] = 1.0f;
	}

	for (j=0; j<au->inputs; j++)
	{
		au->input[j]->proc(au->input[j]);
		for (i=0; i<BUF_SIZE; i++)
		{
			au->out_buf.l_buffer[i] *= au->input[j]->out_buf.l_buffer[i];
			au->out_buf.r_buffer[i] *= au->input[j]->out_buf.r_buffer[i];
		}
	}

	for (i=0; i<BUF_SIZE; i++)
	{			
		au->out_buf.l_buffer[i] = nicer (au->out_buf.l_buffer[i]);
		au->out_buf.r_buffer[i] = nicer (au->out_buf.r_buffer[i]);
	}


}

audio_unitT		*au_ring_new(void)
{
	audio_unitT		*au;

	au = au_new ("rng", "ring mod");
	au->proc = au_ring_proc;
	return (au);
}


/*
** NULL UNIT
*/

void	au_null_proc (struct audio_unitT *au)
{
	if (!au_proc_time_check (au)) return; 
	au_merge_inputs_to_output (au);
}

audio_unitT		*au_null_new(void)
{
	audio_unitT		*au;

	au = au_new ("nul", "null");
	return (au);
}


/*
** i2PiVERB(tm)
*/

void	au_verb_proc (struct audio_unitT	*au)
{
	int				i;
	float			x;
	verb_dataT		*d;
 
	if (!au_proc_time_check (au)) return;  
	cu_parameter_process (au->parameter, au->parameters);

	d = (verb_dataT *) au->data;

	if (au->inputs == 0) return;

	au->status = (d->dl->stretch / 100.0f) * 0.8f + au->status*0.2f;
	
	d->dl->tap[1].gforward = -0.9f * d->will;
	d->dl->tap[10].gback = 0.13f * d->make;
	d->dl->tap[9].gback = 0.12f * d->you;
	d->dl->tap[9].gforward = -0.07f * d->cry;
	d->dl->tap[3].gback =  0.071f * d->bloody;
	d->dl->tap[5].gback =  0.07f * d->tears;
	d->dl->tap[6].gforward =  0.1f * d->will;

	au_merge_inputs_to_buf (au, &d->buf);
	
	for (i=0; i<BUF_SIZE; i++)
	{
		x = i2convolve (d->dl, d->buf.l_buffer[i]);
		au->out_buf.l_buffer[i] = nicer(x);
		au->out_buf.r_buffer[i] = nicer(x);
	}
}

audio_unitT		*au_verb_new (void)
{
	audio_unitT		*au;
	verb_dataT		*d;

	
	au = au_new ("vrb", "verbist");
	au->proc = au_verb_proc;
	d = (verb_dataT *) malloc (sizeof (verb_dataT));
	init_buffer_data (BUF_SIZE, &d->buf);
	d->dl = i2new_delay (16384, 12, 3, 100.0f);
	//i2shape_taps (d->dl, 1.2f, 0.2f,  2.1f, 0.8f);
	d->dl->tap[1].gforward = -0.9f;
	d->dl->tap[10].gback = 0.13f;
	d->dl->tap[9].gback = 0.12f;
	d->dl->tap[9].gforward = -0.07f;
	d->dl->tap[3].gback =  0.071f;
	d->dl->tap[5].gback =  0.07f;
	d->dl->tap[6].gforward =  0.1f;
	au->data = (void *) d;
	au_new_parameter ((void*)&d->stretch, au, "stretch", FLOAT, 0.0f, 100.0f, 10.0f);
	au_new_parameter ((void*)&d->offset, au, "offset", INT, 0.0f, 100.0f, 3.0f);
	au_new_parameter ((void*)&d->will, au, "will", FLOAT,	-1.8f, 1.8f, 0.01f);
	au_new_parameter ((void*)&d->make, au, "make", FLOAT,	-1.8f, 1.8f, 0.01f);
	au_new_parameter ((void*)&d->you, au, "you", FLOAT,		-1.8f, 1.8f, 0.01f);
	au_new_parameter ((void*)&d->cry, au, "cry", FLOAT,		-1.8f, 1.8f, 0.01f);
	au_new_parameter ((void*)&d->bloody, au, "bloody", FLOAT, -1.8f, 1.8f, 0.01f);
	au_new_parameter ((void*)&d->tears, au, "tears", FLOAT, -1.8f, 1.8f, 0.01f);
	

	return (au);
}


/*
** ABS DRUM
*/

void	au_abs_drum_proc (struct audio_unitT *au)
{
	int				i;
	float			x;
	bass_drum_dataT	*bd;

	if (!au_proc_time_check (au)) return;  
	cu_parameter_process (au->parameter, au->parameters);

	bd = (bass_drum_dataT *) au->data;
	
	if (bd->trigger) { bd->amp = 1.0f; bd->trigger = 0; bd->phase = 0.0f; }
	au->status = bd->amp;
	for (i=0; i<BUF_SIZE; i++)
	{
		x = (float) Sin(bd->phase);
		x = (float) Sin(x*x*x);
		x = nicer(bd->amp * x * bd->volume);
		au->out_buf.l_buffer[i] = x;
		au->out_buf.r_buffer[i] = x;
		bd->phase += bd->dphase + bd->amp*0.001f;
		if (bd->amp > 0.001f) bd->amp -= 0.0001f;
	}
}

audio_unitT		*au_abs_drum_new(void)
{
	audio_unitT		*au;
	bass_drum_dataT	*bd;

	au = au_new ("abs", "abs synth");
	au->proc = au_abs_drum_proc;
	bd = (bass_drum_dataT *) malloc (sizeof (bass_drum_dataT));
	bd->trigger = 0;
	bd->phase = 0.0f;
	bd->dphase = 0.01f;
	bd->amp = 0.0;
	au->data = (void *) bd;
	au_new_parameter ((void*)&bd->trigger, au, "trigger", TRIGGER, 0.0f, 1.0f, 0.01f);
	au_new_parameter ((void*)&bd->dphase, au, "pitch", FLOAT, 0.0f, 0.2f, 0.01f);
	au_new_parameter ((void*)&bd->volume, au, "volume", FLOAT, 0.0f, 1.0f, 0.01f);
	au_new_parameter ((void*)&bd->decay, au, "decay", FLOAT, 0.0f, 1.0f, 0.01f);
	return (au);
}

/*
** ADSR
*/

void	au_adsr_proc (struct audio_unitT *au)
{
	int				i, I;
	adsr_dataT		*d;


	if (!au_proc_time_check (au)) return;  
	cu_parameter_process (au->parameter, au->parameters);

	d = (adsr_dataT *) au->data;
	au->status = d->amp;

	au_merge_inputs_to_buf (au, &au->out_buf);

	
	if (d->trigger) 
	{
		d->phase = 0;
		d->amp = 0.0f;
		d->trigger = 0;
	} else
	if (d->phase == -1)
	{
		for (i=0; i<BUF_SIZE; i++)
		{
			au->out_buf.l_buffer[i] = 0.0f;
			au->out_buf.r_buffer[i] = 0.0f;
		}
		return;
	}

	i = 0;
	if (d->phase == 0)
	{
		I = (int)((1.0f - d->amp) / d->A) + 1;
		I = (BUF_SIZE < I) ? BUF_SIZE : I;
		printf ("Attack Time:  %d\n", I);
		for (; i<I; i++)
		{
			d->amp += d->A;
			au->out_buf.l_buffer[i] *= d->amp;
			au->out_buf.r_buffer[i] *= d->amp;
		}
		if (d->amp >= 0.9f) 
		{
			d->phase = 1;
			d->amp = 1.0f;
		}
	}

	if (d->phase == 1)
	{
		I = (int)((d->amp - d->S) / d->D) + i + 1;
		I = (BUF_SIZE < I) ? BUF_SIZE : I;
		printf ("Delay Time :  %d\n", I);
		for (; i<I; i++)
		{
			d->amp -= d->D;
			au->out_buf.l_buffer[i] *= d->amp;
			au->out_buf.r_buffer[i] *= d->amp;
		}
		if (d->amp <= d->S) 
		{
			d->phase = 2;
			d->amp = d->S;
		}
	}

	if (d->phase == 2)
	{
		d->phase = 3;
	}

	if (d->phase == 3)
	{
		I = (int)(d->amp / d->R) + i + 1;
		I = (BUF_SIZE < I) ? BUF_SIZE : I;
		printf ("Release Time: %d\n", I);
		for (; i<I; i++)
		{
			d->amp -= d->R;
			au->out_buf.l_buffer[i] *= d->amp;
			au->out_buf.r_buffer[i] *= d->amp;
		}
		if (d->amp <= 0.0f) 
		{
			d->phase = -1;
			d->amp = 0.0f;
		}
	}
	
}

audio_unitT		*au_adsr_new(void)
{
	audio_unitT		*au;
	adsr_dataT	*d;


	au = au_new ("env", "adsr envelope");

	au->proc = au_adsr_proc;
	d = (adsr_dataT *) malloc (sizeof (adsr_dataT));
	d->trigger = 0;
	d->phase = -1;
	au->data = (void *) d;
	au_new_parameter ((void*)&d->trigger, au, "trigger", TRIGGER, 0.0f, 1.0f, 0.01f);
	au_new_parameter ((void*)&d->A, au, "attack time", FLOAT, 0.0f, 0.001f, 0.0001f);
	au_new_parameter ((void*)&d->D, au, "decay time", FLOAT, 0.0f, 0.001f, 0.0001f);
	au_new_parameter ((void*)&d->S, au, "sustain level", FLOAT, 0.0f, 1.0f, 0.2f);
	au_new_parameter ((void*)&d->St, au, "sustain time", FLOAT, 0.0f, 0.1f, 0.002f);
	au_new_parameter ((void*)&d->R, au, "release time", FLOAT, 0.0f, 0.001f, 0.0001f);
	
	return (au);
}



/*
** HARMONY
*/

void	au_harmony_proc (struct audio_unitT *au)
{
	int				i, j;
	float			x, harm;
	harmony_dataT	*d;


	if (!au_proc_time_check (au)) return;  
	cu_parameter_process (au->parameter, au->parameters);

	d = (harmony_dataT *) au->data;
	
	if (d->trigger) { d->amp = 1.0f / ((float)d->parts + 1.0f); d->trigger = 0; d->phase = 0.0f; }

	au->status = d->amp;

	for (i=0; i<BUF_SIZE; i++)
	{
		x = 0.0;
		harm = d->harm;
		for (j=0; j<d->parts; j++)
		{
			x+= (float) Sin(d->phase * harm + harm);
			harm *= harm;
		}
		x *= d->amp * d->volume;	
		x = nicer(x);
		au->out_buf.l_buffer[i] = x;
		au->out_buf.r_buffer[i] = x;
		d->phase += d->dphase;
		if (d->amp > 0.001f) d->amp -= 0.00003f*d->decay; else d->amp = 0.0f;
	}
}

audio_unitT		*au_harmony_new(void)
{
	audio_unitT		*au;
	harmony_dataT	*d;

	au = au_new ("hrm", "harmony");

	au->proc = au_harmony_proc;
	d = (harmony_dataT *) malloc (sizeof (harmony_dataT));
	d->trigger = 0;
	d->phase = 0.0f;
	d->dphase = 0.1f;
	d->parts = 4;
	d->harm = 1.08f;
	d->decay = 1.0f;
	d->volume = 1.0f;
	d->amp = 0.0;
	au->data = (void *) d;
	au_new_parameter ((void*)&d->trigger, au, "trigger", TRIGGER, 0.0f, 1.0f, 0.01f);
	au_new_parameter ((void*)&d->parts, au, "parts", INT, 0.0f, 5.0f, 4.0f);
	au_new_parameter ((void*)&d->dphase, au, "pitch", FLOAT, 0.0f, 1.0f, 0.1f);
	au_new_parameter ((void*)&d->harm, au, "harm", FLOAT, 0.0f, 5.0f, 1.08f);
	au_new_parameter ((void*)&d->volume, au, "volume", FLOAT, 0.0f, 1.0f, 1.0f);
	au_new_parameter ((void*)&d->decay, au, "decay", FLOAT, 0.0f, 1.0f, 1.0f);
	
	return (au);
}

/*
** HARMONY2
*/

void	au_harmony2_proc (struct audio_unitT *au)
{
	int				i, j, h;
	float			x, p, a;
	harmony2_dataT	*d;

	if (!au_proc_time_check (au)) return;  
	cu_parameter_process (au->parameter, au->parameters);

	d = (harmony2_dataT *) au->data;
	
	au->status = d->amp;

	for (i=0; i<BUF_SIZE; i++)
	{
		x = 0.0;
		h = 0;
		p = 0.0f;	
		a = d->amp;
		for (j=0; j<d->parts; j++)
		{
			x+= (float) (a * Sin(d->phase * (h+1) + p));
			h = h + 2;
			p += d->even_phase;
			a *= d->even_amp;
		}
		h = 1;
		p = 0.0f;	
		a = d->amp;
		for (j=0; j<d->parts; j++)
		{
			x+= (float) (a * Sin(d->phase * (h+1) + p));
			h = h + 2;
			p += d->odd_phase;
			a *= d->odd_amp;
		}
		x *= d->volume;	
		x = nicer(x);
		au->out_buf.l_buffer[i] = x;
		au->out_buf.r_buffer[i] = x;
		d->phase += d->dphase;
		if (d->phase > PI*500000.0) d->phase = 0.0;
		//if (d->amp > 0.001f) d->amp -= 0.00003f*d->decay; else d->amp = 0.0f;
	}
}

audio_unitT		*au_harmony2_new(void)
{
	audio_unitT		*au;
	harmony2_dataT	*d;

	au = au_new ("hr2", "harmony2");

	au->proc = au_harmony2_proc;
	d = (harmony2_dataT *) malloc (sizeof (harmony2_dataT));
	d->phase = 0.0f;
	d->dphase = 0.1f;
	d->parts = 4;
	d->decay = 1.0f;
	d->volume = 1.0f;
	d->amp = 1.0f;
	au->data = (void *) d;
	au_new_parameter ((void*)&d->parts, au, "parts", INT, 0.0f, 15.0f, 4.0f);
	au_new_parameter ((void*)&d->dphase, au, "pitch", FLOAT, 0.0f, 0.1f, 0.01f);
	au_new_parameter ((void*)&d->even_amp, au, "even amp", FLOAT, 0.0f, 1.0f, 0.3f);
	au_new_parameter ((void*)&d->even_phase, au, "even phase", FLOAT, 0.0f, 6.0f, 0.01f);
	au_new_parameter ((void*)&d->odd_amp, au, "odd amp", FLOAT, 0.0f, 1.0f, 0.3f);
	au_new_parameter ((void*)&d->odd_phase, au, "odd phase", FLOAT, 0.0f, 6.0f, 0.01f);
	au_new_parameter ((void*)&d->volume, au, "volume", FLOAT, 0.0f, 1.0f, 1.0f);
	au_new_parameter ((void*)&d->decay, au, "decay", FLOAT, 0.0f, 1.0f, 1.0f);
	
	return (au);
}
/*
** BASS DRUM
*/

void	au_bass_drum_proc (struct audio_unitT *au)
{
	int				i;
	float			x;
	bass_drum_dataT	*bd;

	if (!au_proc_time_check (au)) return;  
	cu_parameter_process (au->parameter, au->parameters);

	bd = (bass_drum_dataT *) au->data;
	
	if (bd->trigger) { bd->amp = 1.0f; bd->trigger = 0; bd->phase = 0.0f; }

	au->status = bd->amp;
	
	for (i=0; i<BUF_SIZE; i++)
	{
		x = (float) (bd->amp * Sin (bd->phase));
		x = nicer(x * bd->volume);
		au->out_buf.l_buffer[i] = x;
		au->out_buf.r_buffer[i] = x;
		bd->phase += bd->dphase + bd->amp*0.001f*bd->decay;
		if (bd->amp > 0.001f) bd->amp -= 0.0015f*bd->decay;
	}
}

audio_unitT		*au_bass_drum_new(void)
{
	audio_unitT		*au;
	bass_drum_dataT	*bd;

	au = au_new ("bd", "bass drum");

	au->inputs = 0;
	au->proc = au_bass_drum_proc;
	bd = (bass_drum_dataT *) malloc (sizeof (bass_drum_dataT));
	bd->trigger = 0;
	bd->phase = 0.0f;
	bd->dphase = 0.01f;
	bd->decay = 1.0f;
	bd->amp = 0.0f;
	au->data = (void *) bd;
	au_new_parameter ((void*)&bd->trigger, au, "trigger", TRIGGER, 0.0f, 1.0f, 0.01f);
	au_new_parameter ((void*)&bd->dphase, au, "pitch", FLOAT, 0.01f, 0.09f, 0.01f);
	au_new_parameter ((void*)&bd->volume, au, "volume", FLOAT, 0.0f, 1.0f, 0.01f);
	au_new_parameter ((void*)&bd->decay, au, "decay", FLOAT, 0.0f, 1.0f, 0.01f);
	return (au);
}

/*
** HI HAT
*/

void	au_hat_proc (struct audio_unitT *au)
{
	int				i;
	float			x;
	hat_dataT		*bd;

	if (!au_proc_time_check (au)) return;  
	cu_parameter_process (au->parameter, au->parameters);

	bd = (hat_dataT *) au->data;
	
	if (bd->trigger) { bd->amp = 1.0f; bd->trigger = 0; }

	au->status = bd->amp;

	for (i=0; i<BUF_SIZE; i++)
	{
		x = nicer((float) ((bd->amp * Sin(rand()))) * bd->volume);
		au->out_buf.l_buffer[i] = x;
		au->out_buf.r_buffer[i] = x;
		if (bd->amp > 0.001f) bd->amp -= 0.001f;
	}
}

audio_unitT		*au_hat_new(void)
{
	audio_unitT		*au;
	hat_dataT	*bd;

	au = au_new ("hat", "hi hat");
	au->proc = au_hat_proc;
	bd = (hat_dataT *) malloc (sizeof (hat_dataT));
	bd->trigger = 0;
	bd->amp = 0.0;
	au->data = (void *) bd;
	au_new_parameter ((void*)&bd->trigger, au, "trigger", TRIGGER, 0.0f, 1.0f, 0.01f);
	au_new_parameter ((void*)&bd->volume, au, "volume", FLOAT, 0.0f, 1.0f, 1.0f);
	
	return (au);
}

/*
** 4-POLE IIR 
*/

void	au_pole4_iir_proc (struct audio_unitT *au)
{
	int				i, j;
	float			xL, xR, l[4], r[4];
	pole4_iir_dataT		*d;

	d = (pole4_iir_dataT *) au->data;

	if (!au_proc_time_check (au)) return;  
	cu_parameter_process (au->parameter, au->parameters);

	if (au->inputs == 0) return;

	d->f_l[0] = (float)(0.10f*Sin(d->suck*0.0001*Pa_StreamTime(stream)));
	d->f_l[1] = (float)(0.15f*Cos(d->my*0.00001*Pa_StreamTime (stream)));
	d->f_l[2] = (float)(0.15f*Sin(d->shiny*0.00009*Pa_StreamTime (stream)) + 0.1*Cos(0.00007*Pa_StreamTime (stream)));
	d->f_l[3] = (float)(0.15f*Cos(d->brass*3.0 + d->balls*0.00007*Pa_StreamTime (stream)));
	
	d->f_r[0] = (float)(0.10f*Sin(d->suck*0.0001*Pa_StreamTime(stream) + 0.1));
	d->f_r[1] = (float)(0.15f*Cos(d->my*0.00007*Pa_StreamTime (stream)));
	d->f_r[2] = (float)(0.15f*Sin(d->shiny*0.00009*Pa_StreamTime (stream)) + 0.1*Cos(0.00006*Pa_StreamTime (stream)));
	d->f_r[3] = (float)(0.15f*Cos(d->brass*3.0 + d->balls*0.00001*Pa_StreamTime (stream)));

	au->status = d->f_l[0];

	/*
	** Store the last 4 outputs before we clear the buffer
	*/

	for (i=0; i<4; i++)
	{
		l[i] = d->buf.l_buffer[BUF_SIZE - i - 1];
		r[i] = d->buf.r_buffer[BUF_SIZE - i - 1];
	}

	/* 
	** Get new audio buffer
	*/

	au_merge_inputs_to_buf (au, &d->buf);

	for (i=0; i<4; i++)
	{

		xL = 0.0;
		xR = 0.0;
		for (j=0; j<i+1; j++)
		{
			xL += d->f_l[j] * d->buf.l_buffer[i-j];						
			xR += d->f_r[j] * d->buf.r_buffer[i-j];
		}
		for (j=i+1; j<4; j++)
		{
			xL += d->f_l[j] * l[j];
			xR += d->f_r[j] * r[j];
		} 
		d->buf.l_buffer[i] = au->out_buf.l_buffer[i] = nicer(xL);
		d->buf.r_buffer[i] = au->out_buf.r_buffer[i] = nicer(xR);
	}

	for (i=4; i<BUF_SIZE; i++)
	{

		xL = 0.0;
		xR = 0.0;
		for (j=0; j<4; j++)
		{
			xL += d->f_l[j] * d->buf.l_buffer[i-j];						
			xR += d->f_r[j] * d->buf.r_buffer[i-j];
		}
		d->buf.l_buffer[i] = au->out_buf.l_buffer[i] = nicer(xL);
		d->buf.r_buffer[i] = au->out_buf.r_buffer[i] = nicer(xR);
	}
}

audio_unitT		*au_4pole_iir_new(void)
{
	audio_unitT		*au;
	pole4_iir_dataT	*d;

	au = au_new ("iir", "4pole iir");
	au->proc = au_pole4_iir_proc;
	d = (pole4_iir_dataT *) malloc (sizeof (pole4_iir_dataT));
	init_buffer_data (BUF_SIZE, &d->buf);
	au->data = (void *) d;
	au_new_parameter ((void*)&d->suck, au, "suck", FLOAT, 0.0f, 0.5f, 0.1f);
	au_new_parameter ((void*)&d->my, au, "my", FLOAT, 0.0f, 0.5f, 0.1f);
	au_new_parameter ((void*)&d->shiny, au, "shiny", FLOAT, 0.0f, 0.5f, 0.1f);
	au_new_parameter ((void*)&d->brass, au, "brass", FLOAT, 0.0f, 0.5f, 0.1f);
	au_new_parameter ((void*)&d->brass, au, "balls", FLOAT, 0.0f, 0.5f, 0.1f);


	return (au);
}


/*
** MOOG FILTER
*/

void	au_moog_proc (struct audio_unitT *au)
{
	int				i;
	float			x;
	moog_dataT		*d;
	float			f; 
	float			fb;
  
	d = (moog_dataT *) au->data;
	if (!au_proc_time_check (au)) return;  
	if (au->inputs == 0) return;
	cu_parameter_process (au->parameter, au->parameters);

	au->status = 1.8f*d->fc;

	f = d->fc * 1.16f;
	fb = d->res * (1.0f - 0.15f * f * f);
  

	clear_buffer_data (&d->buf);

	/* 
	** Get new audio buffer
	*/
	au_merge_inputs_to_buf (au, &d->buf);
	
	for (i=0; i<BUF_SIZE; i++)
	{
		x = d->buf.l_buffer[i];						
		
		x -= (float)(d->out4 * fb);
		x *= (float)(0.35013f * (f*f)*(f*f));
		d->out1 = nicer(x       + 0.3f * d->in1 + (1 - f) * d->out1); // Pole 1
		d->in1  = nicer(x);
		d->out2 = nicer(d->out1 + 0.3f * d->in2 + (1 - f) * d->out2);  // Pole 2
		d->in2  = d->out1;
		d->out3 = nicer(d->out2 + 0.3f * d->in3 + (1 - f) * d->out3);  // Pole 3
		d->in3  = d->out2;
		d->out4 = nicer(d->out3 + 0.3f * d->in4 + (1 - f) * d->out4);  // Pole 4
		d->in4  = d->out3;
  
		au->out_buf.l_buffer[i] = (float)d->out4;
		au->out_buf.r_buffer[i] = (float)d->out4;
	}
}

audio_unitT		*au_moog_filter_new (void)
{
	audio_unitT		*au;
	moog_dataT		*d;

	au = au_new ("mog", "moog filter");
	au->proc = au_moog_proc;
	d = (moog_dataT *) malloc (sizeof (moog_dataT));
	memset (d, 0, sizeof (moog_dataT));
	d->fc = 0.1f;
	d->res = 1.0f;
	init_buffer_data (BUF_SIZE, &d->buf);
	au->data = (void *) d;
	au_new_parameter ((void*)&d->fc, au, "freq", FLOAT, 0.01f, 0.3f, 0.1f);
	au_new_parameter ((void*)&d->res, au, "res", FLOAT, 0.0f, 3.2f, 1.0f);
	
	return (au);
}

/*
** WS1
*/

void	au_ws1_proc (struct audio_unitT *au)
{
	int				i;
	float			x, y;
	float			amount, k;
  
	
	amount = *((float *) au->data);
	if (!au_proc_time_check (au)) return;  
	if (au->inputs == 0) return;
	cu_parameter_process (au->parameter, au->parameters);
	
	k = 2*amount/(1-amount);

	

	au->status = (amount + 1.0f) * 0.5f;

	au_merge_inputs_to_buf (au, &au->out_buf);
	
	for (i=0; i<BUF_SIZE; i++)
	{
		x = au->out_buf.l_buffer[i];						
		y = au->out_buf.r_buffer[i];
		x = (float)((1.0f+k)*x/(1.0f+k*fabs(x)));
		y = (float)((1.0f+k)*y/(1.0f+k*fabs(y)));
		au->out_buf.l_buffer[i] = nicer(x);
		au->out_buf.r_buffer[i] = nicer(y);
	}
}

audio_unitT		*au_ws1_new (void)
{
	audio_unitT		*au;

	au = au_new ("ws1", "ws1 shaper");
	au->proc = au_ws1_proc;
	au->data = malloc (sizeof (float));
	au_new_parameter (au->data, au, "amount", FLOAT, -1.0f, 1.0f, 0.1f);
	
	return (au);
}


/*
** UNIT_DELAY
*/

void	au_ud_proc (struct audio_unitT *au)
{
	int				i;
	float			l, r;
	unit_delay_dataT	*d;
  

	d = (unit_delay_dataT *) au->data;	
	if (!au_proc_time_check (au)) return;  
	if (au->inputs == 0) return;
	cu_parameter_process (au->parameter, au->parameters);
	
	au->status = 0.5f;

	au_merge_inputs_to_buf (au, &au->out_buf);
	
	d->ol = au->out_buf.l_buffer[BUF_SIZE-1];
	d->or = au->out_buf.r_buffer[BUF_SIZE-1];

	for (i=0; i<BUF_SIZE; i++)
	{
		l = au->out_buf.l_buffer[i];						
		r = au->out_buf.r_buffer[i];
		l = nicer(l + d->lf*d->ol);
		r = nicer(r + d->rf*d->or);
		d->ol = au->out_buf.l_buffer[i] = (float)l;
		d->or = au->out_buf.r_buffer[i] = (float)r;
	}
}

audio_unitT		*au_ud_new (void)
{
	audio_unitT			*au;
	unit_delay_dataT	*d;

	au = au_new ("udl", "unit delay");
	au->proc = au_ud_proc;
	au->data = malloc (sizeof (unit_delay_dataT));
	d = (unit_delay_dataT *) au->data;
	au_new_parameter ((void *)&d->lf, au, "left", FLOAT, -0.5f, 0.5f, 0.0f);
	au_new_parameter ((void *)&d->rf, au, "right", FLOAT, -0.5f, 0.5f, 0.0f);
	
	return (au);
}


/*
** GAIN
*/

void	au_gain_proc (struct audio_unitT *au)
{
	int				i;	
	float			l, r;
	gain_dataT		*d;
  

	d = (gain_dataT *) au->data;	
	if (!au_proc_time_check (au)) return;  
	if (au->inputs == 0) return;
	cu_parameter_process (au->parameter, au->parameters);
	
	au->status = (d->l + d->r) * 0.25f + 0.5f;

	au_merge_inputs_to_buf (au, &au->out_buf);
	
	for (i=0; i<BUF_SIZE; i++)
	{
		l = au->out_buf.l_buffer[i];						
		r = au->out_buf.r_buffer[i];
		l *= d->l;
		r *= d->r;
		au->out_buf.l_buffer[i] = (float)l;
		au->out_buf.r_buffer[i] = (float)r;
	}
}

audio_unitT		*au_gain_new (void)
{
	audio_unitT			*au;
	gain_dataT			*d;

	au = au_new ("gan", "gain");
	au->proc = au_gain_proc;
	au->data = malloc (sizeof (gain_dataT));
	d = (gain_dataT *) au->data;
	au->parameters = 0;
	au_new_parameter ((void *)&d->l, au, "left", FLOAT, -1.0f, 1.0f, 1.0f);
	au_new_parameter ((void *)&d->r, au, "right", FLOAT, -1.0f, 1.0f, 1.0f);
		
	return (au);
}


/*
** CLIP
*/

void	au_clip_proc (struct audio_unitT *au)
{
	int				i;	
	float			l, r;
	gain_dataT		*d;
  

	d = (gain_dataT *) au->data;	
	if (!au_proc_time_check (au)) return;  
	if (au->inputs == 0) return;
	cu_parameter_process (au->parameter, au->parameters);
	
	au->status = (d->l + d->r) * 0.25f + 0.5f;

	au_merge_inputs_to_buf (au, &au->out_buf);
	
	for (i=0; i<BUF_SIZE; i++)
	{
		l = au->out_buf.l_buffer[i];						
		r = au->out_buf.r_buffer[i];
		l *= d->l;
		r *= d->r;
		if (l > 1.0f) l = 1.0f; else
		if (l < -1.0f) l = -1.0f;
		if (r > 1.0f) r = 1.0f; else
		if (r < -1.0f) r = -1.0f;
		au->out_buf.l_buffer[i] = (float)l;
		au->out_buf.r_buffer[i] = (float)r;
	}
}

audio_unitT		*au_clip_new (void)
{
	audio_unitT			*au;
	gain_dataT			*d;

	au = au_new ("clp", "clip");
	au->proc = au_gain_proc;
	au->data = malloc (sizeof (gain_dataT));
	d = (gain_dataT *) au->data;
	au->parameters = 0;
	au_new_parameter ((void *)&d->l, au, "left", FLOAT, 0.0f, 5.0f, 1.0f);
	au_new_parameter ((void *)&d->r, au, "right", FLOAT, 0.0f, 5.0f, 1.0f);
		
	return (au);
}