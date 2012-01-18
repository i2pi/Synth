#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portaudio.h"
#include "sound.h"
#include "tick.h"
#include "mathmat.h"
#include "sndfile.h"
#include "audio_units.h"

PortAudioStream *stream;

SNDFILE	*sndfile;

double	curOutTime;

int		get_id (void)
{
	return (++global_id);
}

void		init_au (void)
{
	au_register_ins ("hrm", "harmony", au_harmony_new);
	au_register_ins ("hr2", "harmony2", au_harmony2_new);
	au_register_ins ("bd", "kick drum", au_bass_drum_new);
	au_register_ins ("abs", "abs synth", au_abs_drum_new);
	au_register_ins ("hat", "hi hat", au_hat_new);

	au_register_ins ("gan", "fx:gain", au_gain_new);
	au_register_ins ("evn", "fx:adsr envelope", au_adsr_new);
	au_register_ins ("clp", "fx:clip", au_clip_new);
	au_register_ins ("vrb", "fx:i2piverb", au_verb_new);
	au_register_ins ("iir", "fx:4pole", au_4pole_iir_new);
	au_register_ins ("mog", "fx:moog", au_moog_filter_new);
	au_register_ins ("ws1", "fx:ws1 shaper", au_ws1_new);
	au_register_ins ("udl", "fx:unit delay", au_ud_new);
	au_register_ins ("rng", "fx:ring mod", au_ring_new);
	au_register_ins ("nul", "fx:null", au_null_new);
}

void		au_register_ins (char *short_name, char *name, audio_unitT *(*new_method)(void))
{
	int	i;

	i = audio_unit_inss++;
	sprintf (audio_unit_ins[i].name, name);
	sprintf (audio_unit_ins[i].short_name, short_name);
	audio_unit_ins[i].new_method = new_method;
}

audio_unitT		*au_register (struct audio_unitT	*au)
{
	int	i;
	
	i = 0;
	while ((i < MAX_UNITS) && audio_unit[i]) i++;
	
	if (i >= MAX_UNITS)
	{
		printf ("Too many units. Please pay for full version.\n");
		return (NULL);
	}

	audio_unit[i] = au;
	return (au);
}

int		au_proc_time_check (struct audio_unitT *au)
{
	if (au->out_buf.gen_time < curOutTime) 
	{
		au->out_buf.gen_time = curOutTime;
		return (1);
	}
	return (0);
}

void	au_merge_inputs_to_output (struct audio_unitT *au)
{
	int		j, i;

	clear_buffer_data (&au->out_buf);
	for (j=0; j<au->inputs; j++)
	{
		au->input[j]->proc(au->input[j]);
		for (i=0; i<BUF_SIZE; i++)
		{
			au->out_buf.l_buffer[i] += au->input[j]->out_buf.l_buffer[i];
			au->out_buf.r_buffer[i] += au->input[j]->out_buf.r_buffer[i];
		}
	}
}

void	au_merge_inputs_to_buf (struct audio_unitT *au, buffer_dataT *buf)
{
	int		j, i;

	clear_buffer_data (buf);
	for (j=0; j<au->inputs; j++)
	{
		au->input[j]->proc(au->input[j]);
		for (i=0; i<BUF_SIZE; i++)
		{
			buf->l_buffer[i] += au->input[j]->out_buf.l_buffer[i];
			buf->r_buffer[i] += au->input[j]->out_buf.r_buffer[i];
		}
	}
}

/*
** To chain units together
*/

void	au_add_input (audio_unitT *au, audio_unitT *input)
{	
	int	i;

	if (input == &audio_unit_root.dac) return;
	if (au->inputs >= MAX_INPUTS) return;
	i = au->inputs++;
	au->input[i] = input;	
}



void	au_remove_input (audio_unitT *a, audio_unitT *b)
{
	int		i, j;

	i=0;
	while ((i < a->inputs) && (a->input[i] != b)) i++;
	if (a->input[i] == b)
	{
		for (j=i+1; j<a->inputs; j++)
		{
			a->input[j-1] = a->input[j];
		}
		a->inputs -= 1;
	}
}

void	au_delete_link (audio_unitT *a, audio_unitT *b)
{
	int	i;

	/*
	** Check if it is an input link
	*/

	i = 0;
	while ((i < a->inputs) && (a->input[i] != b)) i++;
	if (a->input[i] == b)
	{
		au_remove_input (a, b);
		return;
	}

	i = 0;
	while ((i < b->inputs) && (b->input[i] != a)) i++;
	if (b->input[i] == a)
	{
		au_remove_input (b, a);
		return;
	}
}

void	au_new_parameter (void *vp, audio_unitT *au, char *name, param_type pt, float min, float max, float def)
{
	int	p;

	p = au->parameters++;
	sprintf (au->parameter[p].name, name);
	au->parameter[p].p_type = pt;
	au->parameter[p].control = NULL;
	if (pt == TRIGGER)
	{
		au->parameter[p].trigger_value = (int *) vp;
		*(au->parameter[p].trigger_value) = (int) def;
	} else
	if (pt == INT)
	{
		au->parameter[p].int_value = (int *) vp;
		au->parameter[p].int_min   = (int) min;
		au->parameter[p].int_max   = (int) max;
		*(au->parameter[p].int_value) = (int) def;
	} else
	if (pt == FLOAT)
	{
		au->parameter[p].float_value = (float *) vp;
		au->parameter[p].float_min   = min;
		au->parameter[p].float_max   = max;
		*(au->parameter[p].float_value) = def;
	}
}

static int patestCallback( void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           PaTimestamp outTime, void *userData )
{
    /* Cast data passed through stream to our structure. */
    SynthMainData *data = (SynthMainData*)userData;
    float *out = (float*)outputBuffer;
    int i, j;
	unsigned int	int_buf[BUF_SIZE * 2];
	unsigned int	*ibp;
	float	vol;

    (void) outTime; /* Prevent unused variable warnings. */
    (void) inputBuffer;

	curOutTime = outTime;

	clear_buffer_data (&data->buf);

	tick_buffer (outTime);


	for (i=0; i<data->dac.inputs; i++)
	{
		data->dac.input[i]->proc(data->dac.input[i]);
		for (j=0; j<BUF_SIZE; j++)
		{
			data->buf.l_buffer[j] += data->dac.input[i]->out_buf.l_buffer[j];
			data->buf.r_buffer[j] += data->dac.input[i]->out_buf.r_buffer[j];
		}
	}

	ibp = int_buf;
	for( i=0; i<BUF_SIZE; i++ )
    {
		*out++ = (float)Sin(data->buf.l_buffer[i]);				
		*out++ = (float)Sin(data->buf.r_buffer[i]);	
	}

	vol = 0.0f;
	for (i=0; i<BUF_SIZE; i+= 50)
	{
		vol += ((data->buf.r_buffer[i]*data->buf.r_buffer[i]) +
			   (data->buf.r_buffer[i]*data->buf.r_buffer[i]));
	}
	audio_unit_root.dac.status = vol / (float) (BUF_SIZE / 50.0f);
		
	if (DISK_WRITE)
		sf_writef_float (sndfile, (float *) outputBuffer, BUF_SIZE);
	
	
    return 0;
}


void	clear_buffer_data (buffer_dataT *buf)
{
	memset (buf->l_buffer, 0, sizeof (float) * BUF_SIZE);
	memset (buf->r_buffer, 0, sizeof (float) * BUF_SIZE);
}

void	init_buffer_data (int buf_size, buffer_dataT *buf)
{
	buf->l_buffer = (float *) malloc (sizeof(float) * BUF_SIZE);
	buf->r_buffer = (float *) malloc (sizeof(float) * BUF_SIZE);
	clear_buffer_data (buf);
	buf->gen_time = -1.0;
}


void	add_root_unit (audio_unitT *au)
{
	int		i;

	if (audio_unit_root.dac.inputs >= MAX_INPUTS) return;

	i = audio_unit_root.dac.inputs++;
	audio_unit_root.dac.input[i] = au;	
}


int pa_init(int	buf_size, int sample_rate)
{
    PaError err;
	SF_INFO	sf_info;
	int		i;

	for (i=0; i<MAX_UNITS; i++)
	{
		audio_unit[i] = NULL;
	}
	audio_unit_inss = 0;
	DISK_WRITE = 1;
	global_id = 0;


    err = Pa_Initialize();
    if( err != paNoError ) goto error;

	init_buffer_data (buf_size, &audio_unit_root.buf);
	
	sf_info.channels = 2;
	sf_info.samplerate = sample_rate;
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	sndfile = sf_open ("i2piSynth.wav", SFM_WRITE, &sf_info);
	if (!sndfile)
	{
		printf ("%s \n", sf_strerror (sndfile));
	}

	
    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream(
              &stream,
              0,              /* no input channels */
              2,              /* stereo output */
              paFloat32,      /* 32 bit floating point output */
              sample_rate,
              buf_size,            /* frames per buffer */
              0,              /* number of buffers, if zero then use default minimum */
              patestCallback,
              &audio_unit_root );
    if( err != paNoError ) goto error;
    
    
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while uSing the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;

}

int pa_close()
{
	PaError err;

	sf_close (sndfile);

    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;
    Pa_Terminate();
	return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while uSing the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
