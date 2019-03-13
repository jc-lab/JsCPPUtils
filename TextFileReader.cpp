/**
 * @file	TextFileReader.cpp
 * @class	TextFileReader
 * @author	Jichan (development@jc-lab.net)
 * @date	2018/07/23
 * @brief	TextFileReader
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#include "MemoryBuffer.h"
#include "StringEncoding.h"

#include "TextFileReader.h"

namespace JsCPPUtils
{
	const unsigned char _TextFileReaderUtils::_JSTFS_UTF_firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

#define UTF_SUR_HIGH_START  (uint32_t)0xD800
#define UTF_SUR_HIGH_END    (uint32_t)0xDBFF
#define UTF_SUR_LOW_START   (uint32_t)0xDC00
#define UTF_SUR_LOW_END     (uint32_t)0xDFFF
#define UTF_MAX_LEGAL_UTF32   (uint32_t)0x0010FFFF
#define UTF_REPLACEMENT_CHAR   (uint32_t)0x0000FFFD

	int _TextFileReaderUtils::_u32_to_utf8(uint32_t ch, char *utf8dat)
	{
		int utf8len = 0;
		char *putf8ptr;
	
		uint32_t byteMask = 0xBF;
		uint32_t byteMark = 0x80;

		if(ch < 0x80)
			utf8len = 1;
		else if(ch < 0x800)
			utf8len = 2;
		else if(ch < 0x10000)
			utf8len = 3;
		else if(ch <= UTF_MAX_LEGAL_UTF32)
			utf8len = 4;
		else{
			utf8len = 3;
			ch = UTF_REPLACEMENT_CHAR;
		}

		putf8ptr = &utf8dat[utf8len];

		switch(utf8len)
		{
			case 4: *--putf8ptr = (char)((ch | byteMark) & byteMask); ch >>= 6;
			case 3: *--putf8ptr = (char)((ch | byteMark) & byteMask); ch >>= 6;
			case 2: *--putf8ptr = (char)((ch | byteMark) & byteMask); ch >>= 6;
			case 1: *--putf8ptr = (char) (ch | _JSTFS_UTF_firstByteMark[utf8len]);
		}

		return utf8len;
	}
	
	int _TextFileReaderUtils::_utf32_to_utf8(uint32_t ch, char *utf8dat)
	{
		uint32_t byteMask = 0xBF;
		uint32_t byteMark = 0x80;

		if (ch >= UTF_SUR_HIGH_START && ch <= UTF_SUR_LOW_END) {
			return -1;
		}

		return _u32_to_utf8(ch, utf8dat);
	}

	int _TextFileReaderUtils::_utf16_to_utf8(uint16_t ch, char *utf8dat)
	{
		uint32_t byteMask = 0xBF;
		uint32_t byteMark = 0x80;
		uint32_t x = ((uint32_t)ch) & 0x0000FFFF;

		if (x >= UTF_SUR_HIGH_START && x <= UTF_SUR_HIGH_END) {
			return -1;
		}

		return _u32_to_utf8(x, utf8dat);
	}

	const char *_TextFileReaderUtils::_removeBlanks(const char *cszbuf, int *premainlen)
	{
		while((*premainlen > 0) && ((*cszbuf == '\r') || (*cszbuf == '\n')))
		{
			cszbuf++;
			(*premainlen)--;
		}
		return cszbuf;
	}

}
