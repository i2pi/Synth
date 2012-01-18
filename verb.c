#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "verb.h"
#include "sound.h"

int	tapnum[] =
{
1, 2, 3, 7, 23, 31, 41, 53, 71,
97, 127, 163, 211, 269, 347, 439, 557, 701,
881, 1103, 1381, 1733, 2179, 2729, 3413, 4271,
5347, 6689, 8363, 10457, 13093, 16369, 20477
};

i2delay_lineT	*i2new_delay (size_t buf_size, int taps, long offset,			
								float stretch)
{
	/*
	** Creates a new delay line
	*/

	i2delay_lineT	*d;
	int				t;

	d = (i2delay_lineT *) malloc (sizeof (i2delay_lineT));
	if (!d) return (NULL);

	d->buf = (float *) calloc (sizeof (float), buf_size);
	if (!d->buf)
	{
		free (d);
		return (NULL);
	}
	d->buf_size = buf_size;

	d->tap = (i2tapT *) malloc (sizeof (i2tapT) * taps);
	if (!d->tap)
	{
		free (d->buf);
		free (d);
		return (NULL);
	}
	d->taps = taps;

	for (t=0; t<taps; t++)
	{
		d->tap[t].gforward = 0;
		d->tap[t].gback = 0;
	}

	d->offset = offset;
	d->stretch = stretch;

	return (d);	
}

void	i2shape_taps (i2delay_lineT	*d, 
						float fa, float fb,
						float ba, float bb)
{
	/*
	** Shapes the delay line taps according to the function
	** f(x) = e^(-a.x) * x^(b) * noise
	** Where x = tapnum / taps
	**
	** Does not check that f(x) exceeds unity. Stability is up to you.
	*/

	int		t;
	float	x;
	float	noise;

	for (t=0; t<d->taps; t++)
	{
		x = t / (float) d->taps;

		noise = (rand() / (float)RAND_MAX) - 0.5f;
		d->tap[t].gforward 	= (float)(exp(-fa*x) * pow(x,fb) * noise);
		noise = (rand() / (float) RAND_MAX) - 0.5f;
		d->tap[t].gback		= (float)(exp(-ba*x) * pow(x,bb) * noise);
	}
}

float	i2convolve (i2delay_lineT *d, float x)
{
	/*
	** Takes an incoming sample in x, and convolves against our
	** IIR delay line
	*/
	
	int		t;
	float	fsum, bsum;

	fsum = 0.0;
	bsum = 0.0;

	for (t=0; t<d->taps; t++)
	{
		fsum += d->buf[(d->idx - (int)(d->stretch*tapnum[t] - d->offset)) 
					% d->buf_size] 
				* d->tap[t].gforward;
		bsum += d->buf[(d->idx - (int)(d->stretch*tapnum[t] - d->offset))
				% d->buf_size] 
				* d->tap[t].gback;
	}

	d->buf[(++d->idx) % d->buf_size] = bsum + x;
	return (bsum + fsum);
}

void	i2circulate (int n, float *in, float *a, float *out)
{
	int	i, j;

	for (i=0; i<n; i++)
	{
		out[i] = 0.0;

		for (j=0; j<n; j++)
		{
			out[i] += in[j] * a[(j+i)%n];
		}
	}
}

float	i2reverb (float x, int n, i2delay_lineT **d, float *fm, 
					float *in, float *out)
{

	int		i;
	float	ret;

	for (i=0; i<n; i++)
	{
		in[i] = i2convolve (d[i], x + out[i]);
	}

	i2circulate (n, in, fm, out);

	ret = 0.0;	
	for (i=0; i<n; i++)
	{
		ret += out[i];	
	}

	return (ret);
}

