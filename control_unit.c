#include "control_unit.h"
#include "mathmat.h"
#include "tick.h"
#include "sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


void	cu_register_ins (char *short_name, char *name, control_unitT *(*new_method)(void))
{
	int		i;
	i = control_unit_inss++;
	sprintf(control_unit_ins[i].name, name);
	sprintf(control_unit_ins[i].short_name, short_name);
	control_unit_ins[i].new_method = new_method;
}

control_unitT		*cu_register (struct control_unitT	*cu)
{
	int	i;
	
	i = 0;
	while ((i < MAX_CONTROLS) && control_unit[i]) i++;
	
	if (i >= MAX_CONTROLS)
	{
		printf ("Too many units. Please pay for full version.\n");
		return (NULL);
	}

	control_unit[i] = cu;
	return (cu);
}


void	cu_add_input (control_unitT *cu, control_unitT *input)
{	
	int	i;

	if (cu->inputs >= MAX_INPUTS) return;
	i = cu->inputs++;
	cu->input[i] = input;	
}



void	cu_remove_input (control_unitT *a, control_unitT *b)
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

void	delete_cu_au_link (audio_unitT *a, control_unitT *c)
{
	int	i;
	
	i =0;
	while ((i < a->parameters) && (a->parameter[i].control != c)) i++;
	if (a->parameter[i].control == c)
	{
		a->parameter[i].control = NULL;
	}
}

void	cu_delete_link (control_unitT *a, control_unitT *b)
{
	int	i;

	/*
	** Check if it is an input link
	*/

	i = 0;
	while ((i < a->inputs) && (a->input[i] != b)) i++;
	if (a->input[i] == b)
	{
		cu_remove_input (a, b);
		return;
	}

	i = 0;
	while ((i < b->inputs) && (b->input[i] != a)) i++;
	if (b->input[i] == a)
	{
		cu_remove_input (b, a);
		return;
	}
}



control_unitT	*cu_generic_new (char *long_name, char *short_name)
{
	control_unitT	*cu;

	cu = (control_unitT *) malloc (sizeof (control_unitT));
	if (!cu)
	{
		printf ("Woopsie!\n");
		exit (-1);
	}

	cu->id = get_id();

	cu->inputs = 0;
	sprintf (cu->long_name, long_name);
	sprintf (cu->short_name, short_name);
	cu->value = 0.0f;
	cu->parameters = 0;
	cu->x = 0;
	cu->y = 0;
	
	return (cu);
}

void	cu_new_parameter (void *vp, control_unitT *cu, char *name, param_type pt, float min, float max, float def)
{
	int	p;

	p = cu->parameters++;
	sprintf (cu->parameter[p].name, name);
	cu->parameter[p].p_type = pt;
	cu->parameter[p].control = NULL;
	if (pt == TRIGGER)
	{
		cu->parameter[p].trigger_value = (int *) vp;
		*(cu->parameter[p].trigger_value) = (int) def;
	} else
	if (pt == INT)
	{
		cu->parameter[p].int_value = (int *) vp;
		cu->parameter[p].int_min   = (int) min;
		cu->parameter[p].int_max   = (int) max;
		*(cu->parameter[p].int_value) = (int) def;
	} else
	if (pt == FLOAT)
	{
		cu->parameter[p].float_value = (float *) vp;
		cu->parameter[p].float_min   = min;
		cu->parameter[p].float_max   = max;
		*(cu->parameter[p].float_value) = def;
	}
}

void	parameter_set (parameterT *p, float v)
{
	if (p->p_type == TRIGGER)
	{
		if (v > 0.0) *p->trigger_value = 1;
	} else
	if (p->p_type == FLOAT)
	{
		*p->float_value = v;
	} else
	if (p->p_type == INT)
	{
		*p->int_value = (int) v;
	}
}

void	cu_parameter_process (parameterT *p, int parameters)
{
	int		i;
	float	value, min, max;
	for (i=0; i<parameters; i++)
	{
		if (p[i].control)
		{
			p[i].control->proc(p[i].control);
			value = 0.5f * (p[i].control->value + 1.0f);
		
			min = 0.0f;
			max = 1.0f;
			
			if (p[i].p_type == TRIGGER)
			{
				min = 0.0f;
				max = 1.0f;
			} else
			if (p[i].p_type == FLOAT)
			{
				min = p[i].float_min;
				max = p[i].float_max;
			} else
			if (p[i].p_type == INT)
			{
				min = (float) p[i].int_min;
				max = (float) p[i].int_max;
			}

			value = value * (max - min) + min;

			if (value > max) value = max; else
			if (value < min) value = min;

			parameter_set (&p[i], value);
		//	printf ("%s %6.4f\n", p[i].name, value);
		}	
	}
}

/*
** SIN LFO
*/

typedef struct {
	float	freq;
	float	amp;
	float	phase;
	float	phase_offset;
} lfo_dataT;

void	cu_sin_lfo_proc (control_unitT *cu)
{
	lfo_dataT	*d;
	d = (lfo_dataT *) cu->data;
	cu->value = (float)(d->amp * Sin (d->phase + d->phase_offset));
	d->phase += d->freq * 0.01f;
}

control_unitT	*cu_sin_lfo_new (void)
{
	control_unitT	*cu;
	lfo_dataT		*d;

	cu = cu_generic_new ("sin oscillator", "sin");
	d = (lfo_dataT *) cu->data = malloc (sizeof (lfo_dataT));
	cu_new_parameter (&d->freq, cu, "freq", FLOAT, 0.0f, 50.0f, 0.1f);
	cu_new_parameter (&d->amp , cu, "amp" , FLOAT, 0.0f, 1.0f, 0.5f);
	cu_new_parameter (&d->phase_offset , cu, "phase" , FLOAT, 0.0f, 7.0f, 0.0f);
	d->phase = 0.0f;
	cu->proc = cu_sin_lfo_proc;
	return (cu);
}

/*
** SQR LFO
*/

void	cu_sqr_lfo_proc (control_unitT *cu)
{
	lfo_dataT	*d;
	d = (lfo_dataT *) cu->data;
	if ((float)(d->amp * Sin (d->phase + d->phase_offset)) < 0.0f)
	{
		cu->value = -1.0f;
	} else
	{
		cu->value = 1.0f;
	}
	d->phase += d->freq * 0.01f;
}

control_unitT	*cu_sqr_lfo_new (void)
{
	control_unitT	*cu;
	lfo_dataT		*d;

	cu = cu_generic_new ("sqr oscillator", "sqr");
	d = (lfo_dataT *) cu->data = malloc (sizeof (lfo_dataT));
	cu_new_parameter (&d->freq, cu, "freq", FLOAT, 0.0f, 50.0f, 0.1f);
	cu_new_parameter (&d->amp , cu, "amp" , FLOAT, 0.0f, 1.0f, 0.5f);
	cu_new_parameter (&d->phase_offset , cu, "phase" , FLOAT, 0.0f, 7.0f, 0.0f);
	d->phase = 0.0f;
	cu->proc = cu_sqr_lfo_proc;
	return (cu);
}

/*
** ADD
*/

void	cu_add_proc (control_unitT *cu)
{
	float	v;
	int		i;

	v = 0.0f;
	
	for (i=0; i<cu->inputs; i++)
	{
		cu->input[i]->proc (cu->input[i]);
		v += cu->input[i]->value;
	}	
	cu->value = v;
}

control_unitT	*cu_add_new (void)
{
	control_unitT	*cu;

	cu = cu_generic_new ("addition", "add");
	cu->proc = cu_add_proc;
	return (cu);
}


/*
** MULT
*/

void	cu_mul_proc (control_unitT *cu)
{
	float	v;
	int		i;

	v = 1.0f;
	
	for (i=0; i<cu->inputs; i++)
	{
		cu->input[i]->proc (cu->input[i]);
		v *= cu->input[i]->value;
	}	
	cu->value = v;
}

control_unitT	*cu_mul_new (void)
{
	control_unitT	*cu;

	cu = cu_generic_new ("multiplu", "mul");
	cu->proc = cu_mul_proc;
	return (cu);
}


/*
** EXP
*/

void	cu_exp_proc (control_unitT *cu)
{
	float	v;
	int		i;

	v = 0.0f;
	
	for (i=0; i<cu->inputs; i++)
	{
		cu->input[i]->proc (cu->input[i]);
		v += cu->input[i]->value;
	}	
	cu->value = exp(v);
}

control_unitT	*cu_exp_new (void)
{
	control_unitT	*cu;

	cu = cu_generic_new ("exponent", "exp");
	cu->proc = cu_exp_proc;
	return (cu);
}



/*
** ABS
*/

void	cu_abs_proc (control_unitT *cu)
{
	float	v;
	int		i;

	v = 0.0f;
	
	for (i=0; i<cu->inputs; i++)
	{
		cu->input[i]->proc (cu->input[i]);
		v += cu->input[i]->value;
	}	
	cu->value = fabs(v);
}

control_unitT	*cu_abs_new (void)
{
	control_unitT	*cu;

	cu = cu_generic_new ("absolute", "abs");
	cu->proc = cu_abs_proc;
	return (cu);
}

/*
** SLIDE
*/

typedef struct 
{
	float	ov;
	float	amount;
} slide_dataT;

void	cu_slide_proc (control_unitT *cu)
{
	float		v;
	int			i;
	slide_dataT	*d;

	d = (slide_dataT *) cu->data;
	v = 0.0f;
	d->ov = cu->value;
	for (i=0; i<cu->inputs; i++)
	{
		cu->input[i]->proc (cu->input[i]);
		v += cu->input[i]->value;
	}	
	cu->value = d->ov*(d->amount) + v*(1.0f-d->amount);
}

control_unitT	*cu_slide_new (void)
{
	control_unitT	*cu;	
	slide_dataT		*d;

	cu = cu_generic_new ("slide", "sli");
	d = (slide_dataT *) cu->data = malloc (sizeof (slide_dataT));
	cu_new_parameter (&d->amount, cu, "amount", FLOAT, 0.0f, 1.0f, 0.3f);
	cu->proc = cu_slide_proc;
	return (cu);
}


/*
** STEP
*/

typedef struct {
	int		freq;
	float	amp;
	float	step_up, step_down;
	int		direction, cnt;
} step_dataT;

void	cu_step_proc (control_unitT *cu)
{
	step_dataT	*d;
	d = (step_dataT *) cu->data;
	d->cnt ++;
	if (d->cnt < d->freq) return;
	
	d->cnt = 0;
	if (d->direction)
	{
		cu->value = cu->value + d->step_up;
		if (cu->value > d->amp) 
		{
			cu->value = d->amp;
			d->direction = 0;
		}
	} else
	{
		cu->value = cu->value - d->step_down;
		if (cu->value < -d->amp)
		{
			cu->value = -d->amp;
			d->direction = 1;
		}
	}
}

control_unitT	*cu_step_new (void)
{
	control_unitT	*cu;
	step_dataT		*d;

	cu = cu_generic_new ("step generator", "stp");
	d = (step_dataT *) cu->data = malloc (sizeof (step_dataT));
	cu_new_parameter (&d->freq, cu, "freq", INT, 0.0f, 50.0f, 1.0f);
	cu_new_parameter (&d->amp , cu, "amp" , FLOAT, 0.0f, 1.0f, 0.5f);
	cu_new_parameter (&d->step_up , cu, "up" , FLOAT, 0.0f, 1.0f, 0.1f);
	cu_new_parameter (&d->step_down , cu, "down" , FLOAT, 0.0f, 1.0f, 0.1f);
	d->direction = 1;		
	d->cnt = 0;
	cu->proc = cu_step_proc;
	return (cu);
}

void	cu_brownian_proc (control_unitT *cu)
{
	step_dataT	*d;
	d = (step_dataT *) cu->data;
	d->cnt ++;
	if (d->cnt < d->freq) return;
	
	d->cnt = 0;
	d->direction = rand() % 2;
	if (d->direction)
	{
		cu->value = cu->value + d->step_up;
		if (cu->value > d->amp) 
		{
			cu->value = d->amp;
		}
	} else
	{
		cu->value = cu->value - d->step_down;
		if (cu->value < -d->amp)
		{
			cu->value = -d->amp;
		}
	}
}

control_unitT	*cu_brownian_new (void)
{
	control_unitT	*cu;
	step_dataT		*d;

	cu = cu_generic_new ("brownian generator", "brn");
	d = (step_dataT *) cu->data = malloc (sizeof (step_dataT));
	cu_new_parameter (&d->freq, cu, "freq", INT, 0.0f, 50.0f, 1.0f);
	cu_new_parameter (&d->amp , cu, "amp" , FLOAT, 0.0f, 1.0f, 0.5f);
	cu_new_parameter (&d->step_up , cu, "up" , FLOAT, 0.0f, 1.0f, 0.1f);
	cu_new_parameter (&d->step_down , cu, "down" , FLOAT, 0.0f, 1.0f, 0.1f);
	d->direction = 1;		
	d->cnt = 0;
	cu->proc = cu_brownian_proc;
	return (cu);
}

void	cu_num_proc (control_unitT *cu)
{
	float	*d;
	d = (float *) cu->data;
	cu->value = *d;
}

control_unitT	*cu_num_new (void)
{
	control_unitT	*cu;
	float			*d;

	cu = cu_generic_new ("number", "num");
	d = (float *) cu->data = malloc (sizeof (float));
	cu_new_parameter (d, cu, "value", FLOAT, -1.0f, 1.0f, 0.1f);
	cu->proc = cu_num_proc;
	return (cu);
}


void	init_cu (void)
{
	int		i;

	for (i=0; i<MAX_CONTROLS; i++)
	{
		control_unit[i] = NULL;
	}
	cu_register_ins ("num", "number", cu_num_new);
	cu_register_ins ("sin", "sin lfo", cu_sin_lfo_new);
	cu_register_ins ("sqr", "sqr lfo", cu_sqr_lfo_new);
	cu_register_ins ("stp", "step gen", cu_step_new);
	cu_register_ins ("brn", "brownian gen", cu_brownian_new);
	cu_register_ins ("sli", "slide", cu_slide_new);
	cu_register_ins ("add", "add", cu_add_new);
	cu_register_ins ("mul", "multiply", cu_mul_new);	
	cu_register_ins ("exp", "exponent", cu_exp_new);
	cu_register_ins ("abs", "absolute", cu_abs_new);
}
