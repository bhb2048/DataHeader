//
//	ChannelSet.cc - routines to store and manipulate ChannelSets
//
//	09-Aug-94	jwp	split from sampgroups.c++ in glas
//	Modified:
//	20-jun-02  bhb	add virtual initDefNames() to use actual channels
//	20-sep-02  bhb	added copy operator
//	20-may-05  bhb	added _curDg init in constructors
//	23-may-05  bhb	merge ChannelInfo class into ChannelSet
//					use new ChannelGr member funcs nusmtr(), gtchan_0
//	09-nov-05  bhb	add setDgList()
//	07-dec-05  bhb	setNames(): add names, n args init to 0
//			   bhb	initDefNames(): use chan(i) for default name, not i
//	10-oct-06  bhb	bug fix - add dg parent init in operator =
//	02-oct-08  bhb	add minNumDgChan() & setDgWithLEChan() as help utilities for Dash
//	
#include "ChannelSet.h"
#include "DisplayGr.h"
#include "DataHeader.h"			// gtchan_0()
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

//*****************************************************************************
//
//  ChannelSet class member functions
//
//*****************************************************************************
ChannelSet::ChannelSet( cs_type type, int nch) : _type(type), _curDg(0), _nNames(0), 
	_nUnits(0), ChannelGr( nch)
{
	_dgs = new DgList;
}

/* make a copy	*/
ChannelSet::ChannelSet( const ChannelSet &cs) : ChannelGr(cs)
{
	int i;

	_dgs = new DgList( *cs.dgList());		// makes copys of DG's
	for ( list<DisplayGr *>::const_iterator dg = dgList()->begin(); dg != dgList()->end(); dg++)
		(*dg)->setParent(this);
	if ( dgList()->size())
		setCurDg( *(_dgs->begin()));
	else
		setCurDg( 0);
	_type = cs.type();
	
	_nNames = cs._nNames;
	if ( _nNames)
	{
		for (i=0; i<_nNames; i++)
		{
			_names[i] = cs._names[i];
		}
	}

	_nUnits = cs._nUnits;
	if ( _nUnits)
	{
		for (i=0; i<_nUnits; i++)
		{
			_units[i] = cs._units[i];
		}
		_unitsIndx = cs._unitsIndx;
	}
}

ChannelSet::~ChannelSet()
{
	freeAll();
}

/*
 *  get number of child display groups
 */
int
ChannelSet::numDgs()
{
	return( _dgs->size());
}

void
ChannelSet::addDg( DisplayGr *dgr)
{
	_dgs->dglist()->push_back(dgr);
	dgr->setParent(this);
}

void
ChannelSet::setDgList( DgList *dgl)
{
	 if ( dgList() != 0)
		delete _dgs;
	_dgs = (DgList *)(new DgList( *dgl));
	for ( list<DisplayGr *>::iterator dgi = dgList()->begin(); dgi != dgList()->end(); dgi++)
		(*dgi)->setParent( this);
}

DisplayGr *
ChannelSet::dg(int i)
{
	if ( i >= 0 && _dgs->size() > unsigned(i))
		return _dgs->elem(i);
	else
		return 0;
}


//
//  Set units indx vector.
//  'sub' allows 0 or 1 based channel str.
//
void
ChannelSet::setUnitsIndx( int u, const char *str, int sub)	// sub: 0 or 1
{
	if ( str == 0 || strlen( str) == 0)
		return;
	unsigned short *uc = new unsigned short[ NCHAN];
	int n = gtchan_0( str, uc, NCHAN);		// may return -1

	for ( int i=0; i<n; i++)
	{
		int c = uc[i] - sub;
		if ( c >= 0 && c < n)
			_unitsIndx[c] = u;
	}
	delete [] uc;
}

//
//	Get chan with units index 'u'.
//	Initialize array (uc) with chan with units index u.
//	Return n chan with units u.
//	
int
ChannelSet::unitsChan( int u, unsigned short *uc)
{
	int i, n, nui = _unitsIndx.size();

	if ( nui > nChan())
	{
		cerr << "ChannelSet::unitsChan: error, _unitsIndx has wrong size" << endl;
		nui = nChan();
	}
	for ( i=n=0; i < _unitsIndx.size(); i++)
		if ( _unitsIndx[i] == u)
			uc[n++] = i;
	return n;
}

void
ChannelSet::setUnitsChan( unsigned short c, unsigned short u)
{
	if ( u < _unitsIndx.size())
		_unitsIndx[c] =  u;
}

//
//	set default channel names
//	
void
ChannelSet::setNames( char **names, int n)
{

	if ( n > 0 && names != 0)
	{
		_nNames = (nChan() > NCHANNAMES) ? NCHANNAMES : nChan();
		for ( int i=0; i < _nNames; i++)
		{
			_names[i] = names[i];
		}
	}
	else
		initDefNames( nChan());
}

void
ChannelSet::initDefUnits( int n)
{
	int i;

	_unitsIndx.clear();
	setUnits( 0, "mv");
	setnUnits( 1);
	for ( i=1; i<MAX_UNITS; i++)
		setUnits( i, "");
	_unitsIndx.reserve( n);
	_unitsIndx.assign( n, 0);	// all chan use _units[0]
}

int
ChannelSet::minNumDgChan()
{
	int min = 100000;
	DgList *dgl = dgList();

	for ( list<DisplayGr *>::iterator dgi = dgl->begin(); dgi != dgl->end(); dgi++)
	{
		if ( (*dgi)->nChan() < min)
			min = (*dgi)->nChan();
	}
	return min;
}

//
//	Set current DG to first w/ <= n chan.  Return success.
//	
bool
ChannelSet::setDgWithLEChan( int n)
{
	DgList *dgl = dgList();
	for ( list<DisplayGr *>::iterator dgi = dgl->begin(); dgi != dgl->end(); dgi++)
	{
		if ( (*dgi)->nChan() <= n)
		{
			setCurDg( *dgi);
			return true;
		}
	}
	return false;
}

void
ChannelSet::rd_chnames( ifstream &inFile, int nchan)
{
	int i, n = (nchan > NCHANNAMES) ? NCHANNAMES : nchan;
	string str;
	istringstream iss;
	
	_nNames = n;
	for ( i = 0; i<_nNames;)
	{
		getline( inFile, str);
		iss.str( str);
		for ( int j=0; j < 16; j++)
			iss >> _names[i++];
		iss.clear();
	}
}

void
ChannelSet::wr_chnames( ofstream &outFile, int nchan)
{
	int i, n;
	n = (nchan > NCHANNAMES) ? NCHANNAMES : nchan;

	for ( i = 0; i<n;)
	{
		if (i<_nNames)
			outFile << _names[i] << " ";
		else
			outFile << i << " ";
		if ( !(++i % 16))
			outFile << endl;
	}
	if ( n % 16)
		outFile << endl;
}

void
ChannelSet::rd_units( ifstream &inFile, int nchan)
{
	register int i, j;
	string str, tmp;
	istringstream iss;
	
	getline( inFile, str);
	iss.str( str);
	iss >> tmp >> _nUnits;		// swallow "Units:"
	if ( numUnits())
	{
		_unitsIndx.clear();
		_unitsIndx.resize( nchan, 0);
		for ( i=0; i<numUnits(); i++)
		{
			getline( inFile, str);
			_units[i] = str;
			getline( inFile, str);
			if ( str.length())
				setUnitsIndx( i, str.c_str(), 0);
		}
	}
}

void
ChannelSet::wr_units( ofstream &outFile, int nchan)
{
	register int i, n;
	char str[256];
	unsigned short *uc = new unsigned short[ nchan];

	outFile << "Units: " << numUnits() << endl;
	for ( i=0; i<numUnits(); i++)
	{
		outFile <<  _units[i] << endl;
		n = unitsChan( i, uc);
		if ( n > 0)
			outFile << numstr(str, uc, n) << endl;
		else
			outFile << endl;		// need something so read doesn't swallow
									// whitespace and get following data
	}
	delete [] uc;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
//	Virtual functions
//	
//////////////////////////////////////////////////////////////////////////////////////////////
//
//	Virtual version to use actual channels
//	
void
ChannelSet::initDefNames(int n)
{
	if ( n <= 0)
		return;
	_nNames = (n > NCHANNAMES) ? NCHANNAMES : n;
	for ( int i=0; i < _nNames; i++)
	{
		char nm[4];
		sprintf( nm, "%3d", chan(i)+1);
		_names[i] = nm;
	}
}

void
ChannelSet::editDgs()
{
	// Empty
}

void
ChannelSet::freeAll()
{
	delete _dgs;
}

bool
ChannelSet::read( ifstream &inFile)
{
	int nchan;
	string str;
	istringstream iss;
	int type;
	
	getline( inFile, str);
	_name = str;
	getline( inFile, str);
	iss.str( str);
	iss >> type >> _mode >> nchan;
	_type = (cs_type)type;
	
	getline( inFile, str);
//	fgets( string, 256, fp);			// avoid eating white space at start of next line
//	string[ strlen( string) - 1] = '\0';// delete '\n'
	nchan = gtchan_0( str.c_str());
	rd_chnames( inFile, nchan);
	rd_units( inFile, nchan);
	getline( inFile, str);
	dgList()->read( inFile);
	for ( list<DisplayGr *>::iterator dg = dgList()->begin(); dg != dgList()->end(); dg++)
		(*dg)->setParent(this);
	return true;
}

bool
ChannelSet::write( ofstream &outFile)
{
	char str[256];

	outFile << name() << endl;
	outFile << type() << " " << mode() << " " << nChan() << endl;
	outFile << numstr( str) << endl;
	wr_chnames( outFile, nChan());
	wr_units( outFile, nChan());
	outFile << endl;
	dgList()->write( outFile);
	return true;
}

// copy operator
ChannelSet&
ChannelSet::operator = ( const ChannelSet& cs)
{
	int i;
	
	ChannelGr::operator = ( cs);
	freeAll();

	_dgs = new DgList( *cs.dgList());
	for ( list<DisplayGr *>::const_iterator dg = dgList()->begin(); dg != dgList()->end(); dg++)
		(*dg)->setParent(this);

	if ( _dgs->dglist()->size())
		setCurDg( *(_dgs->begin()));
	else
		setCurDg( 0);

	_nNames = cs._nNames;
	if ( _nNames)
	{
		for (i=0; i<_nNames; i++)
		{
			_names[i] = cs._names[i];
		}
	}

	_nUnits = cs._nUnits;
	if ( numUnits())
	{
		for (i=0; i<numUnits(); i++)
		{
			_units[i] = cs._units[i];
		}
		_unitsIndx = cs._unitsIndx;
	}

	return *this;
}
