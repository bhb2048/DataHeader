//
//  DisplayGr.cc - Display group class
//
//  10-Aug-94	jwp	pulled out of glas
//  Version: 1.1
//	Function:
//	    Hold array of channels to be displayed at one time.  Is subset of
//	    Parent group channels.  Channel values are indexes of parent group
//	    chan[] array.
//  Modified:
//  18-feb-98  bhb  moved nrow() here from DasDispGroup
//  11-jun-02  bhb	redo edit() virtual
//	13-jun-02  bhb	add setChanFromParent(), getParentChan()
//	18-sep-02  bhb	add initDefault( ChannelSet *cs)
//	23-sep-02  bhb	add _parentList, index()
//	02-oct-02  bhb	fixup initDefault to do no more than 8 rows & 2 columns
//	20-may-05  bhb	add == operator, DgList::dgNum()
//	08-sep-05  bhb	updated DisplayGr::read/write() for large chan strings
//	06-feb-13  bhb	DgList no longer subclass of list
//	08-dec-15  bhb	comment out getline() in DisplayGr::read() case 1
//	
#include "DisplayGr.h"
#include "ChannelSet.h"
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <string>
#include <sstream>

using namespace std;

DisplayGr::DisplayGr() : _parentList(0), _parent(0), _ncol(1), _axisType(0), _nameType(0),
	_grid(0), _scale(AUTO_SCALE), _xwind(1000)
{
	setName("New DG");
	setMode(DG_DYNAMIC);
	setDynChans(8, 0);
}

// does default init from parent chan
DisplayGr::DisplayGr( ChannelSet *par) : _parentList(0), _parent(0), _axisType(0), 
	_nameType(0), _grid(0), _scale(AUTO_SCALE), _xwind(1000)
{
	if ( par->nChan() <= 16)
	{
		setName( par->name());
		setMode( DG_STATIC);
		for ( int i=0; i < par->nChan(); i++)
			_chan.push_back( i);		// indices into parent chan
		setnColumn(  (par->nChan() > 8) ? 2 : 1);
	}
	else if ( par->nChan() <= 64)
	{
		setName( par->name());
		setMode( DG_DYNAMIC);
		setDynChans( 16, 0);
		setnColumn( 2);
	}
	else
	{
		setName( par->name());
		setMode( DG_DYNAMIC);
		setDynChans( 32, 0);
		setnColumn( 4);
	}
}

DisplayGr::DisplayGr( const DisplayGr &dg) : ChannelGr(dg)
{
	_parentList = dg._parentList;
	_parent = dg._parent;
	_ncol = dg._ncol;
	_axisType = dg._axisType;
	_nameType = dg._nameType;
	_grid = dg._grid;
	_scale = dg._scale;
	_xwind = dg._xwind;
}

DisplayGr::~DisplayGr()
{
	// empty
}

int
DisplayGr::nrow()
{
	int n = nChan() / nColumn();

	if (nChan() % nColumn())
		n++;

	return(n);
}

//
//	use parent chan array to init chan indices
//
int
DisplayGr::setChanFromParent( unsigned short *ch, int nch)
{
	ChannelSet * par = getParent();
	if ( par == 0)
		return 0;

	switch ( mode())
	{
		case DG_STATIC:
		{
			_chan.clear();
			// for each chan in parent, init _chan to index in parent chan[]
			for ( int i=0; i<nch; i++)
			{
				int indx = par->getIndx( int(ch[i]));
				if ( indx >= 0)
					addChan(indx);
			}
		}
			break;
		case DG_DYNAMIC:
		{
			int indx = par->getIndx( int(ch[0]));	// use first chan as start
			setDynChans( nch, indx);
		}
			break;
		default:
			break;
	}
	return nChan();
}

//
//	Return parent chan indexed by chan indices.
//	Assumes pchan[] large enough!!
//
int
DisplayGr::getParentChan( unsigned short *pchan)
{
	ChannelSet * par = getParent();
	if ( par == 0)
		return 0;
	for ( int i=0; i < nChan(); i++)
		pchan[i] = par->chan( chan(i));
	return nChan();
}

//
//	'n' is number of chan
//	'start' is start index in parent Channelset
//
void
DisplayGr::setDynChans( int n, int start)
{
	int ch, pnch;

	if ( getParent() == 0)
		return;

	_chan.clear();

	pnch = getParent()->nChan();
	if ( n > pnch)
		n = pnch;

	ch = start;
	if ( ch < 0)
		ch += pnch;

	for (int i=0; i<n; i++, ch++)
	{
		if ( ch >= pnch)
			ch %= pnch;
		addChan( ch);
	}
}

int
DisplayGr::index()
{
	list<DisplayGr *>::iterator dg = parentList()->begin();

	for ( int i=0; dg != parentList()->end(); dg++, i++)
		if ((*dg) == this)
			return(i);
	return(-1);
}

//
//  on success returns TRUE
//
bool
DisplayGr::read( ifstream &inFile, int dg_ver)
{
	int n, sg=-1;
	int nchan, xwind;
	float ymin, ymax;
	string str, tmp;
	istringstream iss;

	switch (dg_ver)
	{
		case 0:
			/* NSCO	    attr[0]	number of screen columns	*/
			/* AX_TYP   attr[1]	axis type			*/
			/* NM_TYP   attr[2]	name type			*/
			/* GRID	    attr[3]	grid (formerly name size)	*/
			/* V_OL	    attr[4]	vertical overlay percent	*/
			/* GSCALE   attr[5]	scale data plot			*/
			getline( inFile, str);
			iss.str( str);
			iss >> tmp >> _mode >> sg >> nchan;
			getline( inFile, str);
			nchan = gtchan_0( str.c_str());
			getline( inFile, str);
			_name = str;
			getline( inFile, str);
			iss.clear(); iss.str( str);
			iss >> _ncol >> _axisType >> _nameType >> _grid >> n >> _scale;
			_grid = sg;							// pass back parent #
			/* read y-units - not used  */
			getline( inFile, str);
			getline( inFile, str);
			iss.clear(); iss.str( str);
			iss >> _xwind >> ymin >> ymax;
			/* ymin,ymax not used	*/
			break;
		case 1:
			getline( inFile, str);
			_name = str;
			getline( inFile, str);
			iss.str( str);
			iss >> _mode >> sg >> nchan;
			getline( inFile, str);
			nchan = gtchan_0( str.c_str());
			getline( inFile, str);
			iss.clear(); iss.str( str);
			iss >> _ncol >> _axisType >> _nameType >> _grid >> n >> _scale;
			_grid = sg;							// pass back parent #
			getline( inFile, str);
			getline( inFile, str);
			iss.clear(); iss.str( str);
			iss >> _xwind >> ymin >> ymax;
			/* ymin,ymax not used	*/
//			getline( inFile, str);		//?? extra newline, 08-dec-15 not for Yamauchi shunt1
			break;
		case 2:
			getline( inFile, str);
			_name = str;
			getline( inFile, str);
			iss.str( str);
			iss >> _mode >> nchan;
			getline( inFile, str);
			nchan = gtchan_0( str.c_str());
			getline( inFile, str);
			iss.clear(); iss.str( str);
			iss >> _ncol >> _axisType >> _nameType >> _grid >> _scale >> _xwind;
			getline( inFile, str);					// extra newline
			break;
		default:
			return false;
	}
	return true;
}

void
DisplayGr::write( ofstream &outFile)
{
	char string[2048];
	int		at = axisType() & 0xf;	// clear AXIS_ZEROX bit

	outFile << name() << endl;
	outFile << (int)mode() << " " <<  nChan() << endl;
	outFile << numstr( string) << endl;
	outFile << nColumn() << " " << at << " " << nameType() << " " << grid() << " "
			<< scale() << " " << xwind() << endl << endl;		// extra newline
}

//
//	Make default DG displaying upto 8 rows in no more than two columns, using parent chan.
//	If more than 16 chan make Dynamic w/ 8 chan in 1 col.
//
void
DisplayGr::initDefault()
{
	ChannelSet * par = getParent();
	if ( par == 0)
		return;

	unsigned short ch[NCHAN];
	int nch = par->nChan();
	int ncol = 1;
	if ( nch <= 16)
	{
		setMode( DG_STATIC);
		if ( nch > 8)
		ncol = 2;
	}
	else
	{
		setMode( DG_DYNAMIC);
		nch = 8;
	}
	setnColumn( ncol);
	for ( int i=0; i<nch; i++)
		ch[i] = par->chan(i);
	setChanFromParent( ch, nch);
}

//
//	mostly equal operator
//
bool
DisplayGr::operator == ( const DisplayGr &dg) const
{
	return (nChan() == dg.nChan() && mode() == dg.mode() && nColumn() == dg.nColumn() &&
			strcmp( name(), dg.name()) == 0);
}


//////////////////////////////////////////////////////////////////////////
//	    DgList class
//////////////////////////////////////////////////////////////////////////

DgList::DgList()
{

}

DgList::DgList( const DgList &dgl)
{
	// create copyies of dgl's display groups...
	for ( list<DisplayGr *>::const_iterator dgi = dgl.begin(); dgi != dgl.end(); dgi++)
	{
		DisplayGr *dg = new DisplayGr( *(*dgi));
		dg->setParentList( this);
		_list.push_back( dg);
	}
}

DgList::~DgList()
{
	free_all();
}

void
DgList::free_all()
{
	if ( size() == 0)
		return;
	for ( list<DisplayGr *>::iterator i = begin(); i != end(); i++)
		delete (*i);
	_list.clear();
}

DisplayGr *
DgList::elem( int i)
{
	list<DisplayGr *>::iterator g;

	if ( unsigned(i) < size())
	{
		for ( g = begin(); i > 0; i--, g++);
		return( *g);
	}
	else return 0;
}

//
//	return index of matching dg
//
int
DgList::dgNum( DisplayGr *dg)
{
	int i = 0;
	for ( list<DisplayGr *>::iterator g = begin(); g != end(); i++, g++)
		if ( *g == dg)
			return i;
	return -1;			// not found
}

//----------------------------------------------------------------------
//  virtual routines
//----------------------------------------------------------------------
void
DgList::edit( ChannelSet *)
{
}

int
DgList::read( ifstream &inFile)
{
	int n;
	int ndg;
	int dg_ver = 0;
	DisplayGr *dg;
	string str, tmp;
	istringstream iss;

	free_all();
	
	getline( inFile, str);
	iss.str( str);
	iss >> dg_ver;
	switch (dg_ver)
	{
		case 1:
			getline( inFile, str);
			iss.clear(); iss.str( str);
			iss >> ndg;
			break;
		case 2:
			getline( inFile, str);
			iss.clear(); iss.str( str);
			iss >> tmp >> tmp >> ndg;				// swallow "Display Groups:"
			break;
		default:
			return 0;
	}

	for ( n=0; n < ndg; n++)
	{
		dg = new DisplayGr;
		dg->read( inFile, dg_ver);
		_list.push_back(dg);
	}
	return ndg;
}

int
DgList::read( ifstream &inFile, int hdr_ver)
{
	int n;
	int ndg;
	int dg_ver = 0;
	DisplayGr *dg;
	string str, tmp;
	istringstream iss;

	free_all();
	
	switch (hdr_ver)
	{
		case 5:
		case 6:
			getline( inFile, str);
			iss.str( str);
			iss >> ndg;
			break;
		case 7:
		case 8:
		case 9:
		case 10:
			getline( inFile, str);
			iss.str( str);
			iss >> dg_ver;
			switch (dg_ver)
			{
				case 1:
					getline( inFile, str);
					iss.clear(); iss.str( str);
					iss >> ndg;
					break;
				case 2:
					getline( inFile, str);
					iss.clear(); iss.str( str);
					iss >> tmp >> tmp >> ndg;				// swallow "Display Groups:"
					break;
				default:
					return 0;
			}
			break;
		default:
			return 0;
	}
	for ( n=0; n < ndg; n++)
	{
		dg = new DisplayGr;
		dg->read( inFile, dg_ver);
		dg->setParentList( this);
		_list.push_back(dg);
	}
	return ndg;
}

int
DgList::write( ofstream &outFile)
{
	list<DisplayGr *>::iterator dg;

	outFile << CUR_DG_VERSION << endl;
	outFile << "Display Groups: " << size() << endl;
	for ( dg = begin(); dg != end(); dg++)
		(*dg)->write( outFile);
	return 1;
}
