///////////////////////////////////////////////////////////////
//
//	ModuleControls.h  - abstract base class for AModule control dialogs
//
//	16-nov-05  bhb
//	Modified:
//	
//////////////////////////////////////////////////////////////

#ifndef MODULECONTROLS_H
#define MODULECONTROLS_H

class AModule;

class ModuleControls
{
	public:

		ModuleControls();
		virtual ~ModuleControls();

		void	setMod( AModule *mod)		{ _mod = mod;	}
		virtual void show() = 0;	// subclass must override, this is where controls are set
		
	protected:
		AModule *	_mod;			// initialized in ModuleNode::createModuleControls()

};
#endif
