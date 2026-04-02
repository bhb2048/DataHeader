//
//	RtAnalysisGr.cc - real time analysis for Dash
//	
//	21-feb-13  bhb
//	Modified:
//	
#include "RtAnalysisGr.h"
#include "DataHeader.h"			// for Parent()->Parent()->
#include <string>
#include <sstream>

using namespace std;

RtAnalysisGr::RtAnalysisGr( RtAgList * p) : _parent(p), _type(NONE), _minCycle(400)
{
	setName( "New RtAg");
	
}

//	Copy ctor
RtAnalysisGr::RtAnalysisGr( const RtAnalysisGr &ag) : ChannelSet(ag)
{
	_parent = ag._parent;			// AgList
	_type = ag._type;
	_minCycle = ag._minCycle;
}

bool RtAnalysisGr::read( std::ifstream &inFile)
{
	int nchan;
	string	str, tmp;
	istringstream iss;

	getline( inFile, str);
	_name = str;
	getline( inFile, str);
	iss.str( str);
	iss >> nchan;
	getline( inFile, str);
	nchan = gtchan_0( str.c_str());
	getline( inFile, str);
	iss.clear(); iss.str( str);
	iss >> nchan;
	_type = RTA_Type(nchan);
	getline( inFile, str);
	iss.clear(); iss.str( str);
	iss >> _minCycle;
	
	return true;
}

bool RtAnalysisGr::write( std::ofstream &outFile)
{
	char string[256];

	outFile << name() << endl;
	outFile << nChan() << endl;
	outFile << numstr( string) << endl;
	outFile << type() << endl;
	outFile << minCycle() << endl;

	return true;
}

//*****************************************************************************
//
//	RtAgList
//	
//*****************************************************************************
void
RtAgList::freeAll()
{
	for ( std::list<RtAnalysisGr *>::iterator i = List()->begin(); i != List()->end(); i++)
		delete (*i);
	List()->clear();
}

RtAnalysisGr *
RtAgList::elem( int i)
{
	std::list<RtAnalysisGr *>::iterator g;
	if ( unsigned(i) < size())
	{
		for ( g = List()->begin(); i > 0; i--, g++);
		return( *g);
	}
	else return 0;
}

bool RtAgList::read( std::ifstream &inFile)
{
	int nag, ag_ver;
	RtAnalysisGr *ag;
	string str, tmp;
	istringstream iss;

	freeAll();

	do
	{
		getline( inFile, str);					// read version
	} while (str.length() == 0);
	iss.str( str);
	iss >> ag_ver;

	getline( inFile, str);
	iss.clear(); iss.str( str);
	iss >> tmp >> tmp >> tmp >> nag;		// swallow "RT Analysis Groups:"
	for ( int n=0; n < nag; n++)
	{
		ag = new RtAnalysisGr( this);
		if ( ag->read( inFile) )
		{
			List()->push_back(ag);
		}
		else
		{
			delete ag;
			return false;			// position in file unknown can't continue
		}
	}
	return true;
}

bool RtAgList::write( std::ofstream &outFile)
{
	std::list<RtAnalysisGr *>::iterator ag;

	outFile << CUR_RTA_VERSION << endl;
	outFile << "RT Analysis Groups: " << size() << endl;
	for ( ag=List()->begin(); ag != List()->end(); ag++)
		(*ag)->write( outFile);
	
	return true;
}

//
//	protected functions
//	
