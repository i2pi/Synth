#ifndef __I2PIVERB__
#define __I2PIVERB__

typedef struct
{
	float	gforward;
	float	gback;
} i2tapT;

typedef struct
{
	float	*buf;
	size_t	buf_size;
	i2tapT	*tap;
	int		taps;
	long	offset;
	float	stretch;

	int		idx;
} i2delay_lineT;


i2delay_lineT	*i2new_delay (size_t buf_size, int taps, long offset, 
								float stretch);
void	i2shape_taps (i2delay_lineT	*d, float fa, float fb,
						float ba, float bb);
float	i2convolve (i2delay_lineT *d, float x);
void	i2circulate (int n, float *in, float *a, float *out);
float	i2reverb (float x, int n, i2delay_lineT **d, float *fm, 
					float *in, float *out);


#endif /* __I2PIVERB__ */
