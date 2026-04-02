//
//	AMode.cc - Analysis Mode class
//
//	22-feb-01  bhb	split from analmodes.cc
//	Modified:
//	01-mar-01  bhb	use pointer to AModule from GlasAnalyze list rather than
//					create new module each time needed by AMode
//  25-may-01  bhb	add AModeList class
//  07-nov-02  bhb	fixup for AModule w/ AModControl
//  11-nov-02  bhb	add updateControls()
//  10-dec-02  bhb	update to use AnalysisGrVk instead of GlasAnalysisGr
//  29-may-03  bhb	fixup updateControls to handle case where ag's and AMode's numMods differ
//	15-dec-04  bhb	add 0 check in copy constr, add id default in constr, check _id in 
//					copy constr
//  09-nov-05  bhb	now use AnalysisGr
//  27-oct-12  bhb	change _dataLoc, _dataTitle to vectors, remove _numOutputs
//	14-nov-12  bhb	in hasVxyOutput() also include ds_vec_vec_xy
//	27-sep-12  bhb	add VolCathDvdtMod, VolCathChanMod
//	19-oct-12  bhb	add VolCathBeatMod
//	30-oct-12  bhb	add PVLoopMod
//	28-nov-12  bhb	add VolCathTauMod
//	29-nov-12  bhb	add VolCathPVRMod
//	22-jan-13  bhb	change VolCathPVRMod to VolCathOclMod
//	01-feb-13  bhb	add Beat2Mod
//	04-feb-13  bhb	add AMode::getNumOutputs(), more error checking in AMode::read()
//	23-apr-13  bhb	change VolCathBeatMod name to VolCathDataMod
//	
#include "AMode.h"
#include "AModule.h"
#include <AModules/AModulesInc.h>
#include <Data/AnalysisGr.h>
#include <iostream>

using namespace std;

AMode::AMode( int id, const char *nm)
{
	_id = id;
	if ( nm != 0)
		setName( nm);
	else
		_name[0] = '\0';
	_numMods = 0;
}

// copy constructor
AMode::AMode( const AMode &am)
{
	if ( &am == 0 || am.id() < 0)
	{
		_name[0] = '\0';
		_numMods = 0;
		return;
	}
	_id = am.id();
	strcpy( _name, am.name());
	_numMods = am._numMods;
	for ( uint i = 0; i < _numMods; i++)
	{
		switch( am._modules[i]->getId())
		{
			case AModule::DS_MODULE:
				_modules[i] = new SelectDataMod(*(SelectDataMod *)am._modules[i]);	break;
			case AModule::BEAT_MODULE:
				_modules[i] = new BeatMod(*(BeatMod *)am._modules[i]);			break;
			case AModule::UPPA_MODULE:
				_modules[i] = new UppaMod(*(UppaMod *)am._modules[i]);			break;
			case AModule::DVDT1_MODULE:
				_modules[i] = new Dvdt1Mod(*(Dvdt1Mod *)am._modules[i]);		break;
			case AModule::SMOOTH_MODULE:
				_modules[i] = new SmoothMod(*(SmoothMod *)am._modules[i]);		break;
			case AModule::OPER1_MODULE:
				_modules[i] = new Oper1Mod(*(Oper1Mod *)am._modules[i]);		break;
			case AModule::OPER2_MODULE:
				_modules[i] = new Oper2Mod(*(Oper2Mod *)am._modules[i]);		break;
			case AModule::BACT_MODULE:
				_modules[i] = new BiActMod(*(BiActMod *)am._modules[i]);		break;
			case AModule::ACTIVITY_MODULE:
				_modules[i] = new ActMod(*(ActMod *)am._modules[i]);			break;
			case AModule::QWAVE_MODULE:
				_modules[i] = new QWaveMod(*(QWaveMod *)am._modules[i]);		break;
			case AModule::TWAVE_MODULE:
				_modules[i] = new TWaveMod(*(TWaveMod *)am._modules[i]);		break;
			case AModule::OPVEC_MODULE:
				_modules[i] = new OpVecMod(*(OpVecMod *)am._modules[i]);		break;
			case AModule::SYSDIA_MODULE:
				_modules[i] = new SysDiaMod(*(SysDiaMod *)am._modules[i]);		break;
			case AModule::OPINT_MODULE:
				_modules[i] = new OpIntMod(*(OpIntMod *)am._modules[i]);		break;
			case AModule::OPPICK_MODULE:
				_modules[i] = new OpPickMod(*(OpPickMod *)am._modules[i]);		break;
			case AModule::OPINTPICK_MODULE:
				_modules[i] = new OpIntPickMod(*(OpIntPickMod *)am._modules[i]);	break;
			case AModule::OPVEC2_MODULE:
				_modules[i] = new OpVec2Mod(*(OpVec2Mod *)am._modules[i]);		break;
			case AModule::PEAKINT_MODULE:
				_modules[i] = new PeakIntMod(*(PeakIntMod *)am._modules[i]);	break;
			case AModule::OPRANGE_MODULE:
				_modules[i] = new OpRangeMod(*(OpRangeMod *)am._modules[i]);	break;
			case AModule::FFT_MODULE:
				_modules[i] = new FftMod(*(FftMod *)am._modules[i]);			break;
			case AModule::PWELCH_MODULE:
				_modules[i] = new PwelchMod(*(PwelchMod *)am._modules[i]);		break;
			case AModule::FILTER_MODULE:
				_modules[i] = new FilterMod(*(FilterMod *)am._modules[i]);		break;
			case AModule::MAXFREQ_MODULE:
				_modules[i] = new MaxFreqMod(*(MaxFreqMod *)am._modules[i]);	break;
			case AModule::BASEV_MODULE:
				_modules[i] = new BaseMod(*(BaseMod *)am._modules[i]);			break;
			case AModule::AREA_MODULE:
				_modules[i] = new AreaMod(*(AreaMod *)am._modules[i]);			break;
			case AModule::SIGAVG_MODULE:
				_modules[i] = new SignalAvgMod(*(SignalAvgMod *)am._modules[i]); break;
			case AModule::SPATIALAVG_MODULE:
				_modules[i] = new SpatialMod(*(SpatialMod *)am._modules[i]); break;
			case AModule::ACTPOTDUR_MODULE:
				_modules[i] = new ActPotDurationMod(*(ActPotDurationMod *)am._modules[i]); break;
			case AModule::PVLOOP_MODULE:
				_modules[i] = new PVLoopMod(*(PVLoopMod *)am._modules[i]); break;
			case AModule::VOLCATH_CH_MODULE:
				_modules[i] = new VolCathChanMod(*(VolCathChanMod *)am._modules[i]); break;
			case AModule::VOLCATH_DVDT_MODULE:
				_modules[i] = new VolCathDvdtMod(*(VolCathDvdtMod *)am._modules[i]); break;
			case AModule::VOLCATH_DATA_MODULE:
				_modules[i] = new VolCathDataMod(*(VolCathDataMod *)am._modules[i]); break;
			case AModule::VOLCATH_FLOW_MODULE:
				_modules[i] = new VolCathFlowMod(*(VolCathFlowMod *)am._modules[i]); break;
			case AModule::VOLCATH_TAU_MODULE:
				_modules[i] = new VolCathTauMod(*(VolCathTauMod *)am._modules[i]); break;
			case AModule::VOLCATH_OCL_MODULE:
				_modules[i] = new VolCathOclMod(*(VolCathOclMod *)am._modules[i]); break;
			case AModule::BEAT2_MODULE:
				_modules[i] = new Beat2Mod(*(Beat2Mod *)am._modules[i]); break;
			default:
				cerr << "AMode read: INVALID MODULE" << endl;;
				break;
		}
	}
	int numOutputs = am.numOutputs();
	for ( uint i = 0; i < numOutputs; i++)
	{
		_dataLoc.push_back( *am.dataLoc(i));
		string title = am.dataTitle(i);
		_dataTitle.push_back( title);
	}
}

AMode::~AMode()
{
	for ( int i=0; i<_numMods; i++)
		delete _modules[i];
}

//
//	Read an analysis mode from analysis.modes file
//	
bool
AMode::read( FILE *fp)
{
	int id;
	uint i;
	
	sscanf(skipComments(fp), "TITLE:\t%[^\n]", _name);
	sscanf(checkOldNames(fp), "NUM MODULES:\t%d\n", &_numMods);
	sscanf(skipComments(fp), "\tMOD\tINPUTS\t\tCONTROLS\t\tOUTPUTS\n");
	sscanf(skipComments(fp), "\t---\t------\t\t--------\t\t-------\n");

	for ( i=0; i<_numMods; i++)
	{
		fscanf(fp, "%d", &id);
		switch ( id)
		{
			case AModule::DS_MODULE:
				_modules[i] = new SelectDataMod();		break;
			case AModule::BEAT_MODULE:
				_modules[i] = new BeatMod();			break;
			case AModule::UPPA_MODULE:
				_modules[i] = new UppaMod();			break;
			case AModule::DVDT1_MODULE:
				_modules[i] = new Dvdt1Mod();			break;
			case AModule::SMOOTH_MODULE:
				_modules[i] = new SmoothMod();			break;
			case AModule::OPER1_MODULE:
				_modules[i] = new Oper1Mod();			break;
			case AModule::OPER2_MODULE:
				_modules[i] = new Oper2Mod();			break;
			case AModule::BACT_MODULE:
				_modules[i] = new BiActMod();			break;
			case AModule::ACTIVITY_MODULE:
				_modules[i] = new ActMod();				break;
			case AModule::QWAVE_MODULE:
				_modules[i] = new QWaveMod();			break;
			case AModule::TWAVE_MODULE:
				_modules[i] = new TWaveMod();			break;
			case AModule::OPVEC_MODULE:
				_modules[i] = new OpVecMod();			break;
			case AModule::SYSDIA_MODULE:
				_modules[i] = new SysDiaMod();			break;
			case AModule::OPINT_MODULE:
				_modules[i] = new OpIntMod();			break;
			case AModule::OPPICK_MODULE:
				_modules[i] = new OpPickMod();			break;
			case AModule::OPINTPICK_MODULE:
				_modules[i] = new OpIntPickMod();		break;
			case AModule::OPVEC2_MODULE:
				_modules[i] = new OpVec2Mod();			break;
			case AModule::PEAKINT_MODULE:
				_modules[i] = new PeakIntMod();			break;
			case AModule::OPRANGE_MODULE:
				_modules[i] = new OpRangeMod();			break;
			case AModule::FFT_MODULE:
				_modules[i] = new FftMod();				break;
			case AModule::PWELCH_MODULE:
				_modules[i] = new PwelchMod();			break;
			case AModule::FILTER_MODULE:
				_modules[i] = new FilterMod();			break;
			case AModule::MAXFREQ_MODULE:
				_modules[i] = new MaxFreqMod();			break;
			case AModule::BASEV_MODULE:
				_modules[i] = new BaseMod();			break;
			case AModule::AREA_MODULE:
				_modules[i] = new AreaMod();			break;
			case AModule::SIGAVG_MODULE:
				_modules[i] = new SignalAvgMod();		break;
			case AModule::SPATIALAVG_MODULE:
				_modules[i] = new SpatialMod();			break;
			case AModule::ACTPOTDUR_MODULE:
				_modules[i] = new ActPotDurationMod();	break;
			case AModule::PVLOOP_MODULE:
				_modules[i] = new PVLoopMod();			break;
			case AModule::VOLCATH_CH_MODULE:
				_modules[i] = new VolCathChanMod();		break;
			case AModule::VOLCATH_DVDT_MODULE:
				_modules[i] = new VolCathDvdtMod();		break;
			case AModule::VOLCATH_DATA_MODULE:
				_modules[i] = new VolCathDataMod();		break;
			case AModule::VOLCATH_FLOW_MODULE:
				_modules[i] = new VolCathFlowMod();		break;
			case AModule::VOLCATH_TAU_MODULE:
				_modules[i] = new VolCathTauMod();		break;
			case AModule::VOLCATH_OCL_MODULE:
				_modules[i] = new VolCathOclMod();		break;
			case AModule::BEAT2_MODULE:
				_modules[i] = new Beat2Mod();			break;
			default:
				cerr << "AMode read: INVALID MODULE" << endl;
				return 0;
				break;
		}

		_modules[i]->read( fp);
		_modules[i]->setId( id);
	}
//	cout << "AMode::read: " << _name << ", " << this << endl;

	if ( i != _numMods)
		return false;
	int numOutputs = getNumOutputs( fp);
	if ( numOutputs == 0)
		return false;
	for ( uint i=0; i<numOutputs; i++)
	{
		data_loc_t dl;
		char str[64];
		sscanf( skipComments(fp), "%d,%d\t%[^\n]", &dl.modnum, &dl.outnum, str);
		_dataLoc.push_back( dl);
		string title = str;
		_dataTitle.push_back( title);
	}
	sscanf(skipComments(fp), "***ENDMODE***");
	return true;
}

/* skips lines that begin with ';' */
char *
AMode::skipComments( FILE *fp)
{
	static char file_line[256];

	file_line[0] = '\0';
	do
	{
		fscanf(fp, "%[^\n]\n", file_line);
	} while (file_line[0] == ';');
	return(file_line);
}

char *
AMode::checkOldNames( FILE *fp)
{
	char oldname[32];
	
	char * fl = skipComments( fp);
	while ( strstr( fl, "OLD TITLE:"))
	{
		sscanf( fl, "OLD TITLE:\t%[^\n]", oldname);
		_oldNames.push_back( string(oldname));
		fl = skipComments( fp);
	}
	return fl;
}

int
AMode::getNumOutputs( FILE *fp)
{
	int numOutputs = 0;
	char * fl = skipComments( fp);
	if ( strstr( fl, "OUTPUTS:") != 0)
		sscanf( fl, "OUTPUTS:\t%d", &numOutputs);
	return numOutputs;
}

// Init this AMode's module control values from ag
void
AMode::initControls( AnalysisGr *ag)
{
	if ( ag->mode() != id())
		return;
	for ( int i = 0; i < numMods(); i++)
	{
		vector<AModControl>	*ctrls = ag->controls( i);
		int j = 0;
		for ( vector<AModControl>::iterator ci = ctrls->begin(); ci != ctrls->end(); j++,ci++)
			module(i)->setControl( j, &(*ci));
	}
}

// Update this AMode's module control values from ag
// For case where old ag's controls may be outof date
// 
void
AMode::updateControls( AnalysisGr *ag)
{
	if ( ag->mode() != id())
		return;
	// check that ag's Mode has same number of Modules
	if ( ag->numMods() == numMods())
	{	// use ag's control vals
		for ( int i = 0; i < numMods(); i++)
		{
			module(i)->resetControls();		// init from defaults, also sets correct number of ctrls
			module(i)->updateControls( ag->controls( i));
			// keep ag's controls in sync
			ag->updateControls( i, module(i)->controls());
		}
	}
	else	// number of Modules differ, use defaults and fix ag
	{
		ag->setnumMods( numMods());			// fix ag's _numMods
		for ( int i = 0; i < numMods(); i++)
		{
			module(i)->resetControls();		// init from defaults, also sets correct number of ctrls
			ag->updateControls( i, module(i)->controls());	
		}	
	}
}

//
//	return true if an output is vxy type
//	
bool	AMode::hasVxyOutput()
{
	for ( int i=0; i < numOutputs(); i++)
	{
		data_loc_t *	loc = dataLoc(i);
		dsDataType		dtype = module(loc->modnum)->outDType(loc->outnum);
		if ( (dtype == ds_vec_xy) || (dtype == ds_vec_vec_xy))
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
//
//	AModeList class
//	
//////////////////////////////////////////////////////////////////////////
AModeList::~AModeList()
{
	// delete AModes in list
	for ( vector<AMode *>::iterator ami = _amlist.begin(); ami != _amlist.end(); ami++)
	{
//		cout << "~AModeList: " << (*ami)->name() << ", " << *ami << endl;
		delete *ami;
	}
}

AMode *
AModeList::amode( int i)
{
	for ( vector<AMode *>::iterator ami = _amlist.begin(); ami != _amlist.end(); ami++)
		if ( (*ami)->id() == i)
			return *ami;
	return 0;
}

AMode *
AModeList::amode( const char *str)
{
	vector<AMode *>::iterator ami;

	if ( str != 0)
	{
		for ( ami = _amlist.begin(); ami != _amlist.end(); ami++)
		{
			if ( strcmp( str, (*ami)->name()) == 0 )
			{
				return (*ami);
			}
		}
		//
		//	Not found yet - if any old names check them
		//	
		for ( ami = _amlist.begin(); ami != _amlist.end(); ami++)
		{
			for ( vector<string>::iterator oni = (*ami)->oldNames().begin(); 
					oni != (*ami)->oldNames().end(); oni++)
				if ( strcmp( str, (*oni).c_str()) == 0)
					return (*ami);
		}
	}
	return 0;
}

//
//	Read analysis.modes file - note has GUI stuff, should be broken up
//	
int
AModeList::read( string &errMsg)
{
	FILE *fp;
	char fname[256], *path, *name;
	AMode *am;
	int numAModes = 0;
	
	for ( vector<AMode *>::iterator ami = _amlist.begin(); ami != _amlist.end(); ami++)
		delete (*ami);
	_amlist.clear();
	
	path = getenv("GLAS_ANALYSIS_PATH");
	if ( path == 0)
	{
		errMsg = "'GLAS_ANALYSIS_PATH' environmental variable not defined";
		return 0;
	}
	name = getenv("GLAS_ANALMODE_FILE");
	if ( name == 0)
	{
		errMsg = "'GLAS_ANALMODE_FILE' environmental variable not defined";
		return 0;
	}
	sprintf( fname, "%s/%s", path, name);
	if ((fp = (FILE *)fopen( fname, "r")) != 0)
	{
		sscanf(skipComments(fp), "NUM MODES:\t%d\n", &numAModes);
		for ( int i=0; i<numAModes; i++)
		{
			am = new AMode( i);
			if ( !am->read(fp))
			{
				errMsg = "Error reading analysis.modes file";
				return 0;
			}
			_amlist.push_back( am);
		}
		fclose( fp);
	}
	else    errMsg = "Error opening analysis.modes file";
	return numAModes;
}

// skips lines that begin with ';'
char *
AModeList::skipComments( FILE *fp)
{
	static char file_line[256];

	file_line[0] = '\0';
	do
	{
		fscanf(fp, "%[^\n]\n", file_line);
	} while (file_line[0] == ';');
	return(file_line);
}
