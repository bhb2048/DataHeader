//*****************************************************************************
//	Copywrite (C) 1992-2002 Barry H. Branham.  All rights reserved.
//*****************************************************************************
//
//	gtchan.c -- Get short integers between 0 and max.
//
//	This routine parses and assigns short integers from an input
//	string of the form "x,y,z-n" where x,y,z,n represent up to
//	3 digit integers.  For each ascii integer n, assigns n-1 to buf.
//
//	Returns number of integers, -1 if error, 0 if exit.
//
//	gtchan_0() returns actual #'s entered, gtchan() subtracts 1 from each #
//
//	sep 84
//	Version 1.3
//	Modified:
//	15-feb-85	optimized
//	07-nov-87  bhb	added max param.
//	11-nov-87  bhb  added input string param.
//	17-feb-94  bhb	allow a-b case where a > b
//	17-feb-94  bhb	added get_ndiff()
//	03-mar-01  bhb	changed gtchan() buf from short to unsigned short
//	26-sep-02  bhb	changed to gtchan.cc, added 'const' to first string arg
//	
//
#include <stdio.h>
#include <malloc.h>
#include <Data/DataHeader.h>

//
//	assumes input is '1' based, returns '0' based array
//	
int
gtchan(	const char	*string, 		// input string
	unsigned short *buf, 			// integer buf
	int	max)
{
    int i, n;
    
    n = gtchan_0( string, buf, max);
    for ( i=0; i<n; i++)
		buf[i]--;
    return n;
}

int
gtchan_0( const char    *string, 	// input string
	    unsigned short *buf,		// integer buf
	    int	    max)				// max integer val
{
    int		i;				// string pointer
    register int j;			// buf pointer
    register int first;		// integers
    register int second;

    i = j = 0;				// init pointers

    if ( string[ 0] == EOS) return(0);
    while ( 1)		/* process string until return	*/
    {
		first = getnum( string, &i);	/* get first integer	*/
		if ((first < 0) || (first > max)) return( -1);
		switch (string[i++])		/* get next char/chan	*/
		{
			case	',':
					buf[j++] = first;
					break;
			case	'-':
					second = getnum( string, &i);
					if ((second < 0) || (second > max)) return( -1);
					else {
					    if ( first <= second)
						while (first <= second) buf[j++] = first++;
					    else
						while ( first >= second) buf[j++] = first--;
					    if (string[i] == ',');
					    else if (string[i] == EOS) return(j);
					    else return( -1);
					    i++;
					}
					break;
			case	EOS:
					buf[j++] = first;
					return(j);	/* number of ints	*/
					break;
			default:
					return( -1);
					break;
		}
    }
}

int getnum(	const char *string, int *i)
{
	register int num = 0;
	bool neg = false;
	
	if ( string[*i] == '-')
	{
		neg = true;
		(*i)++;
	}
	while ((string[*i] >= '0') && (string[*i] <= '9')) {
		num = num * 10 + (string[*i] - '0');
		(*i)++;
	}
	if ( neg)
		num = -num;
	return num;
}

int
get_ndiff( const unsigned short *buf, int n)
{
    int i, ndiff;
    short *found, max;
    
    for ( i=max=0; i<n; i++)
	if ( buf[i] > max) max = buf[i];
    found = (short *)calloc( max+1, sizeof( short));
    for ( i=ndiff=0; i<n; i++)
	if ( !found[buf[i]]) {
	    found[buf[i]]++;
	    ndiff++;
	}
    free( found);
    return ndiff;
}
