/**
 * @file	SimpleConfig.h
 * @class	SimpleConfig
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/12/28
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_SIMPLECONFIG_H__
#define __JSCPPUTILS_SIMPLECONFIG_H__

#include "Common.h"

#include <map>
#include <string>

namespace JsCPPUtils
{

	class SimpleConfig
	{
	private:
		std::basic_string<JSCUTILS_TYPE_DEFCHAR> m_strFilePath;

	protected:
		std::map<std::basic_string<JSCUTILS_TYPE_DEFCHAR>, std::basic_string<JSCUTILS_TYPE_DEFCHAR> > m_configmap;

		static JSCUTILS_TYPE_DEFCHAR *_strtrim(JSCUTILS_TYPE_DEFCHAR *text);

	public:
		SimpleConfig();
		~SimpleConfig();
		
		int load(const JSCUTILS_TYPE_DEFCHAR *szFilePath = NULL);
		int save(const JSCUTILS_TYPE_DEFCHAR *szFilePath = NULL);
		
		bool isContain(const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& name);
		std::basic_string<JSCUTILS_TYPE_DEFCHAR> get(const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& name);
		void set(const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& name, const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& value);
		bool checkGet(const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& name, std::basic_string<JSCUTILS_TYPE_DEFCHAR>& refvalue);
	};

}

#endif /* __JSCPPUTILS_SIMPLECONFIG_H__ */
