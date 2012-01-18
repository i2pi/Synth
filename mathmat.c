
double Sin(double phase)
{
	__asm
	{
		fld		phase
		fSin	
		fstp	phase
	}
	
	return phase;
}

double Cos(double phase)
{
	__asm
	{
		fld		phase
		fCos	
		fstp	phase
	}
	
	return phase;
}

double Sqrt(double phase)
{
	__asm
	{
		fld		phase
		fSqrt	
		fstp	phase
	}
	
	return phase;
}
