/******************************************************************************
 *	Copywrite (C) 1992 Barry H. Branham.  All rights reserved.
 ******************************************************************************/
/*
 *    dataf.h - basic data structure definitions for data files
 *
 *    12-nov-91 xy
 */

#ifndef DATAF_H
#define DATAF_H
#ifndef CXLIBINC
#include <CXlib/cxlib.h>	/* Define date,time vectors  */
#endif

#ifndef DFLAG
#define DFLAG	0x7fff
#define XFLAG	0x7ffe
#endif

#define MAXCHAN		512
#define MAXPASS		4
#define MAXTIMES 	31	/* for .atm data		*/
#define REGSIZ	0x10000		/* 64kb region			*/
#define MAXFTIMES	5000	/* max dynamic map times	*/
#define MAXSCALEVAL	10	/* maximum # iso scale values	*/

/*
 *      page header struct
 */
struct page_header {
    char    title[32];          /* study title                  */
    char    date[12];           /* study date                   */
    int     file;               /* current data file            */
    int     run;                /* current run                  */
    char    start_time[16];     /* start time                   */
    char    comment[ 70];       /* user comment for this run    */
};

struct srd_file {
    char title[32];
    struct date_v date;
    int file;
    int run;
    int rate;
    int start_time;
    char name[4];
    char units[8];		/* scale units string	*/
    int npts;			/* # data points	*/
    float *data;
};
typedef struct srd_file srd_file_t;

/*
 *      structure for map values:
 *              activation times, recovery times, frequencies
 *      data stored in analysis order for multiple run analysis
 */
struct mapval {
    short ref;                  /* reference value      */
    float val[ MAXCHAN*MAXPASS];  /* event values         */
};


/*
 *      structure for potential analysis
 */
struct baseline {
    short tim;
    short bval[ MAXCHAN][2];
};

/*
 *      structure for multi-beat activation times
 */
struct act_times {
    short n_times;
    short times[ MAXTIMES];
};

/*
 *      structure for atm interval values
 */
struct atm_interval {
        int i_min;
        int i_max;
};

/*
 *	analysis function codes (mode2) (>100 to avoid conflict with sdgraf)
 *	here for reading old header files
 */
#define BIP1            101     /* bipolar activation times		*/
#define UNI1            102     /* unipolar qrst waveform		*/
#define AHG1            103     /* obsolete - HEMOED1 instead		*/
#define BEAT_INDX       104
#define SCALE           105
#define HEMOED2         106
#define HEMO1           107     /* basic hemo				*/
#define HEMOED1         108     /* energy dissipation analysis		*/
#define POT1            109     /* for single-frame inplace mapping	*/
#define POT2            110     /* for multi-frame framestack case	*/
#define PACE            111
#define FREQ1           112
#define UPPA1           113     /* unipolar uppa or potential times	*/
#define UNIACT          114     /* unipolar activation times		*/
#define FOUR1           115
#define HEMOW1          116     /* pr/vol work loop			*/
#define HEMOW2          117     /* all pr/vol work loops		*/
#define SMOOTH          120     /* data smooth				*/
#define SIGAVG          121     /* signal averaging			*/
#define FOFT            122     /* fourier analysis			*/
#define DVDT		123	/* dvdt					*/
#define TWOWV		124	/* two wave mapping			*/
#define MAPVAL		125	/* mdt files				*/



#endif
