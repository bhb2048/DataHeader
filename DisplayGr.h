//
//	DislayGr.h - Define Display Group class
//	
//	22-feb-2001  bhb	split outof DataHeader.h
//	Modified:
//	03-mar-01  bhb	beautified names, remove copy(), make copy constructor
//	13-jun-02  bhb	add setChanFromParent(), getParentChan()
//	23-sep-02  bhb	add _parentList, index()
//	20-may-05  bhb	add == operator, DgList::dgNum()
//	08-aug-06  bhb	make Fltk compatible
//	06-feb-13  bhb	DgList no longer subclass of list
//	
#ifndef DISPLAYGR_H
#define DISPLAYGR_H

#include <Data/ChannelGr.h>
#include <list>
#include <fstream>

#define CUR_DG_VERSION 2

//
//  display group modes
//
#define DG_STATIC   1
#define DG_DYNAMIC  3
#define DG_EXTENDED 2

// scale types
#define NO_SCALE    0
#define AUTO_SCALE  1
#define MAN_SCALE   2

//	axis types
#define AXIS_LINEAR	0
#define AXIS_LOGX	1
#define AXIS_LOGY	2
#define AXIS_LOGXY	3
#define AXIS_ZEROX	0x100	// used in DataPlotStat to signal subtract vxy._start
							// set in DataPlots.cc
//	display limits
#define DG_MAX_COL		128
#define DG_MIN_LENGTH   50
#define DG_MAX_LENGTH   20000
#define DG_MAX_DISPCH   1024

class DgList;
class ChannelSet;

// Defines a subset of a SampleGr or AnalysisGr.
// chan are indices in parent group chan
class DisplayGr : public ChannelGr
{
	public:
		DisplayGr();
		DisplayGr( ChannelSet *par);			// does default init from parent chan
		DisplayGr( const DisplayGr &dg);
		virtual ~DisplayGr();

		// access functions
		void        setParentList( DgList *p)	{ _parentList = p;}
		DgList *	parentList()				{ return _parentList;}
		void    	setParent( ChannelSet *p)	{ _parent = p;   }
		ChannelSet *getParent()					{ return _parent;}
		void    	setnColumn( int n)			{ _ncol = n;    }
		const int	nColumn() const				{ return _ncol; }
		void    	setAxisType( int t)			{ _axisType = t;   }
		int     	axisType()					{ return _axisType;}
		void    	setNameType( int t)			{ _nameType = t;   }
		int     	nameType()					{ return _nameType;}
		void    	setGrid( int n)				{ _grid = n; }
		int     	grid()						{ return _grid;  }
		void    	setScale( int n)			{ _scale = n;    }
		int     	scale()						{ return _scale; }
		void    	setxWind( float w)			{ _xwind = w;    }
		float		xwind()						{ return _xwind;   }

		int     		nrow();
		int				setChanFromParent( unsigned short *ch, int nch);
		int				getParentChan( unsigned short *pchan);
		void    		setDynChans(int n, int start);
		int     		index();
		virtual bool     read( std::ifstream &inFile, int dg_ver);
		virtual void    write( std::ofstream &outFile);

		void			initDefault();
		
		bool	operator == ( const DisplayGr &dg) const;

	protected:
		DgList *	_parentList;
		ChannelSet *_parent;				// parent group
		int         _ncol;					// number of screen columns
		int         _axisType;				// axis type
		int         _nameType;				// name type
		int         _grid;					// show grid
		int         _scale;					// data plot scale type
		float       _xwind;					// x window (msec)
};

class DgList
{
	public:
		DgList();
		DgList( const DgList &dgl);
		virtual ~DgList();

		std::list<DisplayGr *> *	dglist()			{ return &_list;	}
		std::list<DisplayGr *>::iterator begin()		{ return _list.begin();	}
		std::list<DisplayGr *>::iterator end()			{ return _list.end();	}
		std::list<DisplayGr *>::const_iterator begin() const	{ return _list.begin();	}
		std::list<DisplayGr *>::const_iterator end() const	{ return _list.end();	}
		int 			size()							{ return _list.size();	}

		virtual void    edit( ChannelSet *par);
		virtual int 	read( std::ifstream &inFile);
		virtual int 	read( std::ifstream &inFile, int hdr_ver);
		virtual int 	write( std::ofstream &outFile);

		void    		free_all();
		DisplayGr * 	elem( int i);
		int				dgNum( DisplayGr *dg);

	protected:
		std::list<DisplayGr *>	_list;
};
#endif
