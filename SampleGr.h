//
//	SampleGr.h - Sample Group class
//
//	22-feb-2001  bhb	split outof DataHeader.h
//	Modified:
//	03-mar-01  bhb	beautified names, remove copy(), make copy constructor
//	07-nov-01  bhb	removed SgList::edit()
//	10-mar-04  bhb	write() now returns int
//	13-jun-07  bhb	SgList no longer has list base class
//	
#ifndef SAMPLEGR_H
#define SAMPLEGR_H

#include <Data/ChannelSet.h>
#include <list>
#include <fstream>

#define CUR_SG_VERSION  2

#define SG_CHANGE_CHAN	1
#define SG_CHANGE_RATE	2

class SgList;
class DataHeader;

class SampleGr : public ChannelSet
{
	public:
		SampleGr(  SgList * p=NULL);
		SampleGr( const SampleGr &sg);
		virtual ~SampleGr();

		// access functions

		const int	rate() const	{ return _rate;  }
		void    	setRate( int r)	{ _rate = r; }
		void        setParent( SgList *p) { _parent = p;}
		SgList *	Parent()		{ return _parent;	}

		// actions
		void		changed( unsigned int v)	{ _changed = v;	}
		unsigned int changed()					{ return _changed;	}
		int     	index();
		
		virtual bool	read( std::ifstream &inFile, int sg_ver, int hdr_ver);
		virtual bool	write( std::ofstream &outFile);
		virtual int     edit();
		virtual void	freeAll();

		SampleGr& operator = ( const SampleGr &sg);

	protected:
		int         	_rate;			// scan rate (hz)
		SgList *		_parent;
		unsigned int	_changed;
};

class SgList
{
	public:
		SgList( DataHeader *hdr=NULL);
		SgList(const SgList &sgl);
		virtual ~SgList();

		void    		setParent( DataHeader *hdr)	{ _parent = hdr;	}
		DataHeader *	Parent()					{ return _parent;	}
		std::list<SampleGr *> *	sgList()			{ return &_list;	}
		
		void    		free_all();
		SampleGr *		elem( int i);
		int 			size()		{ return _list.size();	}
		
		virtual bool 	read( std::ifstream &inFile, int version);
		virtual bool 	write( std::ofstream &outFile);

	protected:
		DataHeader *		_parent;
		std::list<SampleGr *>	_list;
};
#endif
