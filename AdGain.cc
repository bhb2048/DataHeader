//*****************************************************************************
//	Copywrite (C) 1992 Barry H. Branham.  All rights reserved.
//*****************************************************************************
//
//  AdGain.cc  -- routines to set the a/d and bank gains.
//
//	24-aug-92	jwp
//	Version: 1.1
//	Modified:
//	01-dec-92  bhb	add set/get_bank_gain(), set/get_ad_gain(),
//					remove corresponding header() calls
//					added static gain arrays
//	08-dec-92  bhb	changed name to gain.c from chggains.c, included
//					r_w_gnfltext.c
//	30-jun-95  bhb	converted to AdGain class - non GUI base class
//	17-sep-97  bhb	moved to headerlib.a from glaslibs/datalib.a
//	17-sep-97  bhb	removed Num_gain_files, added _changed
//	18-sep-97  bhb	previous version assumed chan bank and adc board
//					had same # chan (Version 1)
//					Now not making that assumption (Version 2)
//					Adding Version # to file,
//	09-jan-98  bhb	fix bug introduced by adding version2, need to init
//					_nChan when read version 1 files.
//
#include <Data/AdGain.h>
#include <errno.h>

#define VERSION 2

//*****************************************************************************
//
//  public functions
//
//*****************************************************************************
AdGain::AdGain( int nb, int nc)
{
	_numBanks = nb;
	_nChan = nc;
	_changed = 0;
	_numFile = 0;
}

AdGain::~AdGain()
{

}

bool
AdGain::setAdcGain( int chan, int gain)
{
	if ((chan < NCHAN) && (gain <= 8))
	{
		_adcGain[ chan] = gain;
		return true;
	}
	else    return false;
}

bool
AdGain::setBankGain( int bank, float gain)
{
	if ( bank < NBANK)
	{
		_bankGain[ bank] = gain;
		_changed++;
		return true;
	}
	else    return false;
}

//*****************************************************************************
//
//  read/write file functions
//
//*****************************************************************************
//
//  uses internal _numFile to get file #
//
bool
AdGain::wrGainText( const char *prefix)
{
	char fn[256];
	FILE *fp;
	int i, j;
	int n_bank;

	sprintf( fn, "%s%02d.gn", prefix, _numFile+1);
	if ((fp = (FILE *)fopen( fn, "w")) == NULL)
		return( false);
	fprintf( fp, "\f\r\t\t*** GAIN FILE ***\n\n\n");
	fprintf( fp, "Version %d\n", VERSION);
	fprintf( fp, "Num Amp Banks: %d\n", n_bank = numBanks());
	for ( i=0; i<n_bank; i++)
	{
		fprintf( fp, "%f\n", bankGain( i));
	}
	fprintf( fp, "Num Chan: %d\n", nChan());
	for ( j=0; j<nChan();)
	{
		fprintf( fp, "%d ", adcGain( j));
		if ( !(++j % 16)) fprintf( fp, "\n");
	}
	_changed = 0;
	fclose( fp);
	_numFile++;
	return true;
}

void
AdGain::wr_gfl_txt( FILE *fp)			// output file or device
{
	int i, j;
	int n_bank;

	fprintf( fp, "%d\n", n_bank = numBanks());
	for ( i=0; i<n_bank; i++)
	{
		fprintf( fp, "%f\n", bankGain( i));
		for ( j=0; j<64;)
		{
			fprintf( fp, "%d ", adcGain( j+i*64));
			if ( !(++j % 16)) fprintf( fp, "\n");
		}
	}
	_changed = 0;
}

bool
AdGain::rd_gfl_text( const char *prefix, const int fn)
{
	FILE *fp;
	char gn_file[512], msg[20];

	fp = NULL;
	sprintf( gn_file, "%s%02d.gn", prefix, fn);

	if  ((fp=(FILE *)fopen(gn_file, "r")) == NULL)
	{
		perror(gn_file);
		printf(msg, "Couldn't find .gn file\n");
		return false;
	}
	else
	{
		return rdGainTextFile( fp);
	}
}

bool
AdGain::rdGainTextFile( FILE *fp)
{
	char str[256];
	int n_banks;
	float bank_gain;
	int ad_gain;
	int i, j;
	int version;

	fscanf(fp, "\f\r\t\t*** GAIN FILE ***\n\n\n");
	fgets( str, 256, fp);
	if ( str[0] == 'V')
		sscanf( str, "Version %d", &version);
	else
		version = 1;
	switch ( version)
	{
		case 1:									// old file
			sscanf( str, "%d", &n_banks);
			setNumBanks( n_banks);
			for (i=0; i<n_banks; i++)
			{
				fscanf(fp, "%f\n", &bank_gain);
				setBankGain( i, bank_gain);
				for (j=0; j<64; j++)
				{
					fscanf(fp, "%d ", &ad_gain);
					setAdcGain( j+i*64, ad_gain);
				}
			}
			setNChan(n_banks * 64);
			break;
		case 2:
			fscanf( fp, "Num Amp Banks: %d\n", &n_banks);
			setNumBanks( n_banks);
			for ( i=0; i<n_banks; i++)
			{
				fscanf(fp, "%f\n", &bank_gain);
				setBankGain( i, bank_gain);
			}
			fscanf( fp, "Num Chan: %d\n", &_nChan);
			for ( i=0; i<nChan(); i++)
			{
				fscanf(fp, "%d ", &ad_gain);
				setAdcGain( i, ad_gain);
			}
			break;
		default:
			fprintf( stderr, "rdGainTextFile: unknown version\n");
			return false;
	}
	_changed++;
	return true;
}
