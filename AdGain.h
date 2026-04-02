//
//  AdGain.h - header for not GUI GLAS gain file class
//
//	30-jun-95  bhb
//	Modified:
//	17-sep-97  removed Num_gain_files, added _changed
//	03-mar-01  bhb	beautified names
//	05-nov-05  bhb	add const to some args
//	
#ifndef ADGAIN_H
#define ADGAIN_H

#ifndef __STDIO_H__
#include <stdio.h>
#endif
#include <Data/ChannelGr.h>					// for NCHAN #define

#define NBANK   4
#define DEFAULT_AD_GAIN     2
#define DEFAULT_AMP_GAIN    500

class AdGain
{
	public:
		AdGain( int nb = 0, int nc = 0);
		virtual ~AdGain();

		// access

		void    setNumBanks( int nbanks){ _numBanks = nbanks;   }
		int 	numBanks()				{ return _numBanks; }
		void    setNChan( int n)		{ _nChan = n;   }
		int 	nChan()             	{ return _nChan;    }
		int *   adcGain()           	{ return _adcGain;   }
		int 	adcGain( int ch)        { return (ch >=0 && ch < NCHAN) ? _adcGain[ch] : 0;   }
		bool 	setAdcGain( int chan, int gain);
		float * bankGain()           	{ return _bankGain;   }
		float   bankGain( int bk)	
				{ return ( bk >= 0 && bk < NBANK) ? _bankGain[ bk] : 0.F;	}
		bool 	setBankGain( int bank, float gain);
		bool 	changed()           	{ return _changed > 0;   }
		void    changed( int v)			{ _changed = v; }
		void    setNumFile( int n)      { _numFile = n; }
		int 	numFile()           	{ return _numFile;  }

		// actions

		bool 	wrGainText( const char *prefix);
		void    wr_gfl_txt( FILE *fp);
		virtual bool rd_gfl_text( const char *prefix, const int fn);
		bool 	rdGainTextFile( FILE *fp);

	protected:
		int 	_numBanks;					// number of amplifier banks
		float   _bankGain[ NBANK];			// amplifier bank gain values
		int 	_nChan;						// number of adc channels
		int 	_adcGain[ NCHAN];			// programmable adc gain vals 1,2,4,8
		int 	_changed;					// True if gains have changed since write
		int 	_numFile;					// number of gain files written
};
#endif
