#ifndef _AUDIO_UNITS_H_
#define _AUDIO_UNITS_H_

#include <stdlib.h>


#include "verb.h"
#include "sound.h"
#include "mathmat.h"


audio_unitT		*au_null_new (void);
audio_unitT		*au_bypass_new (void);



/*
** i2PiVERB(tm)
*/

typedef struct
{	
	i2delay_lineT	*dl;
	buffer_dataT	buf;
	float			stretch;
	int				offset;
	float			will;
	float			make;
	float			you;
	float			cry;
	float			bloody;
	float			tears;
} verb_dataT;

audio_unitT		*au_verb_new (void);

audio_unitT		*au_ws1_new (void);


/*
** HARMONY2
*/

typedef struct
{
	int		parts;
	float	even_amp;
	float	even_phase;
	float	odd_amp;
	float	odd_phase;
	float	decay;
	float	amp;
	double	phase;
	float	dphase;
	float	volume;
} harmony2_dataT;

audio_unitT		*au_harmony2_new(void);

/*
** HARMONY
*/

typedef struct
{
	int		trigger;
	int		parts;
	float	harm;
	float	phase;
	float	decay;
	float	amp;
	float	dphase;
	float	volume;
} harmony_dataT;

audio_unitT		*au_harmony_new(void);

/*
** ADSR
*/

typedef struct
{
	int		trigger;
	float	A, D, S, St, R;
	int		phase;
	float	amp;
	float	volume;
} adsr_dataT;

audio_unitT		*au_adsr_new(void);

/*
** BASS DRUM
*/

typedef struct
{
	int		trigger;
	float	phase;	
	float	decay;
	float	amp;
	float	dphase;
	float	volume;
} bass_drum_dataT;

audio_unitT		*au_bass_drum_new(void);
audio_unitT		*au_abs_drum_new(void);



/*
** HI HAT
*/

typedef struct
{
	int		trigger;
	float	amp;	
	float	volume;
} hat_dataT;

audio_unitT		*au_hat_new(void);


/*
** 4-POLE IIR
*/

typedef struct
{
	buffer_dataT	buf;
	float			f_l[4];
	float			f_r[4];
	float			suck, my, shiny, brass, balls;
} pole4_iir_dataT;

audio_unitT		*au_4pole_iir_new(void);

/*
** MOOG FILTER
*/

typedef struct
{
	float			fc;			// 0 --> 1
	float			res;		// 0 --> 4
	float			in1, in2, in3, in4;
	float			out1, out2, out3, out4;
	buffer_dataT	buf;
} moog_dataT;

audio_unitT		*au_moog_filter_new (void);


/*
** UNIT_DELAY
*/

typedef struct
{
	float			ol, or;
	float			rf, lf;
} unit_delay_dataT;

audio_unitT		*au_ud_new(void);

audio_unitT		*au_ring_new(void);

/*
** GAIN
*/

typedef struct
{
	float		l, r;
} gain_dataT;

audio_unitT		*au_gain_new (void);
audio_unitT		*au_clip_new (void);


#endif