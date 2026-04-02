/******************************************************************************
 *	Copywrite (C) 1992-2006 Barry H. Branham.  All rights reserved.
 ******************************************************************************/
//
//  AdLog.h -- GLAS log class
//
//  14-Jun-93	jwp
//  Version 1.1
//  Modified:
//  17-feb-94  bhb	moved class CSample_gr here from glasdata.h
//  12-dec-97  bhb	added AdLog::readFile(), set_run_lengths() to make
//			 		easier integration w/ DasHeader for Dash
//  28-jan-98  bhb	add AdLog, AdRun '=' operators for copy,
//			 		also AdLog & AdRun copy constructors.
//	17-feb-01  bhb	add inline AdLog::Run() member
//	15-mar-01  bhb	add _changed, AdRun::write()
//	23-aug-02  bhb	added read()
//	24-aug-06  bhb	made comment const
//	07-nov-06  bhb	remove #define LOGSIZE 50, change AdLog/AdRun member vars to _XXX
//					make _Runs a vector<AdRun *>, remove 'int _numRuns'
//	02-apr-06  bhb	add AdRun::read(), AdLog::addRun()
//	05-feb-10  bhb	how have Subject super class, can use notify() & message(string)
//	25-sep-12  bhb	Version 2, add length (msec) to log file, AdRun::_comment now a string
//	
#ifndef ADLOG_H
#define ADLOG_H

#include <Patterns/Observer.h>
#include <Fltk/NotifyCmd.h>
#include <stdio.h>			// for FILE
#include <vector>
#include <string>

#define VERSION	2

class AdLog;
class DataHeader;

class AdRun
{
	friend class AdLog;

	public:
		AdRun();
		AdRun( const AdRun &r);
		AdRun( int sg, int gfn, int blk, int st, int len, const char *cmt=NULL);
		virtual ~AdRun() {}

		// access

		int				Sgr()				{ return _sampGr;	}
		int				Length()			{ return _length;	}
		void			setLength( int len)	{ _length = len;	}
		int				Start()			{ return _start;	}
		int				gainFile()		{ return _gainFileNum;	}
		const char *	Comment()		{ return _comment.c_str();	}
		void			setBlknum( int blk)	{ _blkNum = blk;	}
		int				Blknum()			{ return _blkNum;}
		void			setComment( const char *cmt);
		void			setComment( std::string &cmt)	{ setComment( cmt.c_str());	}
		int				changed()		{ return _changed;	}
		void			write( std::ofstream &outFile, int index);
		bool			read( std::ifstream &inFile, int i, int &lastblk, int version);
		AdRun&	operator = ( const AdRun& r);	// assignment of a AdRun

	protected:
		short		_sampGr;					// this sample group #
		short		_gainFileNum;				// applicable gain file (1 is first)
		int			_blkNum;					// starting block number
		int			_start;						// run start in msec since midnight
		int			_length;					// run length in msec
		std::string	_comment;					// user comment
		int			_changed;					// > 0 if something changed
};

class AdLog : public bhb::Subject
{
	public:
		AdLog();
		AdLog( const AdLog &l);
		AdLog( int fn);
		~AdLog();

		// access

		int				fileNum()		{ return _fnum;	}
		int				num_runs()		{ return _Runs.size();	}		// deprecated
		int				numRuns() const		{ return _Runs.size();	}
		AdRun *			Run( int n)
						{ return ((n >= 0 && n < numRuns()) ? _Runs[n] : 0);	}
		int				get_run_sg(int run)		{ return( _Runs[run]->_sampGr);	}
		int				get_run_blknum(int run)	{ return( _Runs[run]->_blkNum);}
		int				get_run_length(int run)	{ return( _Runs[run]->_length);}
		int				get_run_stime(int run)	{ return( _Runs[run]->_start);}
		int				get_run_gainfl(int run)	{ return(_Runs[run]->_gainFileNum);}
		const char *	get_run_comment(int run){ return( _Runs[run]->_comment.c_str());}
		void			set_run_comment(int run, const char *cmt)
						{ _Runs[run]->setComment( cmt);	}
		//		int	get_run_start()		{ return( _Runs[DM->get_Cur_run()]->_start);}
		//		char *	get_comment()		{ return( get_run_comment( DM->get_Cur_run()));}
		bhb::NotifyCmd *notifyCmd()		{ return _notifyCmd;	}

		//	actions

		void	freeAll();
		AdRun *	newRun();
		void	addRun( AdRun *run)	{ _Runs.push_back( run);	}
		void	initRun( int sg, int gfl, int stime, int nmsec, const char *cmt=((char *)0))
		{
			AdRun *r = _Runs[numRuns()-1];
			r->_sampGr = sg;
			r->_gainFileNum = gfl;
			r->_start = stime;
			r->_length = nmsec;
			r->setComment( cmt);
		}
		int		get_run_for_stime( int stime);
		int		read( const char *prefix, const int nlf, DataHeader *Hdr)
				{ return read_log_file( prefix, nlf, Hdr);	}
		int		write( const std::string &prefix)
				{
					return wrt_log_file( prefix.c_str(), _fnum);
				}
		int		write( const char *prefix)
				{
					return wrt_log_file( prefix, _fnum);
				}
		int changed()
			{
				for (int i=0; i<numRuns(); i++)
					if ( _Runs[i]->changed()) return 1;
				return 0;
			}
		AdLog& operator = ( const AdLog& l);	// assignment of a AdLog

	protected:
		int						_fnum;				// number of this log file (first == 0)
		std::vector<AdRun *>	_Runs;				// current data file log
		bhb::NotifyCmd *		_notifyCmd;

		int		read_log_file( const char *prefix, const int nlf, DataHeader *Hdr);
		int		readFile( const char *prefix, const int nlf, int &version);
		void	set_run_lengths( DataHeader *Hdr);
		bool	wrt_log_file( const char *prefix, const int nlf);
		bool	checkRuns( int nsg);
};
#endif
