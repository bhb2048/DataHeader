//*****************************************************************************
//	Copywrite (C) 1992-2006 Barry H. Branham.  All rights reserved.
//*****************************************************************************
//
//	AdLog.cc -- log class functions, no GUI stuff
//
//	01-apr-92  xy version 2.0 for SGI
//	Version 3.2
//	Modified:
//	20-aug-92  jwp	put in data obj.
//	01-dec-92  bhb	added set/get fcns.
//	02-dec-92  bhb	added get_run_for_stime(), made Runs[] static and moved all 
//					fcns that reference it to this file.
//	04-dec-92  bhb	added init_lf_form(), etc
//	07-dec-92  bhb  added wrt_log_file()
//	08-dec-92  bhb	changed name from rdlogfltext.c to log.c
//	23-nov-93  bhb	C++, use DM->functions
//	14-mar-94  bhb	added page up
//	26-jun-95  bhb	V3.0 created AdLog class, log.c++ now AdLog.c++
//	20-jul-95  bhb	added AdRun class
//	29-aug-96  bhb	redid end of block kludge to fix bug
//	24-oct-96  bhb	made reading log file more robust, added error output
//					using message()
//	07-nov-06  bhb	remove #define LOGSIZE 50, change AdLog/AdRun member vars to _XXX
//					make _Runs a vector<AdRun *>, remove 'int _numRuns'
//	02-apr-06  bhb	add AdRun::read()
//	25-sep-12  bhb	Version 2, add length (msec) to log file
//
#include "AdLog.h"
#include "DataHeader.h"
#include "SampleGr.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <CXlib/cxlib.h>				// Define date,time vectors
#include <CXlib/TimeVec.h>
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace bhb;

//#define DEBUG

extern int errno;

//----------------------------------------------------------------------
//  AdRun class
//----------------------------------------------------------------------
AdRun::AdRun() : _length(0), _changed(0)
{
}

AdRun::AdRun( const AdRun &r)
{
	_sampGr = r._sampGr;
	_gainFileNum = r._gainFileNum;
	_blkNum = r._blkNum;
	_start = r._start;
	_length = r._length;
	setComment( r._comment.c_str());
	_changed = 0;
}

AdRun::AdRun( int sg, int gfn, int blk, int st, int len, const char *cmt) : _sampGr(sg), 
	_gainFileNum(gfn), _blkNum(blk), _start(st), _length(len), _changed(0)
{
	setComment( cmt);
}

void AdRun::setComment( const char *cmt)
{
	_comment = cmt;
	_changed++;
}

void AdRun::write( ofstream &outFile, int index)
{
	TimeVec tv( Start());
	char timStr[32];

	tv.timeStr( timStr);
	outFile << setw(3) << index+1 << "   " << _sampGr << "   " << setw(5) << _blkNum << "    "
	<< timStr << " " << setw(6) << _length << "   " <<_gainFileNum << endl;

	outFile << _comment << endl;
	_changed = 0;
}

bool AdRun::read( ifstream &inFile, int i, int &lastblk, int version)
{
	int n, sg, blk, st, h, m, s, ms, gfn, scans, len = 0;
	string cmt, str;
	int nscan;
	
	getline( inFile, str);
	if ( version == 2)
	{
		scans = sscanf( str.c_str(), "%d %d %d %02d:%02d:%02d.%03d %d %d",
			&n, &sg, &blk, &h, &m, &s, &ms, &len, &gfn);
		nscan = 9;
	}
	else
	{
		scans = sscanf( str.c_str(), "%d %d %d %02d:%02d:%02d.%03d %d",
			&n, &sg, &blk, &h, &m, &s, &ms, &gfn);
		nscan = 8;
	}

	// check data just read
	if ( scans != nscan || (n != i+1) || sg < 0 || blk <= lastblk)
	{
		return false;
	}
	else
	{
		_sampGr = short(sg);
		_gainFileNum = short(gfn);
		_blkNum = blk;
		_start = ( h * 3600 + m * 60 + s) * 1000 + ms;	// get run start time
		_length = len;

		// get optional comment - read first char of next line
		streampos mark = inFile.tellg();
		getline( inFile, cmt);
		if ( str[0] != '\n')						// newline
		{
			setComment( cmt);
		}
		else
			inFile.seekg( mark);
		lastblk = blk;
	}

	return true;
}

//  assignment operator
AdRun& AdRun::operator = (const AdRun& r)
{
	_sampGr = r._sampGr;
	_gainFileNum = r._gainFileNum;
	_blkNum = r._blkNum;
	_start = r._start;
	_length = r._length;
	setComment( r._comment.c_str());
	_changed = ((AdRun)r).changed();	// to fix discard const warning
	return *this;
}

//----------------------------------------------------------------------
//  AdLog class
//----------------------------------------------------------------------
AdLog::AdLog() : _fnum(0)
{
	_notifyCmd = new NotifyCmd;
	addCommand( _notifyCmd, ShowNotifyEvent);
}

AdLog::AdLog( int fn) : _fnum(fn)
{
	_notifyCmd = new NotifyCmd;
	addCommand( _notifyCmd, ShowNotifyEvent);
}

AdLog::AdLog( const AdLog &l)
{
	_fnum = l._fnum;
	for ( int r=0; r<l.numRuns(); r++)
		_Runs.push_back( new AdRun( *l._Runs[r]));
	_notifyCmd = new NotifyCmd;
	addCommand( _notifyCmd, ShowNotifyEvent);
}

AdLog::~AdLog()
{
	for ( int i=0; i<numRuns(); i++)
		delete _Runs[i];
	delete _notifyCmd;
}

void
AdLog::freeAll()
{
	for ( int i=0; i<numRuns(); i++) delete _Runs[i];
	_Runs.clear();
}

AdRun *
AdLog::newRun()
{
	AdRun *r = new AdRun();
	_Runs.push_back( r);
	return r;
}

//
//  get run for given start time in current data file
//
int
AdLog::get_run_for_stime( int stime )			// time in msec past midnite
{
	int i;
	int nruns = num_runs();
	int start, end;
	int diff = 24*3600*1000;					// msec in day
	int closest = -1;
	int tmp;

	if ( stime < get_run_stime(0))
	{
		notifyCmd()->setLabel( "This start time is earlier than this data file.");
		invokeEvent( ShowNotifyEvent);
		return -1;
	}

	if ( stime > (get_run_stime(nruns-1) + get_run_length(nruns-1)))
	{
		notifyCmd()->setLabel( "This start time is later than this data file.");
		invokeEvent( ShowNotifyEvent);
		return -1;
	}
	for ( i=0; i<nruns; i++)
	{
		if ( stime >= (start=get_run_stime(i))
			&& stime <= (end=start+get_run_length(i)))
			return(i);
		if ( (tmp = abs( stime - start)) < diff)
		{
			diff = tmp;
			closest = i;
		}
	}
	// not in any run
	string str = "The closest run is ";
	char num[8];
	sprintf( num, "#%d", closest+1);
	str += num;
	notifyCmd()->setLabel( str);
	invokeEvent( ShowNotifyEvent);
	return( closest);
}

//*****************************************************************************
//  log file read/write routines
//*****************************************************************************
//
//	Return 1 if read OK, 0 if not.
//	
int
AdLog::read_log_file( const char *prefix,
					const int nlf,			// number of this log file (first = 0)
					DataHeader *Hdr)
{
	int version;
	//
	// To set run length in msec, this routine needs to know each sample group
	// to get nchan & sample rate.  It also need total blocks in data file to
	// set last run length.  A pointer to the header would do it.
	//

	if ( readFile( prefix, nlf, version))
	{
		if ( !checkRuns( Hdr->numSgs()))
		{
			string str = "Error reading log file, contains non-existant sample group";
			notifyCmd()->setLabel( str);
			invokeEvent( ShowNotifyEvent);
			return 0;
		}
		if ( version < 2)
			set_run_lengths( Hdr);
		return 1;
	}
	else
		return 0;
}

int
AdLog::readFile( const char * prefix,
				const int    nlf,
				int &version)			// number of this log file (first = 0)
{
	int num_le;
	char log_file[128];
	ifstream inFile;
	
	sprintf( log_file, "%s%d.log", prefix, nlf+1);
	
	inFile.open( log_file);
	if ( inFile)
	{
		string line;
		int lastblk = -1;

		getline( inFile, line);
		// is this version 2?
		if ( line.find( "Version") != string::npos)
		{
			version = 2;			// only version so far
			getline( inFile, line);	// get next line
		}
		else
			version = 0;
		if ( !isdigit( (int)line[0]))			// check first line
			getline( inFile, line);				// first line was probably 'name.tlg'
		if ( sscanf( line.c_str(), "%d RUNS\n", &num_le) != 1)
		{
			string str = "Error reading number of runs in log file: ";
			str += log_file;
			notifyCmd()->setLabel( str);
			invokeEvent( ShowNotifyEvent);
			inFile.close();
			return 0;
		}

		freeAll();
		getline( inFile, line);		// "RUN GROUP BLOCK    START      GAIN FILE\n");
		for ( int i=0; i<num_le; i++)
		{
			AdRun *run = new AdRun();

			if ( run->read( inFile, i, lastblk, version))
				_Runs.push_back( run);
			else
			{
				string str = "Error reading log file at run ";
				char num[8];
				sprintf( num, "%d", i+1);
				str += num;
				notifyCmd()->setLabel( str);
				invokeEvent( ShowNotifyEvent);
				delete run;
			}
		}
		inFile.close();
		_fnum = nlf;								// remember our file number
	}
	else
	{
		string str = "Error - log file: ";
		str += log_file;
		str += "\n--- ";
		str +=  strerror( errno);
		notifyCmd()->setLabel( str);
		invokeEvent( ShowNotifyEvent);
		return 0;
	}

	return numRuns();
}

void
AdLog::set_run_lengths( DataHeader *Hdr)
{
	SampleGr *SG;
	int nchan, blks, scans;

	// set run lengths
	for ( int i=0; i<numRuns(); i++)
	{
		SG = Hdr->sg( _Runs[i]->_sampGr);

		// get num blocks
		if (i == numRuns()-1)
			blks = Hdr->nBlocks( _fnum) - _Runs[i]->_blkNum;
		else
			blks = _Runs[i+1]->_blkNum-_Runs[i]->_blkNum;

		// get num scans
		nchan = SG->nChan();
		//
		// Kludge: to prevent possible bad data after last scan in block
		// set #scans so only one scan extends into last block
		//
		scans = ((blks-1) * 256) / nchan;			// scans always an integer
		// now add enough scans to extend into last block
		scans += (((blks-1) * 256 - scans * nchan) / nchan) + 1;

		// run length in msec
		_Runs[i]->_length = (scans*1000) / SG->rate();
	}
}

bool
AdLog::wrt_log_file( const char *prefix, const int nlf)
{
	char log_file[128];
	ofstream outFile;
	int i;

	sprintf( log_file, "%s%d.log", prefix, nlf+1);

	outFile.open( log_file);
	if ( outFile)
	{
		outFile << "Version 2" << endl;
		outFile << num_runs() << " RUNS" << endl;
		outFile << "RUN GROUP BLOCK    START        LENGTH  GAIN FILE" << endl;
		for (i=0; i < num_runs(); i++)
		{
			_Runs[i]->write( outFile, i);
		}
		outFile.close();
		return true;
	}
	else
	{
		string str = "Error writing log file: ";
		str += log_file;
		str += "\n--- ";
		str += strerror( errno);
		notifyCmd()->setLabel( str);
		invokeEvent( ShowNotifyEvent);
		return false;
	}
}

bool
AdLog::checkRuns( int nsg)
{
	for ( int i=0; i<numRuns(); i++)
	{
		if ( _Runs[i]->Sgr() >= nsg)
			return false;
	}
	return true;
}

//  assignment operator
AdLog& AdLog::operator = (const AdLog& l)
{
	_fnum = l._fnum;
	// delete any existing runs
	freeAll();
	for ( int r=0; r<l.numRuns(); r++)
		_Runs.push_back( new AdRun( *l._Runs[r]));
	return *this;
}
