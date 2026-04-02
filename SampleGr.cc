//
//	SampleGr.cc - routines to store and manipulate sample groups
//
//	Function:
//	    Hold array of physical channels to acquire / acquired.  Values in
//	    chan[] are physical chan numbers (0 = 1st), so index of chan[] is
//	    position of chan sample value in raw data.  It is assumed that data
//	    is acquired (scanned) in order of channels in chan[].
//	    Also stores sample rate (HZ) and group name.  'mode' member intended
//	    for use to indicate mode of data acquistion, eg. triggered start etc
//
//	01-jun-92	jwp	pulled out of glas
//	Version: 1.3
//	Modified:
//	07-nov-01  bhb	removed SgList::edit()
//	13-jun-07  bhb	SgList no longer has list base class
//
#include "SampleGr.h"
#include "DisplayGr.h"
#include <iostream>
#include <sstream>

using namespace std;			// don't know why doesn't seem to apply below

//*****************************************************************************
//
//  SampleGr class member functions
//
//*****************************************************************************
SampleGr::SampleGr( SgList * p) : ChannelSet(CS_TYPE_SG), _parent(p)
{
	setName( "New Sg");
	setRate( 1000);
}

SampleGr::SampleGr( const SampleGr &sg) : ChannelSet( sg)
{
	_rate = sg.rate();
	// _changed & _parent (SgList) not copied
	// caller must manage their init.
}

SampleGr::~SampleGr()
{
	// Empty
}

//  get our own index in parent group
//
int
SampleGr::index()
{
	if ( Parent() != NULL)
	{
		SgList *sglist = Parent();
		list<SampleGr *>::iterator sg = sglist->sgList()->begin();

		for (int i=0; sg != sglist->sgList()->end(); sg++, i++)
			if ((*sg) == this)
				return i;
	}
	return -1;
}

bool
SampleGr::read( ifstream &inFile, int sg_ver, int hdr_ver)
{
	int nchan;
	string str;
	istringstream iss;
		
	switch (sg_ver)
	{
		case 0:
			getline( inFile, str);
			iss.str( str);
			iss >> _name >> _mode >> _rate >> nchan;
			getline( inFile, str);
			nchan = gtchan_0( str.c_str());
			initDefNames(nchan);
			initDefUnits(nchan);
			break;
		case 1:
			getline( inFile, str);
			_name = str;
			getline( inFile, str);
			iss.str( str);
			iss >> _mode >> _rate >> nchan;
			getline( inFile, str);
			nchan = gtchan_0( str.c_str());
			initDefNames(nchan);
			initDefUnits(nchan);
			break;
		case 2:
			getline( inFile, str);
			_name = str;
			getline( inFile, str);
			iss.str( str);
			iss >> _mode >> _rate >> nchan;
			getline( inFile, str);
			nchan = gtchan_0( str.c_str());
			if ( nchan <=0)
			{
				cerr << "SampleGr::read: error getting nchan" << endl;
				return false;
			}
			rd_chnames( inFile, nchan);
			rd_units( inFile, nchan);
			getline( inFile, str);
			dgList()->read( inFile, hdr_ver);
			for ( list<DisplayGr *>::iterator dg = dgList()->begin(); dg != dgList()->end(); 
					dg++)
				(*dg)->setParent(this);
			break;
		default:
			return false;
	}

	if ( dgList()->size() > 0)
		setCurDg( *dgList()->begin());		// need a _curDg set
	return true;
}

bool
SampleGr::write( ofstream &outFile)
{
	char str[2048];

	outFile << name() << endl;
	outFile << mode() << " " << rate() << " " << nChan() << endl;
	outFile << numstr( str) << endl;
	wr_chnames( outFile, nChan());
	wr_units( outFile, nChan());
	outFile << endl;
	dgList()->write( outFile);
	return true;
}

int
SampleGr::edit()
{
	// Empty
	return 1;
}

void
SampleGr::freeAll()
{
	ChannelSet::freeAll();
}

// copy operator
SampleGr&
SampleGr::operator = ( const SampleGr& sg)
{
	ChannelSet::operator=(sg);		// copy ChannelSet part

	setRate( sg.rate());
	return *this;
}

//////////////////////////////////////////////////////////
//////////	SgList     //////////////////////////////
//////////////////////////////////////////////////////////

SgList::SgList( DataHeader *hdr) : _parent(hdr)
{

}

SgList::SgList( const SgList &sgl)
{
	_list = sgl._list;
}

SgList::~SgList()
{
	free_all();
}

void
SgList::free_all()
{
	for ( std::list<SampleGr *>::iterator i = _list.begin(); i != _list.end(); i++)
		delete (*i);
	_list.clear();
}

SampleGr *
SgList::elem( int i)
{
	std::list<SampleGr *>::iterator g;

	if ( unsigned(i) < size())
	{
		for ( g = sgList()->begin(); i > 0; i--, g++);
		return( *g);
	}
	else return NULL;
}

//----------------------------------------------------------------------
//  virtual routines
//----------------------------------------------------------------------
bool
SgList::read( ifstream &inFile, int hdr_ver)
{
	int nsg, sg_ver, n;
	SampleGr *sg;
	string str, tmp;
	istringstream iss;
	
	free_all();

	switch (hdr_ver)
	{
		case 5:
		case 6:
			getline( inFile, str);
			iss.str( str);
			iss >> nsg;
			if ( nsg == 0)
				return 0;
			for ( n=0; n < nsg; n++)
			{
				sg = new SampleGr( this);
				if ( sg->read( inFile, 0, hdr_ver))
					sgList()->push_back(sg);
				else
				{
					delete sg;
					return false;
				}
			}
			break;
		case 7:
		case 8:
		case 9:
		case 10:
			getline( inFile, str);
			iss.str( str);
			iss >> sg_ver;
			switch (sg_ver)
			{
				case 1:
					getline( inFile, str);
					iss.clear(); iss.str( str);
					iss >> nsg;
					if ( nsg == 0)
						return false;
					for ( n=0; n < nsg ; n++)
					{
						sg = new SampleGr( this);
						if ( sg->read( inFile, sg_ver, hdr_ver))
							sgList()->push_back(sg);
						else
						{
							delete sg;
							return false;
						}
					}
					break;
				case 2:
					getline( inFile, str);
					iss.clear(); iss.str( str);
					iss >> tmp >> tmp >> nsg;			// swallow "Sample Groups:"
					if ( nsg == 0)
						return false;
					for ( n=0; n < nsg; n++)
					{
						sg = new SampleGr( this);
						if ( sg->read( inFile, sg_ver, hdr_ver))
							sgList()->push_back(sg);
						else
						{
							delete sg;
							return false;
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
	return true;
}

bool
SgList::write( ofstream &outFile)
{
	outFile << CUR_SG_VERSION << endl;
	outFile << "Sample Groups: " << size() << endl;

	for ( std::list<SampleGr *>::iterator sg = sgList()->begin(); sg != sgList()->end(); sg++)
		(*sg)->write( outFile);

	return true;
}
