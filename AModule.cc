//
//
//	AModule.cc -- analysis module class
//
//	13-oct-92	jwp
//	Version: 2.3
//	Modified:
//	31-jan-94  bhb	V2.0 c++: AMode & AModule classes
//	11-feb-94  bhb	add Samp_int param to exec_mod()
//	17-apr-96  bhb	added real SmoothMod
//	03-may-96  bhb	added Anal_modes, Num_modes to glasAnalyze class
//					many loose c functions became glasAnalyze member fcns
//	22-feb-01  bhb	renamed from analmodes.cc
//	07-nov-02  bhb	revise to use AModControl
//	11-nov-05  bhb	split ModuleNode class to seperate files to eliminate ViewKit dependency
//	02-feb-10  bhb	added cerr outputs in setControl(), updateControl()
//  19-oct-12  bhb	rename exec_mod to execMod, change 1st, 3rd params to vector<Cdata *>
//
#include "AModule.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <CXlib/cxlib.h>

using namespace std;
using namespace boost;
using namespace bhb;

// static
ProgressCmd	AModule::_progressCmd;

//////////////////////////////////////////////////////////////////////////
//  class AModule functions
//////////////////////////////////////////////////////////////////////////
AModule::AModule( const string &name, const string &tip) : _name(name), _tip(tip)
{
}

AModule::AModule( const AModule &am)
{
	int i;

	_id = am._id;
	_name = am._name;
	_inputs = am._inputs;

	vector<AModControl>::const_iterator ci = am._controls.begin();
	vector<AModControl>::const_iterator di = am._defCntrls.begin();
	for ( ; ci != am._controls.end(); ci++, di++)
	{
		_controls.push_back( *ci);
		_defCntrls.push_back( *di);
	}
	int nOutputs = am.numOutputs();
	for ( i=0; i<nOutputs; i++)
	{
		out_type_t ot;
		ot.dtype = am._outType[i].dtype;
		ot.vtype = am._outType[i].vtype;
		_outType.push_back( ot);
	}
	_tip = am._tip;
	_ctrlDlog = am._ctrlDlog;		// share any Control dialog
}

AModule::~AModule()
{
}

int
AModule::setControl( int ci, int t, void *v)
{
	if ( _controls.size() <= ci)
	{
		cerr << "AModule::setControl: error control index to large" << endl;
		return 0;
	}
	if ( v == NULL)
	{
		cerr << "AModule::setControl: error NULL control value pointer" << endl;
		return 0;
	}
	AModControl *c = &(_controls[ci]);
	if ( c->type != t)
	{
		cerr << "AModule::setControl: error control type doesn't match" << endl;
		return 0;
	}
	switch (t)
	{
		case AMC_TYPE_UCHAR:
			c->ctrl.uc = *((unsigned char *)v);
			break;
		case AMC_TYPE_USHORT:
			c->ctrl.us = *((unsigned short *)v);
			break;
		case AMC_TYPE_UINT:
			c->ctrl.ui = *((unsigned int *)v);
			break;
		case AMC_TYPE_FLOAT:
			c->ctrl.fl = *((float *)v);
			break;
		case AMC_TYPE_CHAR:
			c->ctrl.c = *((char *)v);
			break;
		case AMC_TYPE_SHORT:
			c->ctrl.s = *((short *)v);
			break;
		case AMC_TYPE_INT:
			c->ctrl.i = *((int *)v);
			break;
		default:
			return 0;
	}
	return 1;
}

int
AModule::setControl( int ci, AModControl *ctrl)
{
	if ( _controls.size() <= ci)
	{
		cerr << "AModule::setControl: error control index to large" << endl;
		return 0;
	}
	if ( ctrl == NULL)
	{
		cerr << "AModule::setControl: error NULL control pointer" << endl;
		return 0;
	}
	AModControl *c = &(_controls[ci]);
	*c = *ctrl;
	return 1;
}

//	for case where current control may be different from arg
int
AModule::updateControl( int ci, AModControl *ctrl)
{
	if ( _controls.size() <= ci)
	{
		cerr << "AModule::updateControl: error control index to large" << endl;
		return 0;
	}
	if ( ctrl == NULL)
	{
		cerr << "AModule::updateControl: error NULL control pointer" << endl;
		return 0;
	}
	AModControl *c = &(_controls[ci]);
	switch ( c->type)
	{
		case AMC_TYPE_UCHAR:
			c->ctrl.uc = (ctrl->type != AMC_TYPE_FLOAT) ?
							ctrl->ctrl.uc : (unsigned char)ctrl->ctrl.fl;
			break;
		case AMC_TYPE_USHORT:
			c->ctrl.us = (ctrl->type != AMC_TYPE_FLOAT) ?
							ctrl->ctrl.us : (unsigned short)ctrl->ctrl.fl;
			break;
		case AMC_TYPE_UINT:
			c->ctrl.ui = (ctrl->type != AMC_TYPE_FLOAT) ?
							ctrl->ctrl.ui : (unsigned int)ctrl->ctrl.fl;
			break;
		case AMC_TYPE_FLOAT:
			c->ctrl.fl = (ctrl->type != AMC_TYPE_FLOAT) ?
							(float)ctrl->ctrl.i : ctrl->ctrl.fl;
			break;
		case AMC_TYPE_CHAR:
			c->ctrl.c = (ctrl->type != AMC_TYPE_FLOAT) ?
							ctrl->ctrl.c : (char)ctrl->ctrl.fl;
			break;
		case AMC_TYPE_SHORT:
			c->ctrl.s = (ctrl->type != AMC_TYPE_FLOAT) ?
							ctrl->ctrl.s : (short)ctrl->ctrl.fl;
			break;
		case AMC_TYPE_INT:
			c->ctrl.i = (ctrl->type != AMC_TYPE_FLOAT) ?
							ctrl->ctrl.i : (int)ctrl->ctrl.fl;
			break;
	}
	return 1;
}

//
// returns pointer to control value
//
void *
AModule::controlVal( int c)
{
	if ( _controls.size() <= c)
		return NULL;
	return (void *)(&_controls[c].ctrl.fl);
}

void *
AModule::defCntrlVal( int c)
{
	if ( _defCntrls.size() <= c)
		return NULL;
	return (void *)(&_defCntrls[c].ctrl.fl);
}

//
//	read module line from analysis.modes
//
void
AModule::read( FILE *fp)
{
	int h, n, j, nInputs, nOutputs;
	char rest_of_line[128];

	// read inputs
	_inputs.clear();
	fscanf( fp,  "%d:%[^\n]\n", &nInputs, rest_of_line);
	for (h=0; h<nInputs; h++)
	{
		data_loc_t dataloc;
		sscanf(rest_of_line, "%d,%d %[^\n]", &dataloc.modnum, &dataloc.outnum, rest_of_line);
		_inputs.push_back( dataloc);
	}

	// read controls
	_controls.clear();
	_defCntrls.clear();

	struct AModControl c;
	char *sp ,*endptr;
	sp = rest_of_line;
	n = strtoul( sp, &endptr, 10);		// num controls
	sp = endptr+1;						// skip colen
	for ( j = 0; j < n; j++)
	{
		c.type = strtoul( sp, &endptr, 10);	// get val type
		sp = endptr;
		switch ( c.type)
		{
			case AMC_TYPE_UCHAR:
				c.ctrl.uc = (unsigned char)strtoul( sp, &endptr, 10);
				break;
			case AMC_TYPE_USHORT:
				c.ctrl.us = (unsigned short)strtoul( sp, &endptr, 10);
				break;
			case AMC_TYPE_UINT:
				c.ctrl.ui = (unsigned int)strtoul( sp, &endptr, 10);
				break;
			case AMC_TYPE_FLOAT:
				c.ctrl.fl = (float)strtod( sp, &endptr);
				break;
			case AMC_TYPE_CHAR:
				c.ctrl.c = (char)strtoul( sp, &endptr, 10);
				break;
			case AMC_TYPE_SHORT:
				c.ctrl.s = (short)strtoul( sp, &endptr, 10);
				break;
			case AMC_TYPE_INT:
				c.ctrl.i = (int)strtoul( sp, &endptr, 10);
				break;
			default:
//				_err = ctrlErr;
//				throw _err;
				break;
		}
		sp = endptr;
		_controls.push_back( c);
		_defCntrls.push_back( c);
	}

	// read outputs
	_outType.clear();
	sscanf( sp, "%d:%[^\n]", &nOutputs, rest_of_line);
	for ( h=0; h<nOutputs; h++)
	{
		out_type_t ot;
		sscanf( rest_of_line, "%d,%d %[^\n]",
			(int *)(&ot.dtype), (int *)(&ot.vtype), rest_of_line);
		_outType.push_back(ot);
	}
}

void
AModule::write(FILE *fp)
{
	int h;

	fprintf(fp, "%d ", _id);
	for (h=0; h < numInputs(); h++)
		fprintf(fp, "%d,%d ", _inputs[h].modnum, _inputs[h].outnum);

	vector<AModControl>::iterator ci;
	fprintf( fp, "%d ", _controls.size());
	for ( ci = _controls.begin(); ci != _controls.end(); ci++)
	{
		fprintf(fp, "%d ", (*ci).type);
		switch ( (*ci).type)
		{
			case AMC_TYPE_UCHAR:
				fprintf(fp, "%d ", (*ci).ctrl.uc);
				break;
			case AMC_TYPE_USHORT:
				fprintf(fp, "%d ", (*ci).ctrl.us);
				break;
			case AMC_TYPE_UINT:
				fprintf(fp, "%d ", (*ci).ctrl.ui);
				break;
			case AMC_TYPE_FLOAT:
				fprintf(fp, "%f ", (*ci).ctrl.fl);
			case AMC_TYPE_CHAR:
				fprintf(fp, "%d ", (*ci).ctrl.c);
				break;
			case AMC_TYPE_SHORT:
				fprintf(fp, "%d ", (*ci).ctrl.s);
				break;
			case AMC_TYPE_INT:
				fprintf(fp, "%d ", (*ci).ctrl.i);
				break;
				break;
		}
	}

	// Outputs???
	fprintf(fp, "\n");
}

void
AModule::updateControls( vector<AModControl> *ctrls)
{
	int j = 0;
	for ( vector<AModControl>::iterator ci = controls()->begin(); ci != controls()->end(); j++,ci++)
		updateControl( j, &(*ctrls)[j]);
}

void
AModule::resetControls()
{
	vector<AModControl>::iterator ci;
	_controls.clear();
	for ( ci = _defCntrls.begin(); ci != _defCntrls.end(); ci++)
		_controls.push_back( *ci);
}

//
// Reads the range files for a module. Not currently called anywhere.
//
int
AModule::readRangeFile( char *f_name, float *min, float *max, float *step)
{
	FILE    *fp;
	int     num_controls, i;
	int     control;
	char    str[128];

	fp = (FILE *)fopen(f_name, "r");
	if (fp != NULL)
	{
		fscanf(fp, "MODULE CONTROL RANGES\n---------------------\n");
		fscanf(fp, "NUM CONTROLS:\t%d\n", &num_controls);
		fscanf(fp, "\nCONTROL\t\tMIN\tMAX\tSTEP\n-------\t\t---\t---\t----\n");

		for (i=0; i<num_controls; i++)
		{
			fscanf(fp, "%d", &control);
			fscanf(fp, "%f %f %f\n", &min[control], &max[control], &step[control]);
		}
		return 1;
	}
	else
	{
		sprintf( str, "rd_ranges: error opening %s", f_name);
		message( str);
		return 0;
	}
}

void
AModule::showControls()
{
	if ( _ctrlDlog != NULL)
		_ctrlDlog->show();
}
