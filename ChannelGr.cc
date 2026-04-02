//
//	ChannelGr.cc - routines for ChannelGr class
//
//	01-jun-92	jwp	changed to using data object. ver 1.0->2.0
//	Version: 1.3
//	Modified:
//	03-aug-92  jwp	get_sg(): mostly moved from header object.
//	08-dec-92  bhb	revised sg_setup(), combined sgsetup.c with sampgroups.c
//	22-feb-94  bhb	use CSample_gr class, display groups, chan names are members
//	20-sep-02  bhb	added copy operator
//	20-may-05  bhb	added numstr(), numstr_1()
//	23-may-05  bhb	added gtchan(), gtchan_0(), getnum(), get_ndiff()
//	23-feb-06  bhb	added checkSuperSet()
//	20-mar-06  bhb	added sameChan()
//	14-sep-06  bhb	added checkSubSet()
//	09-oct-06  bhb	added isContiguous(), changed other 'check' funcs to 'is'
//	11-oct-06  bhb	use strtol in gtchan_0() instead of getnum(), removed getnum()
//	04-jun-08  bhb	changed _chan to vector<unsigned short>
//	11-jun-08  bhb	speed up isSuperSet(), add _isSequential, speed up getIndex()
//
#include "ChannelGr.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

using namespace std;

//*****************************************************************************
//
//  ChannelGr class member functions
//
//*****************************************************************************
ChannelGr::ChannelGr( int nch)
{
	setName("chan group");
	setMode(-1);
	_chan.reserve(nch);
	for (int i=0; i<nch; i++)
		addChan( i);
	_isSequential = true;
}

ChannelGr::ChannelGr( const ChannelGr &cg)
{
	_name = cg.name();		// get rid of 'const' warning
	_mode = cg.mode();
	_chan = cg._chan;
	_isSequential = cg._isSequential;
}

ChannelGr::~ChannelGr()
{
	// Empty
}

//
//  Get index of ch in chan[], return -1 if not found.
//
int
ChannelGr::getIndex( int ch)
{
	int indx = ch - chan(0);

	if ( _isSequential && (chan(indx) == ch))
		return indx;
	for ( int i = 0; i < nChan(); i++)
		if ( _chan[i] == ch)
			return i;
	return -1;
}

//
//	Construct a string, given an array of increasing positive numbers, such that
//	consecutive numbers are written as 'start-end' and non-consecutive are
//	written as 'num1,num2,num3', etc.
//
char *
ChannelGr::numstr( char *pstr,			// pointer to output string
		const unsigned short *pn1,		// pointer to start of number array
		const int len)					// length of number array
{
	char *ps;							// current position in pstr
	const unsigned short *pn2, *end;
	short int diff;

	if ( len <= 0)
	{
		*pstr = '\0';
		return pstr;
	}
	sprintf( pstr, "%d", *pn1);					// first number
	ps = pstr + strlen( pstr);
	end = pn1 + len;    // len - 1
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
			if ( pn2 == end || *pn2 - *pn1 != 1)	// sequence of two, do ','
			{
				sprintf( ps, ",%d", *pn1);
			}
			else									// find end of up sequence
			{
				while ( pn2 < end && (*pn2 - *pn1) == 1)
				{	++pn1; ++pn2;	}
				sprintf( ps, "-%d", *pn1);
			}
		}
		else if ( diff == -1)						// find end of down sequence
		{
			if ( pn2 == end || *pn2 - *pn1 != -1)	// sequence of two, do ','
			{
				sprintf( ps, ",%d", *pn1);
			}
			else									// find end of down sequence
			{
				while ( pn2 < end && (*pn2 - *pn1) == -1)
				{	++pn1; ++pn2;	}
				sprintf( ps, "-%d", *pn1);
			}
		}
		else
		{
			sprintf( ps, ",%d", *pn1);				// diff = 0
		}
		ps = pstr + strlen( pstr);
	}
	return( pstr);
}

//
//	Get chan string from _chan
//
char *
ChannelGr::numstr( char *pstr)
{
	char *ps;
	vector<unsigned short>::const_iterator pn1, pn2, end;
	short int diff;

	if ( _chan.size() == 0)
	{
		*pstr = '\0';
		return pstr;
	}
	end = _chan.end();
	pn1 = _chan.begin();
	pn2 = pn1 + 1;
	sprintf( pstr, "%d", *pn1);						// first number
	ps = pstr + strlen( pstr);
	while ( pn2 !=end)
	{
		diff = *pn2++ - *pn1++;
		if ( diff > 1 || diff < -1)
		{
			sprintf( ps, ",%d", *pn1);
		}
		else if ( diff == 1)
		{
			if ( pn2 == end || *pn2 - *pn1 != 1)	// sequence of two, do ','
			{
				sprintf( ps, ",%d", *pn1);
			}
			else									// find end of up sequence
			{
				while ( pn2 != end && (*pn2 - *pn1) == 1) {++pn1; ++pn2;}
				sprintf( ps, "-%d", *pn1);
			}
		}
		else if ( diff == -1)						// find end of down sequence
		{
			if ( pn2 == end || *pn2 - *pn1 != -1)	// sequence of two, do ','
			{
				sprintf( ps, ",%d", *pn1);
			}
			else									// find end of down sequence
			{
				while ( pn2 != end && (*pn2 - *pn1) == -1) {++pn1; ++pn2;}
				sprintf( ps, "-%d", *pn1);
			}
		}
		else
		{
			sprintf( ps, ",%d", *pn1);				// diff = 0
		}
		ps = pstr + strlen( pstr);
	}
	return( pstr);
}

//
//	Output 1-based numstr() string given 0-based input array
//
char *
ChannelGr::numstr_1(   char *pstr,			// pointer to output string
			const unsigned short *pn1,		// pointer to start of number array
			const int len)					// length of number array
{
	int i;
	unsigned short *pn2;

	if ( len <= 0)
	{
		*pstr = '\0';
		return pstr;
	}
	pn2 = new unsigned short[len];
	for ( i=0; i<len; i++) pn2[i] = pn1[i]+1;
	numstr( pstr, pn2, len);
	delete [] pn2;
	return pstr;
}

//
//	Output 1-based numstr() string from _chan
//
char *
ChannelGr::numstr_1( char *pstr)
{
	unsigned short *pn2;
	size_t len = _chan.size();

	if ( len == 0)
	{
		*pstr = '\0';
		return pstr;
	}
	pn2 = new unsigned short[len];
	for ( size_t i=0; i<len; i++) pn2[i] = _chan[i]+1;
	numstr( pstr, pn2, len);
	delete [] pn2;
	return pstr;
}

//
//	assumes input is '1' based, returns '0' based array
//
int
ChannelGr::gtchan(	const char	*str, 			// input string
					unsigned short *buf, 		// integer buf
					int max)					// max integer val
{
	int i, n;

	n = gtchan_0( str, buf, max);
	for ( i=0; i<n; i++)
		buf[i]--;
	return n;
}

int
ChannelGr::gtchan(	const char	*str)
{
	int i, n;

	n = gtchan_0( str);
	for ( i=0; i<n; i++)
		_chan[i]--;
	return n;
}

#define EOS	'\0'

//
//	convert string in format 'a,b,c-d,e,f' to array containing numbers [a,b,c...d,e,f]
//
int
ChannelGr::gtchan_0( const char    *str, 		// input string
					unsigned short *buf,		// integer buf
					int	    max)				// max integer val
{
	register int n = 0;		// buf pointer
	register int first;		// integers
	register int second;
	const char *sp1;
	char *sp2;		// string pointers

	if ( str[ 0] == EOS) return(0);
	sp1 = str;
	while ( 1)		// process string until return
	{
		first = strtol( sp1, &sp2, 10);
		if ((first < 0) || (first > max)) return( -1);
		if ( sp2 == sp1)	// didn't get a number
			return n;
		sp1 = sp2;
		buf[n++] = first;
		switch ( *sp1)		// get next char/chan
		{
			case	',':
					sp1++;
					break;
			case	'-':
					sp1++;
					second = strtol( sp1, &sp2, 10);
					if ( sp1 == sp2)
						return -1;
					sp1 = sp2;
					if ((second < 0) || (second > max) || second == first)
						return -1;
					else
					{
						if ( first < second)
							while ( ++first <= second) buf[n++] = first;
						else
							while ( --first >= second) buf[n++] = first;
						// check next char - only ',' and EOS legal
						if ( *sp1 == ',')
							sp1++;
						else if (*sp1 == EOS) return n;
						else return -1;
					}
					break;
			case	EOS:
					return n;
					break;
			default:
					return -1;
					break;
		}
	}
}

//
//	Store chan in _chan.
//
int
ChannelGr::gtchan_0( const char *str)		// integer buf
{
	register int first;		// integers
	register int second;
	const char *sp1;
	char *sp2;		// string pointers

	if ( str[ 0] == EOS) return(0);
	_chan.clear();

	sp1 = str;
	while ( 1)		// process string until return
	{
		first = strtol( sp1, &sp2, 10);
		if ((first < 0)) return( -1);
		if ( sp2 == sp1)	// didn't get a number
			return 0;
		sp1 = sp2;
		_chan.push_back( first);
		switch ( *sp1)		// get next char/chan
		{
			case	',':
					sp1++;
					break;
			case	'-':
					sp1++;
					second = strtol( sp1, &sp2, 10);
					if ( sp1 == sp2)
						return -1;
					sp1 = sp2;
					if ((second < 0) || second == first)
						return -1;
					else
					{
						if ( first < second)
							while ( ++first <= second) _chan.push_back( first);
						else
							while ( --first >= second) _chan.push_back( first);
						// check next char - only ',' and EOS legal
						if ( *sp1 == ',')
							sp1++;
						else if (*sp1 == EOS)
						{
							return nChan();
						}
						else return -1;
					}
					break;
			case	EOS:
					return nChan();
					break;
			default:
					return -1;
					break;
		}
	}
}
#undef EOS

//
//
//
int
ChannelGr::get_ndiff()
{
	int i, ndiff;
	short *found, max;

	for ( i=max=0; i < nChan(); i++)
		if ( _chan[i] > max) max = _chan[i];
	found = (short *)calloc( max+1, sizeof( short));
	for ( i=ndiff=0; i < nChan(); i++)
	if ( !found[_chan[i]]) {
		found[_chan[i]]++;
		ndiff++;
	}
	free( found);
	return ndiff;
}

//
//	Check that cgr's chan are a superset of ours.
//
bool
ChannelGr::isSuperSet( ChannelGr *cgr)
{
	// assume chan in sequential order
	int i;
	int n = cgr->nChan();
	if ( n > nChan())
		return false;

	else	// make sure each physical chan of ours is in cgr
	{
		for ( i=0; i < n; i++)
		{
			if ( chan(i) != cgr->chan(i))
				break;
		}
		if ( i == n)
			return true;
	}

	// <= #chan but not sequential, this is slow
	for ( i=0; i < n; i++)
	{
		if ( getIndex( cgr->chan(i)) == -1)
			return false;
	}
	return true;
}

//
//	Check that our chan are a subset of cgr's chan.
//
bool
ChannelGr::isSubSet( ChannelGr *cgr)
{
	if ( nChan() > cgr->nChan())
		return false;
	for ( int i=0; i< nChan(); i++)
	{
		if ( cgr->getIndex( chan(i)) < 0)
			return false;
	}
	return true;
}

//
//	Check that cgr's chan are a same as ours.
//
bool
ChannelGr::sameChan( ChannelGr *cgr)
{
	if ( nChan() != cgr->nChan())
		return false;
	for ( int i=0; i< nChan(); i++)
	{
		if ( chan(i) != cgr->chan(i))
			return false;
	}
	return true;
}

bool
ChannelGr::sameChan( unsigned short *ch, int nch)
{
	if ( nChan() != nch)
		return false;
	for ( int i=0; i< nChan(); i++)
	{
		if ( chan(i) != ch[i])
			return false;
	}
	return true;
}

bool
ChannelGr::isContiguous()
{
	for ( int i=0; i<nChan()-1; i++)
		if ( chan(i) + 1 != chan(i+1))
			return false;
	return true;
}

//
//	virtual
//
void
ChannelGr::freeAll()
{
}

// copy operator
ChannelGr&
ChannelGr::operator = ( const ChannelGr& cg)
{
	_name = cg.name();
	setMode( cg.mode());
	_chan = cg._chan;
	return *this;
}
