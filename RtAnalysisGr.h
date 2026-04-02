//
//	RtAnalysisGr.h - real time analysis for Dash
//	
//	21-feb-13  bhb
//	Modified:
//	06-mar-13  bhb	add RATE_BPM, RATE_MSEC
//	
#ifndef RTANALYSISGR_H
#define RTANALYSISGR_H

#include <Data/ChannelSet.h>
#include <list>
#include <fstream>

#define CUR_RTA_VERSION 1

class RtAgList;
class DataHeader;

class RtAnalysisGr : public ChannelSet
{
	public:
		enum RTA_Type { NONE, SHUNT_RATIO, RATE_BPM, RATE_MSEC};
		
		RtAnalysisGr( RtAgList * p=0);
		RtAnalysisGr( const RtAnalysisGr &ag);		// make a copy
		virtual ~RtAnalysisGr() {}

		void			setParent( RtAgList *p) 	{ _parent = p;}
		RtAgList  *		Parent()   					{ return _parent;}
		RTA_Type type()			{ return _type;	}
		void	type( int t)	{ _type = RTA_Type(t);	}
		unsigned short	channel()	{ return (nChan() > 0) ? chan(0) : 0;	}
		unsigned short	numer()	{ return (nChan() > 1) ? chan(0) : 0;	}
		unsigned short	denom()	{ return (nChan() > 1) ? chan(1) : 0;	}
		int				minCycle()	{ return _minCycle;	}
		void			minCycle( int v)	{ _minCycle = v;	}
		
		bool	read( std::ifstream &inFile);
		bool	write( std::ofstream &outFile);
		
	protected:
		RtAgList *		_parent;
		RTA_Type		_type;
		int				_minCycle;
};

class RtAgList
{
	public:
		RtAgList() {}
		virtual ~RtAgList()	{ freeAll();	}
		
		std::list<RtAnalysisGr *> *	List()		{ return &_list;	}

		int 			size()					{ return _list.size();	}
		void			freeAll();
		RtAnalysisGr *	elem( int i);
		void    		setParent( DataHeader *hdr)	{ _parent = hdr;	}
		DataHeader *	Parent()					{ return _parent;	}
		
		bool	read( std::ifstream &inFile);
		bool	write( std::ofstream &outFile);

		std::list<RtAnalysisGr *>::iterator	begin()	{ return _list.begin();	}
		std::list<RtAnalysisGr *>::iterator	end()	{ return _list.end();	}
		
	protected:
		DataHeader *				_parent;
		std::list<RtAnalysisGr *>	_list;

};
#endif
