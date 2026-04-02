//
//	GroupList.C - base class for Sample,Display & Analysis group lists
//
//	08-nov-2005  bhb
//	Modified:
//
//#include <Data/GroupList.h>

using namespace std;

//////////////////////////////////////////////////////////
//////////	GroupList     //////////////////////////////
//////////////////////////////////////////////////////////

template <class GR>
GroupList<GR>::GroupList()
{
}

template <class GR>
GroupList<GR>::GroupList( const GroupList &gl)
{
	_grList.assign( gl._grList.begin(), gl._grList.end());
}

template <class GR>
GroupList<GR>::~GroupList()
{
	free_all();
}

template <class GR>
void
GroupList<GR>::free_all()
{
	for ( typename list<GR *>::iterator i = _grList.begin(); i != _grList.end(); i++)
		delete (*i);
	_grList.erase( begin(), end());
}

template <class GR>
GR *
GroupList<GR>::elem( int i)
{
	typename list<GR *>::iterator g;

	if ( unsigned(i) < _grList.size())
	{
		for ( g = _grList.begin(); i > 0; i--, g++);
		return( *g);
	}
	else return NULL;
}
