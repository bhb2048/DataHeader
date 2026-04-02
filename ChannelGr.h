//
// ChannelGr.h - channel group class.  Base for Sample, Display, Analysis groups
//
//	'mode' member intended for use to indicate mode of data acquistion,
//	eg. triggered start etc.
//	
//	22-feb-2001  bhb	split outof DataHeader.h
//	Modified:
//	03-mar-01  bhb	beautified names, remove copy(), make copy constuctor
//	20-jun-02  bhb	changed setChan(buf, n) so doesn't call setnChan(),
//					ChannelSet virtual was doing initDefNames(), initDefUnits()
//	20-sep-02  bhb	added copy operator
//	20-may-05  bhb	added numstr(), numstr_1()
//	23-may-05  bhb	added gtchan(), gtchan_0(), getnum(), get_ndiff()
//	23-feb-06  bhb	added checkSuperSet()
//	20-mar-06  bhb	added sameChan()
//	14-sep-06  bhb	added checkSubSet()
//	09-oct-06  bhb	added isContiguous(), changed other 'check' funcs to 'is'
//	04-jun-08  bhb	changed _chan to vector<unsigned short>, remove _nchan, setnChan()
//	11-jun-08  bhb	speed up isSuperSet(), add _isSequential, speed up getIndex()
//	
#ifndef CHANNELGR_H
#define CHANNELGR_H

#include <string>
#include <vector>

#define NCHAN				10000

class ChannelGr
{
	public:
		ChannelGr( int nch = 0);
		ChannelGr( const ChannelGr &cg);
		virtual ~ChannelGr();

		// access functions
		const char * name()	const		{ return _name.c_str();}
		void		setName( const char *n)	{ _name = n;}
		const int	mode() const		{ return _mode;}
		void		setMode( int m)		{ _mode = m;}
		const int	nChan()	const		{ return (int)_chan.size();}
		const int	chan( int i) const	{ return i < _chan.size() ? (int)_chan[i] : -1;}
//		const unsigned short *	chan() const	{ return _chan;	}
		void		addChan( unsigned short ch)	{ _chan.push_back( ch);	}
		void		setChan( const unsigned short *buf, int n)
		{
			_chan.clear();
			for ( int i=0; i<n; i++) _chan.push_back( buf[i]);
			_isSequential = isContiguous();
		}
		int		getIndex( int);		
		int		getIndx( int c)			{ return getIndex(c);	}		// deprecated
		char *	numstr(char *, const unsigned short *, const int);
		char *	numstr( char *pstr);
		char *	numstr_1(char *, const unsigned short *, const int);
		char *	numstr_1(char *pstr);
		int		gtchan( const char *str, unsigned short *buf, int max);
		int		gtchan( const char *str);
		int		gtchan_0( const char *str, unsigned short *buf, int max);
		int		gtchan_0( const char *str);
		int		get_ndiff();
		bool	isSuperSet( ChannelGr *cgr);
		bool	sameChan( ChannelGr *cgr);
		bool	sameChan( unsigned short *ch, int nch);
		bool	isSubSet( ChannelGr *cgr);
		bool	isContiguous();
		
		virtual void freeAll();

		ChannelGr& operator = ( const ChannelGr &cg);

	protected:
		std::string		_name;					// name of  group
		int				_mode;					// mode bits
		std::vector<unsigned short>	_chan;		// channels in group (0 is 1st)
		bool			_isSequential;			// true if chan are sequential
};
#endif
