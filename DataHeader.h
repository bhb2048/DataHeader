//*******************************************************************************
//	DataHeader.h -- sample display and analysis groups
//
//	3-jun-92  bhb, jwp	from glas.h
//
//	Modified:
//	11-Aug-94  jwp	added DataHeader class.
//	22-apr-96  bhb	added virtual void edit() to Dg, Sg, & AgList classes
//	03-sep-97  bhb	made prettier
//	04-sep-97  bhb	added Dglist() to ChannelSet
//	14-jan-98  bhb	change ScaleInfo::scale() to use vec_xy &
//	26-jan-01  bhb	changed class names: CSample_gr 	-> SampleGr
//										 CDisplay_gr 	-> DisplayGr
//										 CAnalysis_gr 	-> AnalysisGr
//										 Channel_gr		-> ChannelGr
//										 Channel_info	-> ChannelInfo
//										 Channel_set	-> ChannelSet
//	02-feb-01  bhb	added ChannelSet::_curDg, was in DasSampleGr
//	22-feb-01  bhb	split out ChannelGr.h, ChannelInfo.h, ChannelSet.h,
//					DisplayGr.h, SampleGr.h, AnalysisGr.h
//	21-jun-02  bhb	added some access functions
//	04-nov-05  bhb	moved incNumDataFiles(), incNumGainFiles(), incBlocks() here from
//					HeaderVk, changed _studyTitle, _studyDate to string
//					_numBlocks now vector<int>, numDataFiles() now _numBlocks.size()
//	10-nov-05  bhb	moved _fileName & funcs here from HeaderVk
//	04-oct-07  bhb	add haveData()
//	04-mar-08  bhb	add _hdrFileTime
//	24-jul-08  bhb	readSet(), readHdr(), writeSet(), writeHdr(), setFilename() now use 
//					string arg
//	25-sep-08  bhb	add _rawbits, bump CUR_HEADER_VERSION to 9
//	15-oct-08  bhb	add read( ifstream), write(ofstream) so subClasses can use
//					if need to read/write extra
//	05-feb-10  bhb	how have Subject super class, can use notify() & message(string)
//	28-sep-12  bhb	made _sampGroups, _analGroups not pointers, removed setSgList() not used
//					add AModeList *aml=0 param to readHdr(), readSet()
//	22-feb-13  bhb	add RtAgList
//	14-may-13  bhb	add _notifyCmd, used by AnalysisGr for mode change notification.
//	08-dec-15  bhb	delete _notifyCmd, don't want GUI dependence here
//	09-dec-15  bhb	add _errStr, getError(), clearError()
//	15-dec-15  bhh	add haveAgError()
//	
// ******************************************************************************/
#ifndef DATAHEADER_H
#define DATAHEADER_H

#include "SampleGr.h"
#include "AnalysisGr.h"
#include "RtAnalysisGr.h"
#include <Patterns/Observer.h>
#include <time.h>
#include <string>
#include <vector>

#define CUR_HEADER_VERSION  10
#define CUR_SETUP_VERSION   9

class SampleGr;
class SgList;
class AnalysisGr;
class AgList;
class DisplayGr;
class DgList;

/******************************************************************************
 *	Data Header Class
 ******************************************************************************/
class DataHeader : public bhb::Subject
{
	public:
		DataHeader();
		virtual ~DataHeader();

		virtual bool	readSet( const std::string &filename);
		virtual bool	writeSet( const std::string &filename);
		virtual bool	readHdr( const std::string &filename, AModeList *aml=0);
		bool			readHdr( std::ifstream &inFile, AModeList *aml=0);
		virtual bool	writeHdr( const std::string &filename);
		bool			writeHdr( std::ofstream &outFile);

		// access
		const char *	Filename()      { return _filename.c_str(); }
		int				setFilename( const std::string &str, bool stripDfNum = false);
		const char *	title()			{ return studyTitle();	}
		const char *	studyTitle()	{ return _studyTitle.c_str();	}
		void			setTitle( const char *title)	{ _studyTitle = title;	}
		void			setTitle( const std::string &title)	{ _studyTitle = title;	}
		const char *	date()			{ return _studyDate.c_str();	}
		void		setDate();
		void		setDate( char *date)	{ _studyDate = date;	}
		void		setDate( const std::string &date)	{ _studyDate = date;	}

		void		setSgList( SgList *sgl)	{ _sampGroups = sgl;	}
		SgList *	sgList()    { return _sampGroups; }
		AgList *	agList()	{ return &_analGroups;	}
		RtAgList *	rtAgList()	{ return &_RtAgList;	}

		int 		nDataFiles()    { return _numBlocks.size();	}
		int			curDf()         { return nDataFiles() - 1;  }
		int 		nBlocks(int file);
		void		setBlocks( int *blks, int n);
		int 		nGainFiles()    { return _numGainFiles;	}
		void        incNumDataFiles()	{ _numBlocks.push_back(0);	}
		void        incNumGainFiles()   { _numGainFiles++;   }
		void        incBlocks( int nblks)	{ _numBlocks.back() += nblks;	}
		bool		haveData()		{ return( nDataFiles() > 0 && nBlocks(0));	}
		time_t &    hdrFileTime()   { return _hdrFileTime;  }
		
		SampleGr *		sg(int i)		{ return Sg(i);	}
		SampleGr *		Sg(int i)		{ return( sgList()->elem(i));   }
		AnalysisGr *	ag(int i)		{ return Ag(i);	}
		AnalysisGr *	Ag(int i)		{ return( agList()->elem(i));   }
		int     		numSgs()		{ return( sgList()->size());    }
		int     		numAgs()		{ return( agList()->size());    }
		
		bool			prefix( const char *filename, char *fileset);
		unsigned short	databits()		{ return _databits;	}
		void			databits( unsigned short bits)	{ _databits = bits;	}

		std::string &	getError()	{ return _errStr;	}
		void			clearError()	{ _errStr = "";	}
		bool			haveAgError();
		uint			version()	{ return _version;	}
		
//		virtual void    editSgs();
//		virtual void    editAgs();

	protected:
		std::string			_filename;		// complete .hdr filepath & name (prefix)
		std::string			_studyTitle;
		std::string			_studyDate;		// format: mm/dd/yy

		SgList *			_sampGroups;	// pointer because gets set externally
		AgList				_analGroups;
		RtAgList			_RtAgList;
		
		std::vector<int>	_numBlocks;
		int     			_numGainFiles;
		time_t				_hdrFileTime;	// used by Dash to synchronize if Glas changed
		unsigned short		_databits;		// size of samples

		uint				_version;
		std::string			_errStr;
		
		virtual void	clear_header();
};
#endif
