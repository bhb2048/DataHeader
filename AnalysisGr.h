//
//	AnalysisGr.h - Analysis Group class
//
//	AG channels like SG chan are physical chan #'s.
//	This was done so that AG's were not dependent on SG's except that AG chan must be subset
//	of SG chan of course.  This allows AG's to be used w/ different SG's, eg w/ diff sample
//	rates.
//
//	22-feb-2001  bhb	split outof DataHeader.h
//	Modified:
//	01-mar-01  bhb	added controls(), beautified names
//	03-mar-01  bhb	remove copy(), make copy constuctor
//	07-nov-01  bhb	removed AgList::edit()
//	20-sep-02  bhb	added copy (=) operator
//	08-oct-02  bhb	added AgReadErr enum, _err
//	07-nov-02  bhb	added AModControl
//	11-nov-02  bhb	added updateControls()
//	29-may-03  bhb	added setnumMods()
//	07-nov-05  bhb	make _dir, _file1, _file2 string type, modify setDir() to append
//					'/' if missing
//	09-nov-05  bhb	merge w/ AnalysisGrVk & AgListVk
//	27-oct-12  bhb	change _dataTitle to vector<string>, remove _numOuts, 
//					remove #define MAX_AMODE_OUTPUTS
//	08-dec-15  bhb	get rid of notifyCmd() calls, add _errStr, getError(), clearError()
//	09-dec-15  bhb	delete AgList::_err
//	15-dec-15  bhb	add haveError() both AnalysisGr & AgList
//	
#ifndef ANALYSISGR_H
#define ANALYSISGR_H

#include <Data/ChannelSet.h>
#include <Data/AModule.h>
#include <list>
#include <vector>
#include <string>
#include <fstream>

#define CUR_AG_VERSION 5

#define MAX_MODULES 16

#define MAXPASS     4
#define DATA_TITLE_LEN  16

class AgList;
class AMode;
class AModeList;
class DataHeader;

////////////////////////////////////////////////////////////////////////
//	Analysis Group Classes
////////////////////////////////////////////////////////////////////////
//
// ChannelGr::_mode == -1 if no AMode for AG, else is AMode Id
// 
class AnalysisGr : public ChannelSet
{
	public:
		AnalysisGr( AgList * p=0);
		AnalysisGr( const AnalysisGr &ag);		// make a copy
		~AnalysisGr();

		enum AgReadErr { noErr, versionErr, nameErr, modErr, ctrlErr, outErr, chgModeErr,
						 chgCtrlErr, numCtrlErr };
		
		// access functions
		void			setParent( AgList *p) 		{ _parent = p;}
		AgList  *		Parent()   					{ return _parent;}
		void    		setDir( const std::string &d)	{ setDir( d.c_str());	}
		void    		setDir( const char *d);
		const char *	Dir()	const				{ return _dir.c_str(); }
		void			setFile1( const std::string &f)	{ setFile1( f.c_str());	}
		void    		setFile1( const char *f)	{ _file1 = f;}
		const char *	File1() const				{ return _file1.c_str(); }
		void			setFile2( const std::string &f)	{ setFile2( f.c_str());	}
		void			setFile2( const char *f)	{ _file2 = f;}
		const char *	File2() const				{ return _file2.c_str(); }
		
		void    		setnScaleVal( int n) 		{ _nScVal = n; }
		const int		nScaleVal() const			{ return _nScVal; }
		void			setScaleVal( int i, float v) { _scVal[i] = v; }
		float *			scaleVal()					{ return _scVal; }
		const float		scaleVal( int i) const		{ return _scVal[i]; }

		const char *	modeName()					{ return _modeName.c_str();	}
		void			modeName( const char *nm)	{ _modeName = nm;	}
		
		const int		numMods() const     		{ return _numMods;}
		void			setnumMods( int n)			{ _numMods = n;	}
		AModControl *	control( int m, int c)	
						{	return ( m < _numMods && c < _controls[m].size()) ?
							&_controls[m][c] : NULL;
						}
		void *			controlVal( int m, int c);
		void			updateControls( int i, std::vector<AModControl> *ctrls);
		std::vector<AModControl>	* controls( int i)		{ return &_controls[i];	}
		const std::vector<AModControl> *	controls( int i) const { return &_controls[i];}
		const int		numOuts() const 		{ return _dataTitle.size(); }
		void			setDataTitle( const char *s)
						{ std::string title = s; _dataTitle.push_back( title);	}
		const char *	dataTitle( int i)	const	{ return _dataTitle[i].c_str();	}
		int &			count()		{ return _count;	}
		AMode *			amode()		{ return _AMode;	}

		bool			haveError()	{ return _errStr.length() > 0;	}
		std::string &	getError()	{ return _errStr;	}
		void			clearError()	{ _errStr = "";	}
		
		// action functions
		int     		index();
		int				setControls();
		int				setControls( AMode * am);
		bool			initFromAMode();
		bool			setAMode( AMode *am, bool fromAM=false);

		virtual bool read( std::ifstream &inFile, uint ag_ver, int hdr_ver);
		virtual bool write( std::ofstream &outFile);
		
		AnalysisGr& operator = ( const AnalysisGr &ag);

	protected:
		AgList *			_parent;
		int					_count;				// times used - for naming
		AMode *				_AMode;				// our Analysis Mode

		std::string	_dir;						// mapfile path	
		std::string	_file1;						// picture file name
		std::string	_file2;						// electrode locations file name

		std::string	_modeName;

		/* should these be in scale module?	*/
		int     _nScVal;						// number of scale values
		float   _scVal[ 20];					// mapping scale values

		int     _numMods;
		std::vector<AModControl>	_controls[MAX_MODULES];	// controls for each module
		std::vector<std::string>	_dataTitle;

		std::string		_errStr;
		
		int				setControl( int m, int ci, int t, void *v);
		int				setControl( int m, int ci, AModControl *ctrl);
		bool			checkAMode();
		const char * 	oldmode_newmode(int oldmode);
};


class AgList
{
	public:
		AgList( DataHeader *hdr=NULL);
		AgList( const AgList &agl);
		virtual ~AgList();

		enum AgListReadErr { noErr, unknModeErr, modErr, ctrlErr, readAgErr };
		
		void			free_all();
		AnalysisGr *	elem( int i);

		void    		setParent( DataHeader *hdr)	{ _parent = hdr;	}
		DataHeader *	Parent()					{ return _parent;	}
		std::list<AnalysisGr *> *	agList()		{ return &_list;	}
		std::list<AnalysisGr *>::iterator begin()	{ return _list.begin();	}
		std::list<AnalysisGr *>::iterator end()		{ return _list.end();	}
		int 			size()						{ return _list.size();	}
		AModeList *		AModes()					{ return _AModes;	}

		bool			setAModes( AModeList *am = 0);
		int				index( AnalysisGr *ag);
		int				getCounts( std::vector<int> *cnts);
		void			setCounts( std::vector<int> *cnts);
		uint			version()	{ return _version;	}

		bool			haveError()	{ return _errStr.length() > 0;	}
		std::string &	getError()	{ return _errStr;	}
		void			setError( std::string &err)	{ _errStr = err;	}
		void			clearError()	{ _errStr = "";	}

		virtual bool read( std::ifstream &inFile, int hdr_ver);
		virtual bool write( std::ofstream &outFile);

	protected:
		DataHeader *			_parent;
		std::list<AnalysisGr *>	_list;
		AModeList *				_AModes;	// for initializing EditAgDlog option menu
		bool					_freeAModes;// true if this class created _AModes & needs to free
		uint					_version;

		std::string				_errStr;
};
#endif
