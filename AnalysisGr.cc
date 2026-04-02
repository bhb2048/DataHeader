//
//	AnalysisGr.cc - routines to store and manipulate analysis groups
//
//	Channels are physical channels to analyze.
//
//	10-Aug-94	jwp	pulled out of glas
//	Modified:
//	07-nov-01  bhb	removed AgList::edit()
//	20-sep-02  bhb	added copy (=) operator
//	08-oct-02  bhb	began try/catch block for errors in AnalysisGr::read(), needs more work
//	07-nov-02  bhb	added AModControl, incr AG version
//	11-nov-02  bhb	added updateControls()
//	15-dec-04  bhb	init _numMods, _numOuts in constr.
//	07-nov-05  bhb	make _dir, _file1, _file2 string type, modify setDir() to append
//					'/' if missing
//	11-jan-06  bhb	changed AG ver 1 read to set _modeName like ver 0
//	18-jan-06  bhb	Now that we have old/new mode names, make sure they agree in checkAMode()
//					also fixed up ag copy ctor
//	27-oct-12  bhb	change _dataTitle to vector<string>, remove _numOuts
//					remove #define MAX_AMODE_OUTPUTS
//	08-dec-15  bhb	fix read() for Yamauchi shunt1 ag ver=2
//	
#include "AnalysisGr.h"
#include "DataHeader.h"			// for Parent()->Parent()->
#include "DisplayGr.h"
#include <Data/dataf.h>			// legacy conversion
#include <Data/AMode.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdlib.h>				// strtoul(), strtod() (no ISOC99 so no strtof())
#include <sstream>

using namespace std;
using namespace bhb;

//*****************************************************************************
//
//  AnalysisGr class member functions
//
//*****************************************************************************
AnalysisGr::AnalysisGr( AgList * p) : _parent(p), _count(0), _AMode(0),
	_numMods(0), ChannelSet(CS_TYPE_AG)
{
	setName( "New Ag");
	setnScaleVal(0);
}

AnalysisGr::AnalysisGr( const AnalysisGr &ag) : ChannelSet(ag)
{
	int i, j;

	_parent = ag._parent;			// AgList
	_count = 0;
	if ( ag._AMode != 0)
		_AMode = new AMode( *ag._AMode);
	else
		_AMode = 0;

	setDir( ag._dir);
	setFile1( ag._file1);
	setFile2( ag._file2);
	setnScaleVal( ag.nScaleVal());
	for (i=0; i<20; i++)
		_scVal[i] =  ag.scaleVal(i);

	_numMods =  ag.numMods();
	for (i=0; i<_numMods; i++)
	{
		const vector<AModControl> *octrls = ag.controls(i);
		vector<AModControl> *nctrls = controls(i);
		vector<AModControl>::const_iterator ci;
		for ( ci = octrls->begin(); ci != octrls->end(); ci++)
			nctrls->push_back( *ci);
	}
	int numOuts =  ag.numOuts();
	_dataTitle.clear();
	for ( i=0; i < numOuts; i++)
		setDataTitle( ag.dataTitle(i));
	_modeName = ag._modeName;
}

AnalysisGr::~AnalysisGr()
{
	if ( _AMode != 0)
		delete _AMode;
}

void
AnalysisGr::setDir( const char *d)
{
	_dir = d;
	if ( _dir.size() && _dir[_dir.size()-1] != '/')
		_dir.append( "/");
}

int
AnalysisGr::setControl( int m, int ci, int t, void *v)
{
	vector<AModControl>	*ctrls = controls(m);
	if ( ctrls->size() <= ci)
		return 0;
	AModControl *c = &((*ctrls)[ci]);
	if ( c->type != t)
		return 0;
	switch (t)
	{
		case AMC_TYPE_UCHAR:
			c->ctrl.uc = *((unsigned char *)v);
			break;
		case AMC_TYPE_USHORT:
			c->ctrl.us = *((unsigned short *)v);
			break;
		case AMC_TYPE_UINT:
			c->ctrl.ui = *((unsigned int *)v);
			break;
		case AMC_TYPE_FLOAT:
			c->ctrl.fl = *((float *)v);
			break;
		case AMC_TYPE_CHAR:
			c->ctrl.c = *((char *)v);
			break;
		case AMC_TYPE_SHORT:
			c->ctrl.s = *((short *)v);
			break;
		case AMC_TYPE_INT:
			c->ctrl.i = *((int *)v);
			break;
		default:
			return 0;
	}
	return 1;
}

int
AnalysisGr::setControl( int m, int ci, AModControl *ctrl)
{
	vector<AModControl>	*ctrls = controls(m);
	if ( ctrls->size() <= ci)
		return 0;
	AModControl *c = &((*ctrls)[ci]);
	*c = *ctrl;
	return 1;
}

void *
AnalysisGr::controlVal( int m, int c)
{
	vector<AModControl>	*ctrls = controls(m);
	return (void *)(&((*ctrls)[c]).ctrl.fl);
}

void
AnalysisGr::updateControls( int i, vector<AModControl> *ctrls)
{
	controls(i)->clear();
	for ( vector<AModControl>::iterator ci = ctrls->begin(); ci != ctrls->end(); ci++)
		controls(i)->push_back( *ci);		// copies
	if ( i == numMods())					// make sure _numMods is correct
		_numMods++;
}

//
//  get our own index in parent group
//
int
AnalysisGr::index()
{
	if ( Parent() != 0)
	{
		return Parent()->index( this);
	}
	return -1;
}

//
//	private
//	check AG _numMods & _numOutputs against actual AMode incase AMode changed
//
bool
AnalysisGr::checkAMode()
{
	char str[128];

	if ( numMods() != amode()->numMods() || numOuts() != amode()->numOutputs())
	{
		sprintf( str, 
"Analysis Group %s's mode has changed, initializing from defaults.",
			name());
		_errStr = str;
		initFromAMode();
		return false;
	}
	else
	{
		// just make sure modeName agrees w/ AMode
		modeName( amode()->name());
	}

	// check controls
	for ( int i=0; i< numMods(); i++)
	{
		if ( controls(i)->size() != amode()->module(i)->controls()->size())
		{
			sprintf( str, 
"Analysis Group %s's number of controls have changed, initializing from defaults.",
				name());
			_errStr = str;
			setControls();
			return false;
		}
		// check each control type
		for ( int j=0; j < controls(i)->size(); j++)
		{
			int type1 = (*controls(i))[j].type;
			int type2 = (*amode()->module(i)->controls())[j].type;
			if ( (*controls(i))[j].type != (*amode()->module(i)->controls())[j].type)
			{
				sprintf( str, 
"Analysis Group %s's controls have changed, initializing from defaults.",
					name());
				_errStr = str;
				setControls();
				return false;
			}
		}
	}
	return true;
}

//
//	Initialize analysis group's AMode related module and name/unit info from 'am'
//
bool
AnalysisGr::setAMode( AMode *am, bool fromAM)
{
	if ( am == 0)
	{
		return false;
	}
	// store a copy of this AMode
	if ( _AMode == 0)
		_AMode = new AMode( *am);			// make a copy
	else if ( amode()->id() != am->id())
	{
		 delete amode();
		 _AMode =  new AMode( *am);			// make a copy
	}

	if ( fromAM)
		initFromAMode();					// init AG's local values from this AMode
	else
	{
		setMode( amode()->id());			// this is not saved in header for some reason
		if ( checkAMode())
			amode()->initControls( this);		// init _AMode from this AG's controls
	}
	return true;
}

//
//	set AG Module controls from AMode Module default values
int
AnalysisGr::setControls()
{
	int i, j;

	if ( amode() == 0 || amode()->id() != mode())
		return 0;

	// first check that #modules same as 'am' because old Mod's had
	// extra Map & Scale modules which are no longer used
	if ( numMods() != amode()->numMods())
		_numMods = amode()->numMods();

	for (i=0; i< numMods(); i++)
	{	// use default control vals
		updateControls( i, amode()->module(i)->defCntrls());
	}
	return 1;
}

//
//	set AG Module controls from AMode Module values
int
AnalysisGr::setControls( AMode * am)
{
	int i, j;

	if ( am == 0 || am->id() != mode())
		return 0;

	// first check that #modules same as 'am' because old Mod's had
	// extra Map & Scale modules which are no longer used
	if ( numMods() != am->numMods())
		_numMods = am->numMods();

	for (i=0; i< numMods(); i++)
	{
		for (j=0; j<am->module(i)->controls()->size(); j++)
			setControl( i, j, am->module(i)->control(j));
	}
	return 1;
}


bool
AnalysisGr::initFromAMode()
{
	if ( amode() == 0)
		return false;

	setMode( amode()->id());
	modeName( amode()->name());

	// set default data_titles
	int numOuts = amode()->numOutputs();
	_dataTitle.clear();
	for ( int i=0; i < numOuts; i++)
	{
		setDataTitle( amode()->dataTitle(i));
	}

	// set default controls
	setControls();

	return true;
}

bool
AnalysisGr::read( ifstream &inFile, uint ag_ver, int hdr_ver)
{
	int h, i, j, n, nchan, sg;
	string	str, tmp;
	istringstream iss;

	_dataTitle.clear();

  try {
	switch ( ag_ver)
	{
		case 0:
			getline( inFile, str);
			iss.str( str);
			iss >> _name >> h >> sg >> nchan;

			_modeName = oldmode_newmode(h);

			getline( inFile, str);
			nchan = gtchan_0( str.c_str());
			getline( inFile, str);		// 16 parameters ignored

			getline( inFile, str);
			iss.clear(); iss.str( str);
			if ( !isdigit(str[0]))
			{
				string dir, f1, f2;
				iss >> dir >> f1 >> f2;
				setDir( dir);
				setFile1( f1);
				setFile2( f2);
				getline( inFile, str);
				iss.clear(); iss.str( str);
			}
			iss >> _nScVal;

			getline( inFile, str);
			iss.clear(); iss.str( str);
			for ( i=0; i<_nScVal; i++)
				iss >> _scVal[i];

			// set default _controls & titles
			_numMods = 0;
			setDataTitle( "Output 1");

			initDefNames(nchan);
			initDefUnits(nchan);
			break;
		case 1:
			getline( inFile, str);
			_name = str;
			getline( inFile, str);
			iss.str( str);
			iss >> h >> sg >> nchan;

			_modeName = oldmode_newmode(h);

			getline( inFile, str);
			nchan = gtchan_0( str.c_str());
			getline( inFile, str);		// 16 parameters ignored

			getline( inFile, str);
			iss.clear(); iss.str( str);
			if ( !isdigit(str[0]) )
			{
				string dir, f1, f2;
				iss >> dir >> f1 >> f2;
				setDir( dir);
				setFile1( f1);
				setFile2( f2);
				getline( inFile, str);
				iss.clear(); iss.str( str);
			}
			iss >> _nScVal;
			getline( inFile, str);
			iss.clear(); iss.str( str);
			for ( i=0; i<_nScVal; i++)
				iss >> _scVal[i];

			// set default _controls & titles
			_numMods = 0;
			setDataTitle( "Output 1");

			initDefNames(nchan);
			initDefUnits(nchan);
			break;

		case 2:
		case 3:				// output data titles stored after _controls
		case 4:				// output names, units, display groups
		case 5:				// use AModControl struct
			getline( inFile, str);
			_name = str;
			getline( inFile, str);
			iss.str( str);
			if ( ag_ver < 4)
			{
				iss >> sg >> nchan;
			}
			else
				iss >> nchan;
			getline( inFile, str);
			nchan = gtchan_0( str.c_str());

			getline( inFile, str);
			_modeName = str;

			getline( inFile, str);
			iss.clear(); iss.str( str);
			iss >> tmp >> tmp >> _numMods;		// swallow '#mods ='
			if ( _numMods < 1 || _numMods > MAX_MODULES)	// assume error
			{
				throw modErr;
			}
			for ( i=0; i < _numMods; i++)
			{
				vector<AModControl> *ctrls = controls(i);
				ctrls->clear();				// empty this modules controls
				getline( inFile, str);		// read line
				iss.clear(); iss.str( str);
				if ( ag_ver < 5)
				{
					struct AModControl c[3];
					c[0].type = c[1].type = c[2].type = AMC_TYPE_FLOAT;
					iss >> c[0].ctrl.fl >> c[1].ctrl.fl >> c[2].ctrl.fl;
					for ( j=0; j<3; j++)
						ctrls->push_back( c[j]);
				}
				else
				{
					struct AModControl c;

					iss >> n;		// num controls
					for ( j = 0; j < n; j++)
					{
						unsigned short tmp;
						iss >> c.type;	// get val type
						switch ( c.type)
						{
							case AMC_TYPE_UCHAR:
								iss >> tmp;				// stringstream '>>' converts to char val
								c.ctrl.uc = tmp;		// if use unsigned char on right side
								break;					// e.g. 1 >> '1' == 49
							case AMC_TYPE_USHORT:
								iss >> c.ctrl.us;
								break;
							case AMC_TYPE_UINT:
								iss >> c.ctrl.ui;
								break;
							case AMC_TYPE_FLOAT:
								iss >> c.ctrl.fl;
								break;
							case AMC_TYPE_CHAR:
								iss >> tmp;				// stringstream '>>' converts to char val
								c.ctrl.c = tmp;			// if use char on right side
								break;
							case AMC_TYPE_SHORT:
								iss >> c.ctrl.s;
								break;
							case AMC_TYPE_INT:
								iss >> c.ctrl.i;
								break;
							default:
								throw ctrlErr;
								break;
						}
						ctrls->push_back( c);
					}
				}
			}

			if ( ag_ver > 2)			// 08-dec-15  Yamauchi shunt1 skip, maybe higher vers?
			{
				// read titles
				getline( inFile, str);		// read line
				iss.clear(); iss.str( str);
				int numOuts;
				iss >> tmp >> tmp >> numOuts;		// swallow "#outputs ="
				for ( h=0; h < numOuts; h++)
				{
					getline( inFile, str);		// read output data title line
					setDataTitle( str.c_str());
				}
			}
			getline( inFile, str);			// swallow newline
			getline( inFile, str);
			iss.clear(); iss.str( str);
			if ( !isdigit(str[0]) )
			{
				string dir, f1, f2;
				iss >> dir >> f1 >> f2;
				setDir( dir);
				setFile1( f1);
				setFile2( f2);
				getline( inFile, str);
				iss.clear(); iss.str( str);
			}
			iss >> _nScVal;
			if ( _nScVal > 0)
			{
				getline( inFile, str);
				iss.clear(); iss.str( str);
				for ( i=0; i<_nScVal; i++)
					iss >> _scVal[i];
			}
			else
				getline( inFile, str);		// swallow blank line
			if ( ag_ver >= 4)
			{
				rd_chnames( inFile, nchan);
				rd_units( inFile, nchan);
				getline( inFile, str);		// swallow blank line
				dgList()->read( inFile, hdr_ver);
				for ( list<DisplayGr *>::iterator dg = dgList()->begin(); dg != dgList()->end(); dg++)
					(*dg)->setParent(this);
			}
			else
			{
				initDefNames(nchan);
				initDefUnits(nchan);
			}
			if ( ag_ver > 2)			// 08-dec-15  Yamauchi shunt1 skip, maybe higher vers?
				getline( inFile, str);		// swallow blank line
			break;
		default:
			throw versionErr;
			return false;
	}
  }
  catch ( AgReadErr Err)
  {
	_errStr = "Reading Analysis Group: " + _name;
	switch ( Err)
	{
		case versionErr:
			_errStr = _errStr + "  bad version";
			break;
		case nameErr:
			_errStr = _errStr + "  Error reading name";
			break;
		case modErr:
			_errStr = _errStr + "  Error reading Mode";
			break;
		case ctrlErr:
			_errStr = _errStr + "  Error reading controls";
			break;
		case outErr:
			_errStr = _errStr + "  reading outputs";
			break;
		default:
			_errStr = _errStr + "  unknown error";
			break;
	}
	return false;
  }

	if ( dgList()->size() > 0)
		setCurDg( *dgList()->begin());

	return true;
}	// read()

bool
AnalysisGr::write( ofstream &outFile)
{
	char string[256];

	outFile << name() << endl;
	outFile << nChan() << endl;
	outFile << numstr( string) << endl;

	outFile << modeName() << endl;
	outFile << "#mods = " << numMods() << endl;
	for ( int i=0; i<numMods(); i++)
	{
		vector<AModControl> *ctrls = controls(i);
		vector<AModControl>::iterator ci;
		outFile << ctrls->size() << " ";
		for ( ci = ctrls->begin(); ci != ctrls->end(); ci++)
		{
			unsigned short tmp;
			outFile << (*ci).type << " ";
			switch ( (*ci).type)
			{
				case AMC_TYPE_UCHAR:
					tmp = (*ci).ctrl.uc;			// fix '<<' convert to char prob
					outFile << tmp << " ";
					break;
				case AMC_TYPE_USHORT:
					outFile << (*ci).ctrl.us << " ";
					break;
				case AMC_TYPE_UINT:
					outFile << (*ci).ctrl.ui << " ";
					break;
				case AMC_TYPE_FLOAT:
					outFile << (*ci).ctrl.fl << " ";
					break;
				case AMC_TYPE_CHAR:
					tmp = (*ci).ctrl.c;			// fix '<<' convert to char prob
					outFile << tmp << " ";
					break;
				case AMC_TYPE_SHORT:
					outFile << (*ci).ctrl.s << " ";
					break;
				case AMC_TYPE_INT:
					outFile << (*ci).ctrl.i << " ";
					break;
			}
		}
		outFile << endl;
	}

	outFile << "#outputs = " << numOuts() << endl;
	for ( int i=0; i<numOuts(); i++)
		outFile << dataTitle(i) << endl;

	outFile << endl << Dir() << " " << File1() << " " << File2() << endl;
	outFile << nScaleVal() << endl;
	for ( int i=0; i<nScaleVal(); i++)
		outFile << scaleVal(i) << " ";
	outFile << endl;
	wr_chnames( outFile, nChan());
	wr_units( outFile, nChan());
	outFile << endl;
	dgList()->write( outFile);
	outFile << endl;

	return true;
}

static string amoderet[] =
{
	"BIPOLAR ACTIVATION",
	"ACTIVATION RECOVERY INTERVAL",
	"UNKNOWN ANALYSIS",
	"UNKNOWN ANALYSIS",
	"UNKNOWN ANALYSIS",
	"UNKNOWN ANALYSIS",
	"UNKNOWN ANALYSIS",
	"UNKNOWN ANALYSIS",
	"POTENTIAL DISTRIBUTION ANALYSIS",
	"UNKNOWN ANALYSIS",
	"UNKNOWN ANALYSIS",
	"FREQUENCY DATA",
	"POTENTIAL THRESHOLD TIMES",
	"UNIPOLAR ACTIVATION"
};

const char *
AnalysisGr::oldmode_newmode( int oldmode)
{

	if ( oldmode >= BIP1 && oldmode < FOUR1)
		return amoderet[ oldmode - BIP1].c_str();
	else return amoderet[ AHG1 - BIP1].c_str();		// "UNKNOWN ANALYSIS"
}

// copy operator
AnalysisGr&
AnalysisGr::operator = ( const AnalysisGr& ag)
{
	int i, j;

	ChannelSet::operator=(ag);		// copy ChannelSet part

	_parent = ag._parent;
	_count = 0;
	if ( ag._AMode != 0)
		_AMode = new AMode( *ag._AMode);
	else
		_AMode = 0;

	setDir( ag._dir);
	setFile1( ag._file1);
	setFile2( ag._file2);

	_modeName = ag._modeName;

	// should these be in scale module?
	_nScVal = ag._nScVal;
	for ( i=0; i < _nScVal; i++)
		_scVal[ i] = ag._scVal[ i];

	_numMods = ag._numMods;

	for ( i=0; i < _numMods; i++)
	{
		const vector<AModControl> *octrls = ag.controls(i);
		vector<AModControl> *nctrls = controls(i);
		vector<AModControl>::const_iterator ci;
		nctrls->clear();				// empty
		for ( ci = octrls->begin(); ci != octrls->end(); ci++)
			nctrls->push_back( *ci);
	}
	int numOuts = ag.numOuts();

	_dataTitle.clear();
	for ( i=0; i < numOuts; i++)
	{
		setDataTitle( ag.dataTitle(i));
	}

	return *this;
}

///////////////////////////////////////////////////////////////////
///////////////	    AgList	   ////////////////////////////////////
///////////////////////////////////////////////////////////////////

AgList::AgList( DataHeader *hdr) : _parent(hdr), _AModes(0), _freeAModes(false), _version(0)
{
}

AgList::AgList( const AgList &agl)
{
	_list = agl._list;
}

AgList::~AgList()
{
	if ( _AModes != 0 && _freeAModes)
		delete _AModes;
	free_all();
}

void
AgList::free_all()
{
	for ( std::list<AnalysisGr *>::iterator i = agList()->begin(); i != agList()->end(); i++)
		delete (*i);
	agList()->clear();
}

AnalysisGr *
AgList::elem( int i)
{
	std::list<AnalysisGr *>::iterator g;
	if ( unsigned(i) < size())
	{
		for ( g = agList()->begin(); i > 0; i--, g++);
		return( *g);
	}
	else return 0;
}

bool
AgList::setAModes( AModeList *aml)
{
	string errMsg;
	
	if ( aml != 0)
	{
		if ( aml == _AModes)
			return true;
		if ( _freeAModes)
			delete _AModes;
		_AModes = aml;
		_freeAModes = false;		// someone else's responsibility to delete
	}
	else if ( AModes() == 0)
	{
		_AModes = new AModeList();
		_freeAModes = true;		// our responsibility to delete
		if ( !_AModes->read( errMsg))
		{
			_errStr = errMsg;
			return false;
		}
	}
	return true;
}

int
AgList::index( AnalysisGr *ag)
{
	std::list<AnalysisGr *>::iterator agi = agList()->begin();

	for ( int i=0; agi != agList()->end(); agi++, i++)
		if ((*agi) == ag)
			return(i);
	return -1;
}

//
//	for getting & resetting AG counts
//
int
AgList::getCounts( vector<int> *cnts)
{
	cnts->clear();
	for ( std::list<AnalysisGr *>::iterator ag = agList()->begin(); ag != agList()->end(); ag++)
	{
		cnts->push_back( (*ag)->count());
	}
	return cnts->size();
}

//	assumes cnts in same order as agList
void
AgList::setCounts( vector<int> *cnts)
{
	vector<int>::iterator ci = cnts->begin();
	for ( std::list<AnalysisGr *>::iterator ag = agList()->begin();
			ag != agList()->end() && ci != cnts->end(); ag++, ci++)
	{
		(*ag)->count() = *ci;
	}
}

//----------------------------------------------------------------------
//  virtual routines
//----------------------------------------------------------------------
bool
AgList::read( ifstream &inFile, int hdr_ver)
{
	int n, nag;
	AnalysisGr *ag;
	AgListReadErr Err = noErr;
	string str, tmp;
	istringstream iss;

	free_all();

	// Check that _AModes not 0.
	// When used with Glas, GlasAnalyze ctor calls AModes()->read() and keeps AModeList.
	// But if not used with Glas, AgList may need to read AModes itself.
	if ( AModes() == 0)
	{
		_AModes = new AModeList();
		_freeAModes = true;		// our responsibility to delete
		if ( !AModes()->read( str))
		{
			_errStr = str;
			return false;
		}
	}

	// check that have some AModes:
	if ( AModes()->size() == 0)
	{
		_errStr = "No analysis modes read, skipping Analysis Groups.";
		return false;
	}

  try {
	switch (hdr_ver)
	{
		case 5:
		case 6:
			getline( inFile, str);
			iss.str( str);
			iss >> nag;
			for ( n=0; n < nag; n++)
			{
				ag = new AnalysisGr( this);
				if (ag->read( inFile, _version, hdr_ver))
				{
					if ( ag->setAMode( AModes()->amode( ag->modeName())))
					{
						ag->amode()->updateControls( ag);
						agList()->push_back(ag);
					}
					else
					{
						setError( ag->getError());
//						throw unknModeErr;	// no error need to check _errStr higher up.
					}
				}
				else
				{
					setError( ag->getError());
					delete ag;
					throw readAgErr;
				}
			}
			break;
		case 7:
		case 8:
		case 9:
		case 10:
			do
			{
				getline( inFile, str);					// read version
			} while (str.length() == 0);
			iss.str( str);
			iss >> _version;
			switch ( _version)
			{
				case 0:
				case 1:
				case 2:
				case 3:
					getline( inFile, str);
					iss.clear(); iss.str( str);
					iss >> nag;
					for ( n=0; n < nag; n++)
					{
						ag = new AnalysisGr( this);
						if ( ag->read( inFile, _version, hdr_ver) )
						{
							if ( ag->setAMode( AModes()->amode( ag->modeName())))
							{
								ag->amode()->updateControls( ag);
								agList()->push_back(ag);
							}
							else
							{
								setError( ag->getError());
//								throw unknModeErr;	// no error need to check _errStr higher up.
							}
						}
						else
						{
							setError( ag->getError());
							delete ag;
							throw readAgErr;
						}
					}
					break;
				case 4:
				case 5:
					getline( inFile, str);
					iss.clear(); iss.str( str);
					iss >> tmp >> tmp >> nag;				// swallow "Analysis Groups:"
					for ( n=0; n < nag; n++)
					{
						ag = new AnalysisGr( this);
						if ( ag->read( inFile, _version, hdr_ver) )
						{
							if ( ag->setAMode( AModes()->amode( ag->modeName())))
							{
								if ( _version < 5)
									ag->amode()->updateControls( ag);
								agList()->push_back(ag);
							}
							else
							{
								setError( ag->getError());
//								throw unknModeErr;	// no error need to check _errStr higher up.
							}
						}
						else
						{
							setError( ag->getError());
							delete ag;
							throw readAgErr;
						}
					}
					break;
				default:
					return false;
			}
			break;
		default:
			return false;
	}
  }
  catch ( AgListReadErr Err)
  {
	string str = "Reading Analysis Group ";
	str = str + ag->name() + ": ";

	switch ( Err)
	{
		case unknModeErr:
		case readAgErr:
			str = str + getError();
			break;
		default:
			str = str + " unknown error";
			break;
	}
	setError( str);
	delete ag;
	return false;
  }

	return true;
}	// read()

bool
AgList::write( ofstream &outFile)
{
	std::list<AnalysisGr *>::iterator ag;

	outFile << CUR_AG_VERSION << endl;
	outFile << "Analysis Groups: " << size() << endl;
	for ( ag=agList()->begin(); ag != agList()->end(); ag++)
		(*ag)->write( outFile);
	outFile << endl;
	
	return true;
}
