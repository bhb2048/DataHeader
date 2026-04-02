//*****************************************************************************
//	Copywrite (C) 1992-2005 Barry H. Branham.  All rights reserved.
//*****************************************************************************
//
//	DataHeader.cc - container for SG AG DG LOG files
//
//	3-jun-92  jwp	from glas.h
//	Modified:
//	07-dec-92  bhb	added millisecond to time_v, changed from short to int
//	10-dec-92  bhb	moved DG_... #defines to display.h
//	11-aug-94  jwp	now just SG AG DG
//	18-feb-94  bhb	rename file from newglas.h to glas.h
//	26-jan-01  bhb	renamed from glas.h to GlasHeader.h, fixed up for Linux port
//	03-feb-01  bhb	move EditSgDlog, EditDgDlog to SgListVk
//	17-jul-02  bhb	put EditDgDlog here instead of with GlasDgList
//					so not so many copies
//	24-sep-02  bhb	removed _editDgDlog
//	13-nov-03  bhb	in readSet() check 2nd line for "SETUP" incase have extra lf/nl
//					at top, as previously in readHdr()
//	09-may-05  bhb	fixup readSet so check Sg & Ag list read errors
//	04-nov-05  bhb	moved incNumDataFiles(), incNumGainFiles(), incBlocks() here from
//					HeaderVk, changed _studyTitle, _studyDate to string
//					_numBlocks now vector<int>, numDataFiles() now _numBlocks.size()
//	09-nov-05  bhb	have merged Vk groups w/ reg SG,AG,DG, so copied .set, .hdr read/
//					write funcs from HeaderVk, deleted from HeaderVk
//	02-apr-07  bhb	modify setFilename() so doesn't change filename arg.
//	24-jul-08  bhb	setFilename() now uses string arg
//	25-sep-08  bhb	add _rawbits, bump CUR_HEADER_VERSION to 9
//	26-sep-08  bhb	convert to c++ style file i/o
//	14-may-13  bhb	add _notifyCmd, addCommand() in ctor
//	08-dec-15  bhb	delete _notifyCmd, don't want GUI dependence here
//	
#include "DataHeader.h"
#include "DisplayGr.h"
#include <CXlib/cxlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>			// these for stat
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

using namespace std;
using namespace bhb;

DataHeader::DataHeader() : _filename("NO FILE"), _studyTitle("NO TITLE"),
		_studyDate("NO DATE"), _sampGroups(0), _numGainFiles(0), _hdrFileTime(0), _databits(12)
{
	_sampGroups = new SgList;
}

DataHeader::~DataHeader()
{
	if (_sampGroups)
		delete _sampGroups;
}

//
//  set header file name (prefix only) with path
//  return file num if present.
//  ???Problems here if # data files > 9.
//  stripDfNum default is false.
//
int
DataHeader::setFilename( const string &filename, bool stripDfNum)
{
	int filenum = 0;

	_filename = filename;
	if ( filename.length())
	{
		string::size_type index = _filename.find_last_of( '.');
		if ( index != string::npos)
		{
			if ( stripDfNum)
			{
				index--;
				if ( isdigit( _filename[index]))
					filenum = _filename[index] - '0';
				else
					index++;
			}
			_filename.erase( index, _filename.length()-index);
		}
	}

	return filenum;
}

void
DataHeader::setDate()
{
	char date[16];
	long clock;
	struct tm *tod;

	time(&clock);
	tod = localtime(&clock);
//	sprintf( date, "%02d/%02d/%02d", tod->tm_mon+1, tod->tm_mday, tod->tm_year-100);
	strftime( date, 16, "%D", tod);			// see strftime(3) man page
	_studyDate = date;
}

int
DataHeader::nBlocks(int file)
{
	if (file < nDataFiles())
		return(_numBlocks[file]);
	else
		return 0;
}

void
DataHeader::setBlocks( int *blks, int n)
{
	_numBlocks.clear();

	for ( int i=0; i<n; i++)
		_numBlocks.push_back( blks[i]);
}

////////////////////////////////////////////
////	    header & setup files		////
////////////////////////////////////////////

bool
DataHeader::readSet( const string &filename)
{
	int		version;
	bool	ret = false;
	ifstream inFile;
	string	str;
	istringstream iss;

	inFile.open( filename.c_str());
	if ( inFile)
	{
		// read & check first line
		getline( inFile, str);
		if ( str.length() < 6)	// old header w/ form feed at top, read next line
		{
			getline( inFile, str);
		}
		if (str.find( "SETUP") == string::npos)
		{
			return false;
		}
		// swallow newlines till get version
		do
		{
			getline( inFile, str);
		} while ( str.length() == 0);

		iss.str( str);
		iss >> version;

		switch ( version)
		{
			case 8:							// current and only Setup version 8-11-94
			case 9:
				if ( haveData())
					clear_header();			// allows loading setup if new dataset
				if ( sgList()->read( inFile, version))
				{
					ret = agList()->read( inFile, version);
					if ( !ret)
						_errStr = agList()->getError();
				}
				else
					ret = false;
				if ( ret && version == 9)
				{
					ret = rtAgList()->read( inFile);
				}
				break;

			default:
				break;
		}
		inFile.close();
		return ret;
	}
	else
	{
		_errStr = "Error opening " + filename;
		return false;
	}
}

bool
DataHeader::writeSet( const string &filename)
{
	ofstream outFile;
	string fname;
	struct stat buf;

	//
	//	make sure has .set ext
	//
	string::size_type sp = filename.find( ".set");
	if ( sp == string::npos || sp < filename.length()-4)
		fname = filename + ".set";
	else
		fname = filename;

	//
	//	make a backup copy if file exists
	//
	if ( stat( fname.c_str(), &buf) == 0)
	{
		string cmd = "cp " + fname + " " + fname + ".bak";
		system( cmd.c_str());
	}

	outFile.open( fname.c_str());

	if (outFile)
	{
		outFile << "\t\t*** GLAS SETUP FILE ***\n\n\n";
		outFile << CUR_SETUP_VERSION << endl;

		sgList()->write( outFile);
		agList()->write( outFile);
		rtAgList()->write( outFile);
		
		outFile.close();

		return true;
	}
	else
	{
		_errStr = "Error opening " + filename;
		return false;
	}
}

//
//	Read a header file.
//
bool
DataHeader::readHdr( const string &filename, AModeList *aml)
{
	ifstream inFile;

	inFile.open( filename.c_str());
	if ( inFile)
	{
		setFilename( filename);
		bool ret = readHdr( inFile, aml);
		inFile.close();
		return ret;
	}
	else
	{
		_errStr = "Error opening " + filename;
		return false;
	}
}

bool
DataHeader::readHdr( ifstream &inFile, AModeList *aml)
{
	string	str;
	istringstream iss;
	int 	numDataFiles, nblks;
	int 	i, n;
	string	Ch_name[NCHANNAMES];
	DgList  *dgs;
	bool	ret = false;
	
	clear_header();

	if ( !agList()->setAModes( aml))
	{
		_errStr = agList()->getError();
		return false;
	}
	
	char c = inFile.get();		// check for '\f' in case old header file
	if ( c == '\f')
		c = inFile.get();		// read '\r' in case old header file

	// read & check first line
	getline( inFile, str);
	if ( str.length() < 6)	// old header w/ form feed at top, read next line
	{
		getline( inFile, str);
	}
	if ( str.find( "HEADER") == string::npos)
	{
		_errStr = "Bad header file";
		return false;			// bad header file
	}
	// swallow newlines till get title or date
	do
	{
		getline( inFile, str);
	} while ( str.length() == 0);

//	str[ str.length()-1] = '\0';			// erase newline char

	if ( !( str[2] == '/' && str[5] == '/'))
	{									// if not date,
		setTitle( str);					// assume user entered title
		getline( inFile, str);			// next line should be date
	}
	else						// must be date, make title out of filename
	{
		string title = _filename;
		string::size_type sp = title.rfind( '/');		// eliminate path prefix
		if ( sp != string::npos)
		{
			title.erase( 0, sp);
		}
		sp = title.find( ".hdr");		// get rid of '.hdr'
		title.erase( sp, title.length()-sp);
		setTitle( title);
	}
	setDate( str);

	getline( inFile, str);				// version
	iss.str( str);
	iss >> _version;
	if ( _version >= 9)
	{
		getline( inFile, str);
		iss.clear(); iss.str( str);
		iss >> _databits;
	}
	else
		databits( 12);
	getline( inFile, str);				// read num Data files
	iss.clear(); iss.str( str);
	iss >> numDataFiles;
	getline( inFile, str);				// read DataFile lengths (blocks)
	iss.clear(); iss.str( str);
	for ( i=0; i < numDataFiles ; i++)
	{
		iss >> nblks;
		_numBlocks.push_back( nblks);
	}
	getline( inFile, str);				// read num Gain files
	iss.clear(); iss.str( str);
	iss >> _numGainFiles;

	// read SampleGr list
	switch ( _version)
	{
		case 5:
		case 6:
		case 7:
			for ( i = n = 0; i < 4; i++)
			{
				getline( inFile, str);
				iss.clear(); iss.str( str);
				for ( int j=0; j < 16; j++)
					iss >> Ch_name[n++];
			}
			sgList()->read( inFile, _version);
			// set chan names for SG's
			for ( i=0; i < (int)sgList()->size(); i++)
			{
				 SampleGr *sg = sgList()->elem(i);
				 int nNames = sg->nChan();
				 if ( nNames > NCHANNAMES)
					nNames = NCHANNAMES;
				 for ( int j=0; j < nNames; j++)
					sg->setChName( j, Ch_name[j].c_str());
			}
			dgs = new DgList();
			i = dgs->read( inFile, _version);
			ret = agList()->read( inFile, _version);
			if ( !ret)
				_errStr = agList()->getError();

			for ( list<DisplayGr *>::iterator dg=dgs->begin(); dg != dgs->end(); dg++)
			{
				int sgnum = (*dg)->grid();	// after read() holds sg#
				(*dg)->setGrid( 0);
				SampleGr *sg = sgList()->elem(sgnum);
				sg->addDg( *dg);
				(*dg)->setParentList( sg->dgList());
			}

			dgs->dglist()->clear();	// erase first so don't delete dg's with free_all()
			delete dgs;
			break;

		case 8:
		case 9:
		case 10:
			if ( sgList()->read( inFile, _version))
			{
				ret = agList()->read( inFile, _version);
				if ( !ret)
					_errStr = agList()->getError();
			}
			else
			{
				_errStr = "Error reading sample groups";
				ret = false;
			}
			if ( ret && _version == 10)
			{
				ret = rtAgList()->read( inFile);
				if ( !ret)
					_errStr = "Error reading Real Time Ag list";
			}
			return ret;
	}
	return true;
}

bool
DataHeader::writeHdr( const string &filename)
{
	ofstream outFile;
	string fname;
	struct stat buf;

	//
	//	Make sure has .hdr ext
	//
	string::size_type sp = filename.find( ".hdr");
	if ( sp == string::npos || sp < filename.length()-4)
		fname = filename + ".hdr";
	else
		fname = filename;

	//
	//	Make a backup copy if file exists.
	//	Use quotes incase name has spaces.
	//
	if ( stat( fname.c_str(), &buf) == 0)
	{
		string cmd = "cp '" + fname + "' '" + fname + ".bak'";
		system( cmd.c_str());
	}

	outFile.open( fname.c_str());

	if (outFile)
	{
		bool ret = writeHdr( outFile);
		outFile.close();
		return ret;
	}
	else
	{
		_errStr = "Error opening " + filename;
		return false;
	}
}

bool
DataHeader::writeHdr( ofstream &outFile)
{
	outFile << "\t\t*** HEADER FILE ***\n\n\n";
	outFile << title() << endl;
	outFile << date() << endl;
	outFile << CUR_HEADER_VERSION << endl;
	outFile << databits() << endl;
	outFile << nDataFiles() << endl;
	for ( int i=0; i < nDataFiles(); i++)
		outFile << nBlocks(i) << " ";
	outFile << endl;
	outFile << nGainFiles() << endl;

	sgList()->write( outFile);
	agList()->write( outFile);
	rtAgList()->write( outFile);
	return true;
}

void
DataHeader::clear_header()
{
	_studyTitle = "NO TITLE";
	_studyDate = "NO DATE";

	sgList()->free_all();
	agList()->free_all();
	rtAgList()->freeAll();
	
	_numGainFiles = 0;
	_numBlocks.clear();
}

bool
DataHeader::haveAgError()
{
	for ( uint i=0; i < numAgs(); i++)
	{
		string err = Ag(i)->getError();
		if ( err.length() > 0)
			return true;
	}
	return false;
}

//
//	strip the data file number and extention from a raw data filename path
//	Complicated by fact that may have > 9 .dat files and no special way
//	of indicating file number in name, just appending number to prefix.
//	Algorithm for getting correct dataset prefix from .dat file name:
//
//	1. use current algorithm to create prefix:
//		0. copy .dat filename
//		1. find last '.' if any
//		2. if found set '.' position - 1 to '\0'
//			else return false
//	2. check for file existance, if exists return true
//	3. create prefix assuming 2 digit file number
//	4. check for file existance, if exists return true
//	5. get directory name from filename
//		if doesn't exist return false
//		1. get list of .hdr files in directory
//		2. present dialog w/ list to user and ask to select .hdr to use
//		3. construct prefix from .hdr and return
//
bool
DataHeader::prefix( const char *filename, char *fileset)
{
	char hdrname[256];
	struct stat buf;

	if ( filename == NULL)
	{
		fileset[0] = '\0';
		return false;
	}

	// construct prefix assuming 1 digit .dat filename
	strcpy( hdrname, filename);
	if (strchr( hdrname, '.'))
	{
		*(strchr( hdrname, '.')-1) = '\0';
		strcat( hdrname, ".hdr");
		if ( stat( hdrname, &buf) == 0)
		{	// found .hdr
			strcpy( fileset, hdrname);
			*(strchr( fileset, '.')) = '\0';
			return true;
		}

		// still here, assume 2 digit name
		strcpy( hdrname, filename);
		*(strchr( hdrname, '.')-2) = '\0';
		strcat( hdrname, ".hdr");
		if ( stat( hdrname, &buf) == 0)
		{	// found .hdr
			strcpy( fileset, hdrname);
			*(strchr( fileset, '.')) = '\0';
			return true;
		}

		// still here, not looking good
		fileset[0] = '\0';
	}

	// if didn't return above, had error
	fileset[0] = '\0';
	return false;
}
