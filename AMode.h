//////////////////////////////////////////////////////////////////////////
//  AMode.h - Analysis mode class
//  
//  22-feb-01  bhb	split out of analysis.h
//  Modified:
//  01-mar-01  bhb	beautified names
//  23-may-01  bhb  add _id
//  25-may-01  bhb	add AModeList class
//  11-nov-02  bhb	add updateControls()
//  09-dec-02  bhb	now use AnalysisGrVk (renamed from GlasAnalysisGr)
//  09-nov-05  bhb	now use AnalysisGr
//  18-jan-06  bhb	add _oldNames, checkOldNames()
//  20-mar-06  bhb	add hasVxyOutput()
//  27-oct-12  bhb	change _dataLoc, _dataTitle to vectors, remove _numOutputs
//  04-feb-13  bhb	add AMode::getNumOutputs()
//  
//////////////////////////////////////////////////////////////////////////
#ifndef AMODE_H
#define AMODE_H

#include <Data/AModule.h>			// for data_loc_t
#include <Data/AnalysisGr.h>		// for MAX_MODULES (15)
#include <vector>
#include <string>

class AModule;
class AnalysisGr;

class AMode
{
	public:
		AMode( int id = -1, const char *nm = NULL);
		AMode( const AMode &am);
		~AMode();

		// access functions
		int			id() 	const			{ return _id;	}
		int	&		id() 					{ return _id;	}
		void		setName( const char *n)	{ strncpy( _name, n, 31); }
		char *		name()					{ return _name;}
		const char *name()	const			{ return _name;}
		std::vector<std::string> & oldNames()			{ return _oldNames;	}
		void		setNumMods( int n)		{ _numMods = n;}
		int			numMods()	const		{ return _numMods;}
		void		setModule( int i, AModule *am) { _modules[i] = am;}
		AModule *	module( int i)			{ return _modules[i];	}
		int			numOutputs()	const	{ return _dataLoc.size();	}
		void		setDataLoc( int i, data_loc_t *dl) { _dataLoc[i] = *dl;}
		const data_loc_t *dataLoc( int i) const	{ return &_dataLoc[i]; }
		data_loc_t *dataLoc( int i)			{ return &_dataLoc[i]; }
		void		setDataTitle( const char *s)
					{ std::string title = s; _dataTitle.push_back( title);	}
		const char * dataTitle( int i) const { return _dataTitle[i].c_str();	}

		bool		read( FILE *fp);
		void		initControls( AnalysisGr *ag);
		void		updateControls( AnalysisGr *ag);
		bool		hasVxyOutput();
		
	protected:
		int			_id;							// mode id #
		char		_name[32];						// the name of this mode
		std::vector<std::string>	_oldNames;
		int			_numMods;						// number of modules in mode
		AModule *	_modules[MAX_MODULES];			// the modules
		std::vector<data_loc_t>  	_dataLoc;		// location of final anlz data outputs											
		std::vector<std::string>	_dataTitle;		// output member names

		char *		skipComments( FILE *fp);
		char *		checkOldNames( FILE *fp);
		int			getNumOutputs( FILE *fp);
};

class AModeList
{
	public:
		AModeList()	{}
		virtual ~AModeList();

		AMode *	elem( unsigned int i)		{ if ( i < size()) return _amlist[i];	}
		AMode *	amode( int i);
		AMode * amode( const char *str);
		void	add( AMode *am)		{ _amlist.push_back(am);	}
		virtual int		read( std::string &errMsg);
		unsigned int size()			{ return _amlist.size();	}

	protected:
		std::vector<AMode *>	_amlist;

		char *	skipComments( FILE *fp);
};
#endif
