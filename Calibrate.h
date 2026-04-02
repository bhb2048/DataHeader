//
//	Calibrate.h - calibrate class for GLAS
//
//	23-dec-92  bhb
//	Version 1.0
//	Modified:
//	17-dec-97  bhb	removed CGlasDataMgr *DM member
//	03-mar-08  bhb	add bits arg to set_xform so can adjust for bits in a/d
//	
//

#ifndef CALIBRATE_H
#define CALIBRATE_H

#include <Data/ChannelGr.h>

class AdGain;

//
//  raw data to calibrated conversion formula:
//	caldat = (rawdat - off) * conv;
//
class Calibrate
{
	public:
		Calibrate();
		virtual ~Calibrate();

		int     getNchan()          { return _nchan; }
		void    setNchan( int n)        { _nchan = n; }
		void    setOffset( int pch, int val)  { _offset[pch] = val;  }
		void    setScale( int pch, float val)   { _scale[pch] = val; }
		int     getOffset( int pch)       { return _offset[pch]; }
		float   getScale( int pch)      { return _scale[pch];}
		int     got_cal()           { return _gotCal;    }

		// convert calibrated value to raw
		int to_raw( int pch, float val)
			{ return( int(val/_scale[pch] + _offset[pch]));  }

		float   to_cal( int pch, int val)
			{ return( (val - _offset[pch]) * _scale[pch]);}
		void    to_cal( int pch, int nscan, short *in, float *out);

		void    get_xform( int pch, float *c, int *o);
		void    set_xform( AdGain *Gn, unsigned short bits);

		int writeCal( const char *fn);
		int readCal( const char *fn, int *toff, float *tscale);
		virtual void    writeCalFile();
		virtual int     readCalFile( const char *prefix = 0);

	protected:

		int     _nchan;
		int     _offset[NCHAN];					// offsets & conversion stored in
		float   _scale[NCHAN];					// physical channel order
		int     _gotCal;
};
#endif
