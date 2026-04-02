//*****************************************************************************
//	Copywrite (C) 1992 Barry H. Branham.  All rights reserved.
//*****************************************************************************
// define DEBUG
//
//	numstr.cc -- function to take a linear array of numbers and return a
//		string of the form: "6,9,13-24,22" etc.
//		Returns pointer to string.
//
//	1-dec-84
//	Version 2.2
//	Modified:
//	04-jan-88  bhb	updated for VAX C, sprintf not transportable from
//			DECUS C.
//	07-jan-93  bhb	add numstr_1() which adds one to every number in string
//	16-feb-94  bhb	if only two in sequence use a,b instead of a-b
//	07-mar-01  bhb	changed pn1 arg from short to short unsigned int
//	25-jun-02  bhb	add len <= 0 check in numstr_1
//	27-sep-02  bhb	changed to .cc, added const args in numstr
//	18-may-05  bhb	moved getnum() here from getnum.cc, no more iolib, moved to DataHeader
//	
#include <stdio.h>
#include <string.h>
#include <malloc.h>

char *
numstr( char *pstr,						// pointer to output string
		const short unsigned int *pn1,	// pointer to start of number array
		const int len)					// length of number array
{
	char *ps;
	const short unsigned int *pn2, *end;
	short int diff;

	if ( len <= 0)
	{
		*pstr = '\0';
		return pstr;
	}
	sprintf( pstr, "%d", *pn1);					/* first number	*/
	ps = pstr + strlen( pstr);
	end = pn1 + len;
	pn2 = pn1 + 1;
	while ( pn2 < end)
	{
		diff = *pn2++ - *pn1++;
		if ( diff > 1 || diff < -1)
		{
			sprintf( ps, ",%d", *pn1);
		}
		else if ( diff == 1)
		{
			if ( *pn2 - *pn1 != 1)				/* sequence of two, do ','	*/
			{
				sprintf( ps, ",%d", *pn1);
			}
			else								/* find end of up sequence	*/
			{
				while ( pn2 < end && (*pn2 - *pn1) == 1) {++pn1; ++pn2;}
				sprintf( ps, "-%d", *pn1);
			}
		}
		else if ( diff == -1)					/* find end of down sequence	*/
		{
			if ( *pn2 - *pn1 != -1)				/* sequence of two, do ','	*/
			{
				sprintf( ps, ",%d", *pn1);
			}
			else								/* find end of down sequence	*/
			{
				while ( pn2 < end && (*pn2 - *pn1) == -1) {++pn1; ++pn2;}
				sprintf( ps, "-%d", *pn1);
			}
		}
		else
		{
			sprintf( ps, ",%d", *pn1);			/* diff = 0	*/
		}
		ps = pstr + strlen( pstr);
	}
	return( pstr);
}

char *
numstr_1(   char *pstr,						// pointer to output string
			const short unsigned int *pn1,	// pointer to start of number array
			const int len)					// length of number array
{
	int i;
	short unsigned int*pn2;

	if ( len <= 0)
	{
		*pstr = '\0';
		return pstr;
	}
	pn2 = (short unsigned int *)malloc( len * sizeof( short));
	for ( i=0; i<len; i++) pn2[i] = pn1[i]+1;
	numstr( pstr, pn2, len);
	free( pn2);
	return( pstr);
}
