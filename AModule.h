//
//	AModule.h    -- analysis module base class
//
//	30-jul-92   jwp	moved from header.h
//	Version: 1.5
//	Modified:
//	09-oct-92  jwp	changed to add modular anal group design
//	02-feb-93  bhb	move page_header struct to anal.h
//	29-mar-93  bhb	added n_inputs, n_outputs, out_type to struct mod_call
//	31-jan-94  bhb	converted struct analysis_group to class CAnalysis_gr
//	03-may-96  bhb	got rid of a lot of loose c prototypes
//	24-jun-98  bhb	deleted #define ANALYSIS_MOD_PATH
//	22-feb-01  bhb	renamed from analysis.h to AModule.h, 
//					split out AMode class to AMode.h
//  01-mar-01  bhb	beautified names
//  09-mar-01  bhb	add ModuleNode subclass of VkNode
//  25-apr-01  bhb	add ModuleNode::print()
//  09-nov-01  bhb	split SelectDataMod into SelectDataMod.h
//  07-jun-02  bhb	add _defCntrls, resetControls()
//  19-jun-02  bhb	add OPRANGE_MODULE, FFT_MODULE #defines
//  24-jun-02  bhb	add PWELCH_MODULE #define
//	17-sep-02  bhb	add filter module
//	07-nov-02  bhb	revise to use AModControl
//	12-oct-05  bhb	add BASEV_MODULE & AREA_MODULE
//	26-oct-05  bhb	got rid of Module id #defines and added Module_ID enum
//	11-nov-05  bhb	split ModuleNode class to seperate files to eliminate ViewKit dependency
//	29-nov-06  bhb	add SIGAVG_MODULE
//	11-jun-08  bhb	made AModule subclass of Object, to all InvokeEvent for progress
//	23-jul-08  bhb	change ctor to use string name
//	06-feb-10  bhb	change base class to Subject, remove progress methods, vars
//	13-sep-12  bhb	add VOLCATH_MODULE
//	26-sep-12  bhb	change rd_ranges() name to readRangeFile(), changed exec_mod() sampint
//					from float * to float.
//	19-oct-12  bhb	change _outType to vector<out_type_t>, delete #define N_OUTPUTS
//					delete unused setOutType(), change exec_mod 1st, 3rd args from CData ** to
//					vector<CData *> and name to execMod()
//	31-oct-12  bhb	remove _nOutputs, setNumOutputs()
//	22-jan-13  bhb	change VOLCATH_PVR_MODULE to VOLCATH_OCL_MODULE
//	01-feb-13  bhb	add BEAT2_MODULE
//	12-feb-13  bhb	changed _inputs to vector, allow any number
//	19-aug-19  bhb	added comment for 'type' in struct AModControl
//	
#ifndef AMODULE_H
#define AMODULE_H

#include <Data/CData.h>				// dsDataType, dsValType
#include "ModuleControls.h"
#include <Patterns/Observer.h>		// Subject base class
#include <Fltk/ProgressCmd.h>
#include <vector>
#include <boost/shared_ptr.hpp>

// analysis group got bits defines
#define GOT_AG_NAME 1
#define GOT_AG_MODE 4
#define GOT_AG_CHAN 8
#define GOT_AG_CONTROLS 16

// analysis group status defines
#define AG_MODE_CHG 1
#define AG_CANCEL   2
#define AG_ACCEPT   3
#define AG_DELETE   4

// max data inputs & outputs for modules
#define N_INPUTS    3

//////////////////////////////////////////////////////////////////////////
//  analysis mode classes & structs
//////////////////////////////////////////////////////////////////////////
//
// for indexing Outputs array to select inputs for a module...
//
struct  data_loc
{
	int     modnum;						// index in the anal group of an AModule
	int     outnum;						// index of the output within that AModule
};
typedef struct data_loc data_loc_t;

struct  out_data_type
{
	dsDataType dtype;					// data type output
	dsValType  vtype;					// value type output
};
typedef struct out_data_type out_type_t;

struct AModControl
{
	int	type;			// AMC_TYPE of 'ctrl', only used in Amodule.cc
	union
	{
		unsigned char	uc;
		unsigned short	us;
		unsigned int	ui;
		float			fl;
		char			c;
		short			s;
		int				i;
	} ctrl;
};

// types for AModControl
enum AMC_TYPE {
	AMC_TYPE_UCHAR = 1,
	AMC_TYPE_USHORT,	//	2
	AMC_TYPE_UINT,		//	3
	AMC_TYPE_FLOAT,		//	4
	AMC_TYPE_CHAR,		//	5
	AMC_TYPE_SHORT,		//	6
	AMC_TYPE_INT		//	7
};

//////////////////////////////////////////////////////////////////////////
//  analysis module base class
//////////////////////////////////////////////////////////////////////////
class AModule : public bhb::Subject
{
	public:
		//
		//  built-in analysis module id #'s
		//
		enum Module_ID {
			DS_MODULE,			//  0: supply data set data (not actual module)
			BEAT_MODULE,		//  1: Beat find
			UPPA_MODULE,		//  2: uppa peak locations
			DVDT1_MODULE,		//  3: first derivitave
			SMOOTH_MODULE,		//  4: smooth data using moving average
			OPER1_MODULE,		//  5: operations on 1 set of input values
			OPER2_MODULE,		//  6: ops on 2 sets of input vals
			BACT_MODULE,		//  7: bipolar activation
			ACTIVITY_MODULE,	//  8: check for activity (rather than noise) in data
			QWAVE_MODULE,		//  9: find q-wave
			TWAVE_MODULE,		// 10: find t-wave
			OPVEC_MODULE,		// 11: vec_xy, vec_sc math ops
			SYSDIA_MODULE,		// 12: sys, diastolic times
			OPINT_MODULE,		// 13: ops in intervals
			OPPICK_MODULE,		// 14: pick input
			OPINTPICK_MODULE,	// 15: pick intervals from 2 inputs
			OPVEC2_MODULE,		// 16: two dataset vec_xy, vec_sc math ops
			PEAKINT_MODULE,		// 17: generate interval around peak, + peak
			OPRANGE_MODULE,		// 18: find max in a range
			FFT_MODULE,			// 19: find fft
			PWELCH_MODULE,		// 20: get power spectrum dist w/ Welch's method
			FILTER_MODULE,		// 21: Butterworth, Bessel, Chebyshev LP,HP,BP,BS filter module
			MAXFREQ_MODULE,		// 22: find Max frequency at a series of timesteps in data
			BASEV_MODULE,		// 23: find baseline voltage (flattest section) in signal
			AREA_MODULE,		// 24: find area under signal curve
			SIGAVG_MODULE,		// 25: signal average a succession of beats
			SPATIALAVG_MODULE,	// 26: Spatial average data using convolution with Gaussian kernel
			ACTPOTDUR_MODULE,	// 27: Action Potential duration	// qrst dir
			PVLOOP_MODULE,		// 28: Pressure/Volume loop
			VOLCATH_CH_MODULE,	// 29: Volume Catheter channel setup
			VOLCATH_DVDT_MODULE,// 30: Volume Catheter DVDTs
			VOLCATH_DATA_MODULE,// 31: Volume Catheter beat analysis
			VOLCATH_FLOW_MODULE,// 32: Volume Catheter flow analysis
			VOLCATH_TAU_MODULE,	// 33: Volume Catheter Tau analysis
			VOLCATH_OCL_MODULE,	// 34: Volume Catheter Occlusion analysis
			BEAT2_MODULE		// 35: Find max or min peaks (beats) in 1 or all chan
		};
		
		AModule( const std::string &name, const std::string &tip=0);
		AModule( const AModule &am);		// copy constructor
		virtual ~AModule();

		// access functions
		void        setId( int id)			{ _id = Module_ID(id); };
		Module_ID  	getId()					{ return _id;};
		std::string &	name()				{ return _name;	}
		int     	numInputs()				{ return _inputs.size();	}
		data_loc_t *getInput( int n)		{ return &_inputs[n];	}

		std::vector<AModControl> *	controls()	{ return &_controls;	}
		std::vector<AModControl> *	defCntrls()	{ return &_defCntrls;	}
		int			setControl( int n, int t, void *v);
		int			setControl( int ci, AModControl *ctrl);
		int			updateControl( int ci, AModControl *ctrl);
		AModControl *	control( int n)
						{ return (n < _controls.size()) ? &_controls[n] : NULL;	}
		AModControl *	defCntrl( int n)
						{ return (n < _defCntrls.size()) ? &_defCntrls[n] : NULL;	}
		void *		controlVal( int n);
		void *		defCntrlVal( int n);

		const int     	numOutputs() const	{ return _outType.size(); };
		dsDataType	outDType( int n)		{ return _outType[n].dtype; };
		dsValType 	outVType( int n)		{ return _outType[n].vtype; };
		const char *		tip()			{ return _tip.c_str();	}
		ModuleControls *	ctrlDlog()		{ return _ctrlDlog.get();	}
		bhb::ProgressCmd *	progressCmd()	{ return &_progressCmd;	}
		
		//	actions
		//	
		void    read(FILE *);
		void    write(FILE *);
		void	setControls( ModuleControls *c)	{ _ctrlDlog.reset(c);	}
		void	updateControls( std::vector<AModControl> *ctrls);
		void	resetControls();
		
		// virtual action functions
		virtual void showControls();
		virtual int  execMod( std::vector<CData *> &, std::vector<AModControl> *, 
								std::vector<CData *> &, std::vector<int> &, float) = 0;

	protected:
		Module_ID					_id;				// the module id #
		std::string					_name;
		int							_nInputs;			// number of inputs to module
		std::vector<data_loc_t>		_inputs;			// the inputs to the module
		std::vector<AModControl> 	_controls;			// controls for the module
		std::vector<AModControl> 	_defCntrls;			// default controls from analysis.modes
		std::vector<out_type_t>		_outType;			// data type of each output
		std::string					_tip;				// tooltip
		boost::shared_ptr<ModuleControls>	_ctrlDlog;
		static bhb::ProgressCmd		_progressCmd;
		
		int readRangeFile( char *f_name, float *min, float *max, float *step);

};

#endif
