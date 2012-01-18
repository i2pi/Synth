#ifndef _CONTROL_UNIT_H_
#define _CONTROL_UNIT_H_

#include "sound.h"

#define MAX_CONTROLS	2048

typedef struct control_unitT
{
	struct control_unitT	*input[MAX_INPUTS];
	int						inputs;
	char					long_name[128];
	char					short_name[5];
	float					value;
	void					(*proc)(struct control_unitT *cu);
	parameterT				parameter[MAX_PARAMETERS];
	int						parameters;
	void					*data;
	int						x, y;
	int						id;
} control_unitT;


typedef struct
{
	char				name[128];	
	char				short_name[16];
	control_unitT		*(*new_method)(void);
} control_unit_insT;


control_unitT		*control_unit[MAX_CONTROLS];
control_unit_insT	control_unit_ins[MAX_CONTROLS];
int					control_unit_inss;

void			init_cu (void);

void			parameter_set (parameterT *p, float v);
void			cu_parameter_process (parameterT *p, int parameters);
control_unitT	*cu_register (struct control_unitT	*cu);
control_unitT	*cu_sin_lfo_new (void);
void			cu_add_input (control_unitT *cu, control_unitT *input);
void			cu_remove_input (control_unitT *a, control_unitT *b);
void			cu_delete_link (control_unitT *a, control_unitT *b);
void			delete_cu_au_link (audio_unitT *a, control_unitT *c);


#endif