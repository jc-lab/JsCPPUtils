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

#pragma once

#include <string>

#include "Common.h"
#include "StringBuffer.h"
#include "MemoryBuffer.h"

#include <map>
#include "TextFileReader.h"

namespace JsCPPUtils
{
	class basic_ConfigFile : public basic_TextFileReader
	{
	private:
		std::map<std::string, std::string> m_configs;
		virtual void onReadLine(char *utf8Text, int utf8Length);

		char *_trim(char *input, int *plen);

	public:
		basic_ConfigFile();
		virtual ~basic_ConfigFile();
		bool isContain(const std::string& key);
		std::string get(const std::string& key, const std::string& defaultValue = "", bool *bExists = NULL);
		void set(const std::string& key, const std::string& value);
		int getInt(const std::string& key, int defaultValue = 0, bool *bExists = NULL);
	};

}