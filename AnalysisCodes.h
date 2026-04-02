//
//	AnalysisCodes.h	- old analysis codes, used in Map3, from unused dataf.h
//	
//	04-jan-02  bhb
//	Modified:
//	02-apr-13  bhb   add ARI
//	
#ifndef ANALYSISCODES_H
#define ANALYSISCODES_H
/*
 *	analysis function codes (mode2) (>100 to avoid conflict with sdgraf (seedat!))
 *	here for reading old header files
 */
#define BIP1            101     // bipolar activation times
#define UNI1            102     // unipolar qrst waveform
#define AHG1            103     // obsolete - HEMOED1 instead
#define BEAT_INDX       104
#define SCALE           105
#define HEMOED2         106
#define HEMO1           107     // basic hemo
#define HEMOED1         108     // energy dissipation analysis
#define POT1            109     // for single-frame inplace mapping
#define POT2            110     // for multi-frame framestack case
#define PACE            111
#define FREQ1           112
#define UPPA1           113     // unipolar uppa or potential times
#define UNIACT          114     // unipolar activation times
#define FOUR1           115
#define HEMOW1          116     // pr/vol work loop
#define HEMOW2          117     // all pr/vol work loops
#define SMOOTH          120     // data smooth
#define SIGAVG          121     // signal averaging
#define FOFT            122     // fourier analysis
#define DVDT			123		// dvdt
#define TWOWV			124		// two wave mapping
#define MAPVAL			125		// mdt files, or any single value
#define ARI             126     // activation recovery interval
#endif
