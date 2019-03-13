/**
 * @file	ConfigFile.cpp
 * @class	ConfigFile
 * @author	Jichan (development@jc-lab.net)
 * @date	2018/07/23
 * @brief	ConfigFile
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "ConfigFile.h"

namespace JsCPPUtils
{
	basic_ConfigFile::basic_ConfigFile(){}
	basic_ConfigFile::~basic_ConfigFile(){}
	bool basic_ConfigFile::isContain(const std::string& key)
	{
		std::map<std::string, std::string>::iterator iter = m_configs.find(key);
		return (iter != m_configs.end());
	}
	std::string basic_ConfigFile::get(const std::string& key, const std::string& defaultValue, bool *bExists)
	{
		std::map<std::string, std::string>::iterator iter = m_configs.find(key);
		if (iter != m_configs.end())
		{
			if (bExists)
				*bExists = true;
			return iter->second;
		} else {
			if (bExists)
				*bExists = false;
			return std::string();
		}
	}
	void basic_ConfigFile::set(const std::string& key, const std::string& value)
	{
		m_configs[key] = value;
	}
	int basic_ConfigFile::getInt(const std::string& key, int defaultValue, bool *bExists)
	{
		bool exists = false;
		std::string value = get(key, "", &exists);
		if (bExists)
			*bExists = exists;
		if (exists)
		{
			return atoi(value.c_str());
		} else {
			return defaultValue;
		}
	}

	char *basic_ConfigFile::_trim(char *input, int *plen)
	{
		while ((*plen > 0) && ((input[0] == ' ') || (input[0] == '\r') || (input[0] == '\n') || (input[0] == '\t')))
		{
			input++;
			(*plen)--;
		}
		while ((*plen > 0) && ((input[*plen - 1] == ' ') || (input[*plen - 1] == '\r') || (input[*plen - 1] == '\n') || (input[*plen - 1] == '\t')))
		{
			(*plen)--;
			input[*plen] = 0;
		}
		return input;
	}
	void basic_ConfigFile::onReadLine(char *utf8Text, int utf8Length)
	{
		int pos = 0;
		int vallen = 0;
		char *ptoken = NULL;
		utf8Text = _trim(utf8Text, &utf8Length);
		if (utf8Length <= 0)
			return;
		if ((utf8Text[0] == ';') || (utf8Text[0] == '#'))
			return;
		while (pos < utf8Length)
		{
			if (utf8Text[pos] == '=')
			{
				ptoken = &utf8Text[pos + 1];
				vallen = utf8Length - (pos + 1);
				utf8Text[pos] = 0;
				utf8Text = _trim(utf8Text, &pos);
				ptoken = _trim(ptoken, &vallen);
				break;
			}
			pos++;
		}
		if (ptoken)
			m_configs[utf8Text] = std::string(ptoken, vallen);
		else
			m_configs[utf8Text] = "";
	}
}
