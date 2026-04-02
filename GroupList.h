//
//	GroupList.h - base class for Sample,Display & Analysis group lists
//
//	08-nov-2005  bhb
//	Modified:
//	
#ifndef GROUPLIST_H
#define GROUPLIST_H

#include <list>

template <class GR>
class GroupList
{
	public:
		GroupList();
		virtual ~GroupList();

		void    free_all();
		GR * 	elem( int i);

		typename std::list<GR *>::iterator	begin()	{ return _grList.begin();	}
		typename std::list<GR *>::iterator	end()	{ return _grList.end();	}
		std::size_t			size()	{ return _grList.size();	}
		void				push_back(GR *g)	{ _grList.push_back(g);	}
		void				clear()	{ free_all();	}
		virtual int read( FILE *fp, int version) = 0;
		virtual int write( FILE *fp) = 0;

	private:
		std::list<GR *>	_grList;

		GroupList(const GroupList &gl);		// don't allow copying since just copies
											// pointers and can mess up destruction
};

#include <Data/GroupList.C>
#endif
