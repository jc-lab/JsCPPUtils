/**
 * @file	SimpleConfig.h
 * @class	SimpleConfig
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/12/28
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "SimpleConfig.h"
#include <errno.h>
#include <stdio.h>

#include "StringBuffer.h"

namespace JsCPPUtils
{

	SimpleConfig::SimpleConfig()
	{
	}


	SimpleConfig::~SimpleConfig()
	{
	}

	int SimpleConfig::load(const JSCUTILS_TYPE_DEFCHAR *szFilePath)
	{
		FILE *fp = NULL;
		JSCUTILS_TYPE_ERRNO eno = 0;
		JSCUTILS_TYPE_DEFCHAR linebuf[384];

		if(szFilePath == NULL && m_strFilePath.empty())
			return 0;
		else if(szFilePath != NULL)
			m_strFilePath = szFilePath;

#if defined( _MSC_VER )
#if( _MSC_VER < 1310) // < vs2003
		fp = fopen(m_strFilePath.c_str(), "rt");
#elif( _MSC_VER < 1400) // < vs2005
		fp = _tfopen(m_strFilePath.c_str(), _T("rt"));
#else // >= vs2005
		eno = _tfopen_s(&fp, m_strFilePath.c_str(), _T("rt"));
#endif
#else
		fp = fopen(m_strFilePath.c_str(), "rt");
#endif
		if ((eno == 0) && (fp == NULL))
		{
			eno = errno;
		}
		if(eno != 0)
		{
			return -eno;
		}


#if defined( _MSC_VER )
#if( _MSC_VER < 1310) // < vs2003
		while (fgets(linebuf, sizeof(linebuf) / sizeof(linebuf[0]), fp))
#else
		while (_fgetts(linebuf, sizeof(linebuf) / sizeof(linebuf[0]), fp))
#endif
#else
		while (fgets(linebuf, sizeof(linebuf) / sizeof(linebuf[0]), fp))
#endif
		{
			JSCUTILS_TYPE_DEFCHAR *szToken = NULL;
			JSCUTILS_TYPE_DEFCHAR *szContext = NULL;

			StringBuffer<JSCUTILS_TYPE_DEFCHAR> tmpsb;

#if defined( _MSC_VER )
#if( _MSC_VER < 1310) // < vs2003 (maybe not work)
			szToken = strtok_s(linebuf, "=", &szContext);
#else
			szToken = _tcstok_s(linebuf, _T("="), &szContext);
#endif
#else
			szToken = strtok_r(linebuf, "=", &szContext);
#endif
			szToken = _strtrim(szToken);
			szContext = _strtrim(szContext);
			if((szToken[0] == ';') || (szToken[0] == '#'))
				continue;

			tmpsb = szToken;
			tmpsb.replaceToLower();

			m_configmap[tmpsb.toString()] = szContext;
		}

		fclose(fp);

		return 1;
	}

	int SimpleConfig::save(const JSCUTILS_TYPE_DEFCHAR *szFilePath)
	{
		FILE *fp = NULL;
		JSCUTILS_TYPE_ERRNO eno;

		if(szFilePath == NULL && m_strFilePath.empty())
			return 0;
		else if(szFilePath != NULL)
			m_strFilePath = szFilePath;

#if defined( _MSC_VER )
#if( _MSC_VER < 1310) // < vs2003
		fp = fopen(m_strFilePath.c_str(), "rt");
#elif( _MSC_VER < 1400) // < vs2005
		fp = _tfopen(m_strFilePath.c_str(), _T("wt"));
#else // >= vs2005
		eno = _tfopen_s(&fp, m_strFilePath.c_str(), _T("wt"));
#endif
#else
		fp = fopen(m_strFilePath.c_str(), "wt");
#endif
		if ((eno == 0) && (fp == NULL))
		{
			eno = errno;
		}
		if(eno != 0)
		{
			return -eno;
		}

		for (std::map<std::basic_string<JSCUTILS_TYPE_DEFCHAR>, std::basic_string<JSCUTILS_TYPE_DEFCHAR> >::iterator iter = m_configmap.begin(); iter != m_configmap.end(); iter++)
		{
#if defined( _MSC_VER )
#if( _MSC_VER < 1310) // < vs2003
			fprintf(fp, _T"%s=%s\r\n", iter->first.c_str(), iter->second.c_str());
#else
			_ftprintf(fp, _T("%s=%s\r\n"), iter->first.c_str(), iter->second.c_str());
#endif
#else
			fprintf(fp, "%s=%s\r\n", iter->first.c_str(), iter->second.c_str());
#endif
		}

		fclose(fp);

		return 1;
	}

	JSCUTILS_TYPE_DEFCHAR *SimpleConfig::_strtrim(JSCUTILS_TYPE_DEFCHAR *text)
	{
		size_t len;
		if(text == NULL)
			return NULL;

#if defined( _MSC_VER )
#if( _MSC_VER < 1310) // < vs2003
		len = strlen(text);
#else
		len = _tcslen(text);
#endif
#else
		len = strlen(text);
#endif
		
		while((len > 0) && ((text[0] == '\r') || (text[0] == '\n') || (text[0] == ' ') || (text[0] == '\t')))
		{
			len--;
			text++;
		}

#if defined( _MSC_VER )
#if( _MSC_VER < 1310) // < vs2003
		len = strlen(text);
#else
		len = _tcslen(text);
#endif
#else
		len = strlen(text);
#endif
		while((len > 0) && ((text[len - 1] == '\r') || (text[len - 1] == '\n') || (text[len - 1] == ' ') || (text[len - 1] == '\t')))
		{
			len--;
			text[len] = 0;
		}

		return text;
	}
		
	bool SimpleConfig::isContain(const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& name)
	{
		std::map<std::basic_string<JSCUTILS_TYPE_DEFCHAR>, std::basic_string<JSCUTILS_TYPE_DEFCHAR> >::iterator iter;
		StringBuffer<JSCUTILS_TYPE_DEFCHAR> sbname(name);
		sbname.replaceToLower();
		iter = m_configmap.find(sbname.toString());
		return (iter != m_configmap.end());
	}

	std::basic_string<JSCUTILS_TYPE_DEFCHAR> SimpleConfig::get(const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& name)
	{
		std::map<std::basic_string<JSCUTILS_TYPE_DEFCHAR>, std::basic_string<JSCUTILS_TYPE_DEFCHAR> >::iterator iter;
		StringBuffer<JSCUTILS_TYPE_DEFCHAR> sbname(name);
		sbname.replaceToLower();
		iter = m_configmap.find(sbname.toString());
		if(iter == m_configmap.end())
			return std::basic_string<JSCUTILS_TYPE_DEFCHAR>();
		else
			return iter->second;
	}

	void SimpleConfig::set(const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& name, const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& value)
	{
		StringBuffer<JSCUTILS_TYPE_DEFCHAR> sbname(name);
		sbname.replaceToLower();
		m_configmap[sbname.toString()] = value;
	}
	
	bool SimpleConfig::checkGet(const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& name, std::basic_string<JSCUTILS_TYPE_DEFCHAR>& refvalue)
	{
		std::map<std::basic_string<JSCUTILS_TYPE_DEFCHAR>, std::basic_string<JSCUTILS_TYPE_DEFCHAR> >::iterator iter;
		StringBuffer<JSCUTILS_TYPE_DEFCHAR> sbname(name);
		sbname.replaceToLower();
		iter = m_configmap.find(sbname.toString());
		if(iter == m_configmap.end())
			return false;
		else
		{
			refvalue = iter->second;
			return true;
		}
	}

}
