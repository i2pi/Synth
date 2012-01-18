#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sound.h"
#include "tick.h"
#include "control_unit.h"

typedef struct
{
	int		id;
	void	*vp;
} fmapT;

fmapT	cu_map[MAX_CONTROLS];
int		cu_maps = 0;
fmapT	au_map[MAX_UNITS];
int		au_maps = 0;

parameterT	*pmap[MAX_UNITS*10];
int			pmaps=0;

FILE	*fp;

void	save_counters (void)
{
	int		c, i;

	for (c=0; c<tick_data.counters; c++)
	{
		fprintf (fp, "t%d: BD%d V%d LV%d ", c, tick_data.counter[c].beat_div,
					tick_data.counter[c].value, tick_data.counter[c].loop_value);
		for (i=0; i<tick_data.counter[c].loop_value; i++)
		{
			if (tick_data.counter[c].hit[i])
				fprintf (fp, "1");
			else
				fprintf (fp, "0");	
		}
		fprintf (fp, " >a%d\n", tick_data.counter[c].au->id);
	}	
}

void	load_counter (char *str)
{
	char			*cp, *ep;
	int				i;
	static int		c = 0;

	cp = strstr (str, "BD"); cp += 2;
	ep = strchr (cp, ' '); *ep = '\0';
	tick_data.counter[c].beat_div = atoi (cp);
	cp = ep+1;

	cp = strstr (cp, "V"); 
	cp += 1;
	ep = strchr (cp, ' '); *ep = '\0';
	tick_data.counter[c].value = atoi (cp);
	cp = ep+1;

	cp = strstr (cp, "LV"); cp += 2;
	ep = strchr (cp, ' '); *ep = '\0';
	tick_data.counter[c].loop_value = atoi (cp);
	cp = ep+1;
	
	for (i=0; i<tick_data.counter[c].loop_value; i++)
	{
		if (cp[i] == '1')
			tick_data.counter[c].hit[i] = 1; 
		else
			tick_data.counter[c].hit[i] = 0; 
	}

	cp = strstr (cp, ">"); cp += 1;
	
	
	c++;
	tick_data.counters = c;
}

void	save_parameters (int parameters, parameterT *parameter)
{
	int		i;

	for (i=0; i<parameters; i++)
	{
		float	v;

		fprintf (fp, "[%s]", parameter[i].name);
		if (!parameter[i].control)
		{
			if (parameter[i].p_type == TRIGGER)
				v = (float) (*parameter[i].trigger_value); else
			if (parameter[i].p_type == INT)
				v = (float) (*parameter[i].int_value); else
			if (parameter[i].p_type == FLOAT)
				v = (float) *parameter[i].float_value;	
			fprintf (fp, "%+08.3f ", v);
		} else
		{
			fprintf (fp, "c%d ", parameter[i].control->id);
		} 
	}	
}

void	load_parameter (char *str, int parameters, parameterT *parameter)
{
	int		i;
	char	name[256], *cp, *ep;
	float	v;

	cp = strchr (str, '[');
	cp++;
	ep = strchr (str, ']');
	*ep = '\0';
	strcpy (name, cp);
	cp = ++ep;
	
	i = 0;
	while ((i < parameters) && strcmp (parameter[i].name, name)) i++;
	if (i >= parameters)
	{
		printf ("Couldnt set parameter '%s' - nothing goes by that name\n", name);
		return;
	}

	if (cp[0] != 'c')
	{
		v = (float) atof (cp);
		if (parameter[i].p_type == TRIGGER)
			*parameter[i].trigger_value = (int) v; else
		if (parameter[i].p_type == INT)
			*parameter[i].int_value = (int) v; else
		if (parameter[i].p_type == FLOAT)
			*parameter[i].float_value = v;	
	} else
	{
		v = 0.0f;	
		pmap[pmaps++] = &parameter[i];		
	}

	
}

void	load_parameters (char *str, int parameters, parameterT *parameter)
{
	char	*cp, *ep;

	cp = str;

	while (cp && (cp = strchr (cp, '[')))
	{
		ep = strchr (cp+1, '[');
		if (ep) *ep = '\0';
		load_parameter (cp, parameters, parameter);
		if (ep) *ep = '[';
		cp = ep;
	}
}

void	save_au (audio_unitT *au)
{
	int		i;

	fprintf (fp, "a%d: NM[%s] XY%d,%d ", au->id, au->short_name, au->x, au->y);
	save_parameters (au->parameters, au->parameter);
	if (au->inputs) fprintf (fp, ">");
	for (i=0; i<au->inputs; i++)
	{
		fprintf (fp, "Ia%d ", au->input[i]->id);
	}
	fprintf (fp, "\n");
}

void	load_au (char *str)
{
	int			id, x, y, i;
	char		short_name[32];
	char		*cp, *ep;
	audio_unitT	*au;

	cp = strstr (str, "a"); cp += 1;
	ep = strchr (cp, ' '); *ep = '\0';
	id = atoi (cp);
	cp = ep+1;

	cp = strstr (cp, "NM["); cp += 3;
	ep = strchr (cp, ']'); *ep = '\0';
	strcpy (short_name, cp);
	cp = ep+1;

	cp = strstr (cp, "XY"); cp += 2;
	ep = strchr (cp, ','); *ep = '\0';
	x = atoi (cp);
	cp = ep+1;
	ep = strchr (cp, ' '); *ep = '\0';
	y = atoi (cp);
	cp = ep + 1;

	if (!strcmp (short_name, "dack"))
	{
		audio_unit_root.dac.id = id;
		audio_unit_root.dac.x = x;
		audio_unit_root.dac.y = y;
		sprintf (audio_unit_root.dac.short_name, "dack");
		audio_unit_root.dac.parameters = 0;
		audio_unit_root.dac.inputs = 0;
		audio_unit_root.dac.id = get_id ();	
		au_register (&audio_unit_root.dac);
		i = au_maps++;
		au_map[i].id = id;
		au_map[i].vp = (void *) &audio_unit_root.dac;
		if (au_map[i].id > global_id) global_id = au_map[i].id;
	} else
	{
		i = 0;
		while ((i < audio_unit_inss) && strcmp(audio_unit_ins[i].short_name, short_name)) i++;
		if (i >= audio_unit_inss)
		{
			printf ("Failed to find audio unit by the name of '%s'\n", short_name);
		} else
		{
			au = audio_unit_ins[i].new_method ();
			au->id = id;
			au->x = x;
			au->y = y;
			au_register (au);
			load_parameters (cp, au->parameters, au->parameter);
			i = au_maps++;
			au_map[i].id = id;
			au_map[i].vp = (void *) au;
			if (au_map[i].id > global_id) global_id = au_map[i].id;
		}
	}

}

void	save_cu (control_unitT *cu)
{
	int		i;

	fprintf (fp, "c%d: NM[%s] XY%d,%d V%+6.4f ", cu->id, cu->short_name, cu->x, cu->y, cu->value);
	save_parameters (cu->parameters, cu->parameter);
	if (cu->inputs) fprintf (fp, ">");
	for (i=0; i<cu->inputs; i++)
	{
		fprintf (fp, "Ic%d ", cu->input[i]->id);
	}
	fprintf (fp, "\n");
}

void	load_cu (char *str)
{
	int				id, x, y, i;
	float			v;
	char			short_name[32];
	char			*cp, *ep;
	control_unitT	*cu;

	if (strstr (str, "21"))
	{
		printf ("here");
	}
	cp = strstr (str, "c"); 
	cp += 1;
	ep = strchr (cp, ':'); 
	*ep = '\0';
	id = atoi (cp);
	cp = ep+1;

	cp = strstr (cp, "NM["); cp += 3;
	ep = strchr (cp, ']'); *ep = '\0';
	strcpy (short_name, cp);
	cp = ep+1;

	cp = strstr (cp, "XY"); cp += 2;
	ep = strchr (cp, ','); *ep = '\0';
	x = atoi (cp);
	cp = ep+1;
	ep = strchr (cp, ' '); *ep = '\0';
	y = atoi (cp);
	cp = ep + 1;

	cp = strstr (cp, "V"); cp += 1;
	ep = strchr (cp, ' '); *ep = '\0';
	v = (float) atof (cp);
	cp = ep+1;

	i = 0;
	while ((i < control_unit_inss) && strcmp(control_unit_ins[i].short_name, short_name)) i++;
	if (i >= control_unit_inss)
	{
		printf ("Failed to find control unit by the name of '%s'\n", short_name);
	} else
	{
		cu = control_unit_ins[i].new_method ();
		cu->id = id;
		cu->x = x;
		cu->y = y;
		cu_register (cu);
		load_parameters (cp, cu->parameters, cu->parameter);
		i = cu_maps++;
		cu_map[i].id = id;
		cu_map[i].vp = (void *) cu;
		if (cu_map[i].id > global_id) global_id = cu_map[i].id;
	}

}

void	save (void)
{
	int	i;

	fp = fopen ("session.i2s", "w");
	if (!fp)
	{
		printf ("Failed to open session.i2s for writing\n");
		return;
	}

	fprintf (fp, "SAMPLE_RATE: %12.2f\n", tick_data.samples_per_second);
	fprintf (fp, "BPM: %8.3f\n", tick_data.bpm);
	
	for (i=0; i<MAX_UNITS; i++) if (audio_unit[i]) save_au (audio_unit[i]);
	for (i=0; i<MAX_CONTROLS; i++) if (control_unit[i]) save_cu (control_unit[i]);
	save_counters ();
	fprintf (fp, "EOF\n\n");
	fclose (fp);
}

void	*get_map_pointer (fmapT *map, int maps, int id)
{
	int	i;
	
	i = 0;
	while ((i < maps) && (map[i].id != id)) i++;
	if (map[i].id == id)
	{
		return (map[i].vp);
	}
	return (NULL);
}

void	load (void)
{	
	char	str[2048], *cp;
	int		i, pi;

	fp = fopen ("session.i2s", "r");
	if (!fp)
	{
		printf ("Failed to open session.i2s for reading\n");
		exit (-1);
	}
	
	for (i=0; i<MAX_UNITS; i++)
	{
		audio_unit[i] = NULL;
	}
	global_id = 0;

	/*
	** SAMPLE RATE
	*/
	fgets (str, 2040, fp);
	cp = strchr (str, ':');
	tick_data.samples_per_second = atof (cp);
	/*
	** BPM
	*/
	fgets (str, 2040, fp);
	cp = strchr (str, ':');
	tick_data.bpm = atof (cp);
	init_tick (tick_data.samples_per_second, tick_data.bpm);

	while (!feof (fp))
	{
		fgets (str, 2040, fp);
		switch (str[0])
		{
			case 't': load_counter (str); break;
			case 'a': load_au (str); break;
			case 'c': load_cu (str); break;
		}
	}

	/*
	** 2nd pass to link inputs
	*/
	fseek (fp, 0, SEEK_SET);
	pi = 0;
	while (!feof (fp))	
	{
		char	*ep;
		int		id, id2;

		fgets (str, 2040, fp);
		if ((cp = strchr (str, '>')))
		{
			ep = strchr (str, ':');
			*ep = '\0';
			id = atoi (str+1);
			*ep = ':';
			switch (str[0])
			{
				case 't': 
					tick_data.counter[id].au = (audio_unitT *) 
						get_map_pointer (au_map, au_maps, atoi (cp+2)); 
					tick_data.counter[id].p = 0; // CHEAT IS WRONG
				break;
				case 'a':
					cp += 1;
					while (cp)
					{
						ep = strchr (cp, ' ');
						if (ep) *ep = '\0';
						id2 = atoi (cp+2);
						cp = strstr (ep+1, "Ia");
						au_add_input ((audio_unitT *)get_map_pointer (au_map, au_maps, id),
									  (audio_unitT *)get_map_pointer (au_map, au_maps, id2));
					}
				break;
				case 'c':
					cp += 1;
					while (cp)
					{
						ep = strchr (cp, ' ');
						if (ep) *ep = '\0';
						id2 = atoi (cp+2);
						cp = strstr (ep+1, "Ic");
						cu_add_input ((control_unitT *)get_map_pointer (cu_map, cu_maps, id),
									  (control_unitT *)get_map_pointer (cu_map, cu_maps, id2));
					}
				break;
			}
		}
		if ((cp = strstr (str, "]c")))
		{
			while (cp)
			{
				ep = strchr (cp, ' ');
				*ep = '\0';
				id = atoi (cp+2);
				pmap[pi++]->control = (control_unitT *) get_map_pointer (cu_map, cu_maps, id);
				cp = strstr (ep+1, "]c");
			}			
		}
	}


	fclose (fp);
}