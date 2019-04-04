/**
 * @file	JsSrcfileRegister.h
 * @class	JsSrcfileRegister
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/04/09
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_JSSRCFILEREGISTER_H__
#define __JSCPPUTILS_JSSRCFILEREGISTER_H__

#include <string>
#include <map>

namespace JsCPPUtils {
	class JsSrcfileRegisterBase
	{
	public:
		struct ModuleBase {
			std::basic_string<char> __JSFR_name;
		};

		typedef std::map<std::basic_string<char>, ModuleBase*> ModuleListBaseType;

		ModuleListBaseType m_modules;

	protected:
		JsSrcfileRegisterBase();

	protected:
		virtual ~JsSrcfileRegisterBase();
		void _register(ModuleBase *pModule);
		void _unregister(ModuleBase *pModule);
	};

}

#define _JsSrcfileRegisterBase_STRUCTNAME(MODULENAME) _JsSrcfileRegisterBase_MODULE_ ## MODULENAME
#define _JsSrcfileRegisterBase_STRUCTVARNAME(MODULENAME) __p_JsSrcfileRegisterBase_MODULE_ ## MODULENAME
#define _JsSrcfileRegisterBase_DEFTOSTRING(MODULENAME) #MODULENAME

#define __JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER) JsSrcfileRegisterImpl_ ## PROVIDER
#define __JSCPPUTILS_JSSRCFILEREGISTER_STRUCTNAME(PROVIDER, MODULENAME) _JsSrcfileRegisterModule_ ## PROVIDER ## _ ## MODULENAME

#define JsSrcfileRegister_PROVIDERCLASS(PROVIDER) _JsCPPUtils_private::__JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)
#define JsSrcfileRegister_PROVIDER(PROVIDER) _JsCPPUtils_private::__JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)*
#define JsSrcfileRegister_getProvider(PROVIDER) _JsCPPUtils_private::__JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)::getInstance()
#define JsSrcfileRegister_getModuleList(PROVIDERCLASS) (PROVIDERCLASS)->m_modules
#define JsSrcfileRegister_PROVIDERMODULETYPE(PROVIDER) JsSrcfileRegister_PROVIDERCLASS(PROVIDER)::Module*

#define JsSrcfileRegister_PROVIDER_DECLARE(PROVIDER, TDATASTRUCTURE) \
namespace _JsCPPUtils_private { \
	class __JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER) : public JsCPPUtils::JsSrcfileRegisterBase { \
	public: \
		struct Module : public ModuleBase { TDATASTRUCTURE data; Module(const TDATASTRUCTURE &_data) : data(_data){ } }; \
	private: \
		static __JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)* m_instance; \
		static void _init(){ if(!m_instance){ m_instance = new  __JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)(); } } \
	public: \
		static __JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)* getInstance(){ _init(); return m_instance; } \
		static void _Sregister(Module *pModule) { _init(); m_instance->_register(pModule); } \
		static void _Sunregister(Module *pModule) { m_instance->_unregister(pModule); } \
	}; \
}

#define JsSrcfileRegister_PROVIDER_DEFINE(PROVIDER) \
namespace _JsCPPUtils_private { \
	__JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER) *__JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)::m_instance = NULL; \
}

#define __JSCPPUTILS_JSSRCFILEREGISTER_BRACED_INIT_LIST(...) {__VA_ARGS__}

#define JsSrcfileRegister_REGISTER(PROVIDER, MODULENAME, INITIALIZER) \
namespace _JsCPPUtils_private { \
	static struct __JSCPPUTILS_JSSRCFILEREGISTER_STRUCTNAME(PROVIDER, MODULENAME) : public __JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)::Module \
	{ \
		__JSCPPUTILS_JSSRCFILEREGISTER_STRUCTNAME(PROVIDER, MODULENAME)() : \
		__JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)::Module(__JSCPPUTILS_JSSRCFILEREGISTER_BRACED_INIT_LIST INITIALIZER) \
		{ \
			this->__JSFR_name = _JsSrcfileRegisterBase_DEFTOSTRING(MODULENAME); \
			__JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)::_Sregister(this); \
			 \
		} \
		~__JSCPPUTILS_JSSRCFILEREGISTER_STRUCTNAME(PROVIDER, MODULENAME)() { \
			__JSCPPUTILS_JSSRCFILEREISTER_PROVIDERCLASS(PROVIDER)::_Sunregister(this); \
		} \
	 \
	} _JsSrcfileRegisterBase_STRUCTVARNAME(MODULENAME); \
}

#endif /* __JSCPPUTILS_JSSRCFILEREGISTER_H__ */
