#include "JsSrcfileRegister.h"

namespace JsCPPUtils {

	JsSrcfileRegisterBase::JsSrcfileRegisterBase()
	{
	}

	JsSrcfileRegisterBase::~JsSrcfileRegisterBase()
	{
	}

	void JsSrcfileRegisterBase::_register(ModuleBase *pModule)
	{
		m_modules[pModule->__JSFR_name] = pModule;
	}

	void JsSrcfileRegisterBase::_unregister(ModuleBase *pModule)
	{
		std::map<std::basic_string<char>, ModuleBase*>::iterator iter;
		iter = m_modules.find(pModule->__JSFR_name);
		if (iter != m_modules.end())
		{
			m_modules.erase(iter);
		}
	}

}
