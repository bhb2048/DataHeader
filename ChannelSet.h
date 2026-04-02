//
//	ChannelSet.h - ChannelGr subclass w/ extra channel info
//
//	22-feb-2001  bhb	split outof DataHeader.h
//	Modified:
//	03-mar-01  bhb	beautified names, remove copy(), make copy constuctor
//	19-jun-02  bhb	removed ScaleInfo
//	20-sep-02  bhb	added copy operator
//	04-jun-03  bhb	move chName() here from ChannelInfo.h, using chan(i) not 'i'.
//	10-mar-04  bhb	add read(), write()
//	16-jun-04  bhb	add cs_type, _type
//	23-may-05  bhb	merge ChannelInfo class into ChannelSet
//	09-nov-05  bhb	add setDgList()
//	23-jul-08  bhb	change _units to string[]
//	02-oct-08  bhb	add minNumDgChan() & setDgWithLEChan() as help utilities for Dash
//	03-feb-11  bhb	add nch to ctor
//	23-oct-12  bhb	add names()
//	
#ifndef CHANNELSET_H
#define CHANNELSET_H

#include <Data/ChannelGr.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>

#define NCHANNAMES  64		// only names for 1st 64 chan
#define MAX_UNITS	8

class DisplayGr;
class DgList;

class ChannelSet : public ChannelGr
{
	public:
		enum cs_type { CS_TYPE_NONE, CS_TYPE_SG, CS_TYPE_AG};
		
		ChannelSet( cs_type type=CS_TYPE_NONE, int nch=0);
		ChannelSet( const ChannelSet &cs);
		virtual ~ChannelSet();


		// access functions
		DgList *		dgList() const		{ return _dgs;   }
		int				numDgs();
		void			addDg( DisplayGr *dgr);
		void			setDgList( DgList *dgl);
		DisplayGr *		dg(int i);
		DisplayGr *		curDg()				{ return _curDg;	}
		void			setCurDg( DisplayGr *dg)	{ _curDg = dg;	}
		const cs_type	type() const		{ return _type;	}

		std::string *	names()				{ return _names;	}
		const char * 	chName( int i) const
						{	if ( i < NCHANNAMES) return _names[i].c_str();
							else
							{
								static char tmpname[8];
								sprintf( tmpname, "%d", chan(i)+1);
								return tmpname;
							}
						}
		
		int				numNames()				{ return _nNames; }
		void			setChName( int i, const char *nm)
						{ if ( i < NCHANNAMES) _names[i] = nm;}
		void			setnUnits( int n)		{ _nUnits = n;  }
		int				numUnits()       { return _nUnits; }
		void			setUnits( int i, const std::string &u)  { _units[i] = u; }
		const char *	units( int i) const	{ return _units[i].c_str(); }
		const short		unitsIndx( int i) const	{ return _unitsIndx[i]; }
		int				unitsChan( int c, unsigned short *uc);
		void			setUnitsChan( unsigned short c, unsigned short u);
		virtual const char *yunits( int i)		{ return _units[_unitsIndx[i]].c_str(); }

		void			setNames( char **names = 0, int n=0);
		void			initDefUnits(int n);
		int				minNumDgChan();
		bool			setDgWithLEChan( int n);
		
		void			rd_chnames( std::ifstream &inFile, int);
		void			wr_chnames( std::ofstream &outFile, int);
		void			rd_units( std::ifstream &inFile, int);
		void			wr_units( std::ofstream &outFile, int);

		virtual void	initDefNames( int n);
		virtual void	editDgs();
		virtual	void	freeAll();
		virtual bool	read( std::ifstream &outFile);
		virtual bool	write( std::ofstream &outFile);

		ChannelSet& operator = ( const ChannelSet &cs);

	protected:
		DgList *		_dgs;				// display groups
		DisplayGr *		_curDg;				// current Dg
		cs_type			_type;				// channel set type SG, AG, DG
		int				_nNames;
		std::string		_names[NCHANNAMES];	// channel names
		int				_nUnits;
		std::string		_units[MAX_UNITS];	// units strings
		std::vector<unsigned short>	_unitsIndx;		// index in units[] for each chan

		void			setUnitsIndx( int u, const char *chstr, int sub);
};
#endif
