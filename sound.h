#ifndef __SOUND_H__
#define __SOUND_H__

#include "portaudio.h"

#define MAX_INPUTS	32
#define BUF_SIZE	4096
#define MAX_UNITS	1024
#define MAX_PARAMETERS	32
#define SAMPLE_RATE	44100

typedef struct
{
	float	*l_buffer;
	float	*r_buffer;
	double	gen_time;
} buffer_dataT;


typedef enum
{	
	TRIGGER,
	FLOAT,
	INT
} param_type; 

typedef struct
{
	char			name[128];
	param_type		p_type;	
	int				*trigger_value;
	int				*int_value;
	float			*float_value;
	int				int_min, int_max;
	float			float_min, float_max;
	struct	control_unitT	*control;
} parameterT;


typedef struct audio_unitT
{
	struct audio_unitT	*input[MAX_INPUTS];
	int					inputs;
	void				(*proc)(struct audio_unitT *au);
	void				*data;
	buffer_dataT		out_buf;
	parameterT			parameter[MAX_PARAMETERS];
	int					parameters;
	char				short_name[5];
	char				long_name[128];	
	float				status;
	int					x,y;
	int					id;
} audio_unitT;

typedef struct
{
	buffer_dataT		buf;
	audio_unitT			dac;
} SynthMainData;

typedef struct
{
	char			name[256];
	char			short_name[16];
	audio_unitT   *(*new_method)(void);
} audio_unit_insT;

audio_unitT		*audio_unit[MAX_UNITS];
audio_unit_insT	audio_unit_ins[MAX_UNITS];
int				audio_unit_inss;

int			DISK_WRITE;
int			global_id;

extern PortAudioStream *stream;
SynthMainData audio_unit_root;


int		get_id (void);
void		au_register_ins (char *short_name, char *name, audio_unitT *(*new)(void));
audio_unitT		*au_register (struct audio_unitT	*au);
int		au_proc_time_check (struct audio_unitT *au);
void	add_root_unit (audio_unitT *au);
void	init_buffer_data (int buf_size, buffer_dataT *buf);
void	clear_buffer_data (buffer_dataT *buf);
void	au_new_parameter (void *vp, audio_unitT *au, char *name, param_type pt, float min, float max, float def);
void	au_delete_link (audio_unitT *a, audio_unitT *b);
void	au_add_input (audio_unitT *au, audio_unitT *input);
void	au_merge_inputs_to_output (struct audio_unitT *au);
void	au_merge_inputs_to_buf (struct audio_unitT *au, buffer_dataT *buf);
void	init_au (void);

int pa_init(int buf_size, int sample_rate);
int pa_close();


#endif 