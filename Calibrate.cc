//#define DEBUG
//
//	Calibrate.cc - calibrate class
//
//	23-dec-92  bhb
//	Version: 2.0
//	modified:
//	22-nov-93  bhb	C++, use DM->functions
//	06-jul-95  bhb	convert to C++ class
//	17-dec-97  bhb	remove dependance on CGlasDataMgr (move to CalibrateUI)
//	07-feb-01  bhb	fixup: remove underscores from names
//	23-aug-02  bhb	added code to readCalFile() virtual
//
#include "Calibrate.h"
#include <stdio.h>
#include <Data/AdGain.h>
#include <math.h>
#include <iostream>

using namespace std;

Calibrate::Calibrate()
{
	_nchan = 0;
	_gotCal = 0;
}

Calibrate::~Calibrate()
{

}

//
//  input chan is physical chan
//
void
Calibrate::get_xform(int pch, float *c, int *o)
{
	*c = _scale[pch];
	*o = _offset[pch];
}

//
//  Setup calibration arrays for raw to user values.
//  note: this fcn similiar to cal_gain() in CalibrateUI.c++
//
void
Calibrate::set_xform( AdGain *Gn, unsigned short bits)
{
	int i, bank, chan;
	float bkgain, gain;
	int nAmpBanks = Gn->numBanks();
	int nBankChan = Gn->nChan() / nAmpBanks;
	unsigned int maxval = (unsigned int)powf( 2.F, float(bits));
	unsigned int offval = (maxval >> 1) - 1;
//	cout << "Calibrate::set_xform: maxval=" << maxval << ", offval=" << offval << endl;
	
	if ( Gn->nChan() % nAmpBanks)				// check for incomplete bank usage
		nBankChan++;

	for ( bank = chan = 0; bank < nAmpBanks; bank++)
	{
		bkgain = Gn->bankGain( bank);
		for ( i = 0; i < nBankChan && chan < Gn->nChan(); i++,chan++)
		{
			_offset[ chan] = offval;			// offset binary, 0 Volts
			gain = bkgain * Gn->adcGain( chan);
			if ( gain != 0)						// -10000mv to +10000mv range
				_scale[ chan] = (20000.0 / maxval) / gain;
			else
				_scale[ chan] = 1.0;
		}
	}
}

void
Calibrate::to_cal( int pch, int nscan, short *in, float *out)
{
	for ( int i=0; i<nscan; i++)
		*out++ = to_cal( pch, *in++);
}

int
Calibrate::writeCal( const char *fn)
{
	FILE *fp;

	if ( fn != NULL && (fp = fopen( fn, "w")) != NULL)
	{
		fwrite( &_nchan, sizeof( int), 1, fp);
		fwrite( _offset, sizeof( int), _nchan, fp);
		fwrite( _scale, sizeof( float), _nchan, fp);
		fclose( fp);
		return 1;
	}
	else return 0;
}

int
Calibrate::readCal( const char *fn, int *toff, float *tscale)
{
	FILE *fp;
	int n;

	if ( fn != NULL && (fp = fopen( fn, "r")) != NULL)
	{
		fread( &n, sizeof( int), 1, fp);
		if ( n > NCHAN) return 0;
		fread( toff, sizeof( int), n, fp);
		fread( tscale, sizeof( float), n, fp);
		fclose( fp);
		return n;
	}
	else return 0;
}

void
Calibrate::writeCalFile( )
{
	// empty virtual
}

int
Calibrate::readCalFile( const char *prefix)
{
	char fname[128];
	int n;

	sprintf( fname, "%s.cal", prefix);
	if ((n = readCal( fname, _offset, _scale)))
	{
#ifdef SKIP
		// this crashes when called from CGlasDataMgr::read_raw_file()
		// because Cur_sg() not set.
		// Should enhance .cal file so we know what physical chans cal'd
		// At least the number of chan should agree - as attempted here.
		if ( n != DM()->get_raw_data()->get_Nchan())
		{
			char str[128];
			sprintf( str, "Cal file %s has %d chan, current sample group has %n.",
				fname, n, DM()->getCurSg()->nChan());
			if ( fl_show_choice( str, "Load anyway?", "", 2, "Yes", "No", "") != 1)
				return 0;
		}
#endif
		_gotCal = 1;
	}
	return _gotCal;
}
