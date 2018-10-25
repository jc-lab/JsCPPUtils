/**
 * @file	StringEncoding.h
 * @class	StringEncoding
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/01
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "Common.h"

#if defined(JSCUTILS_OS_WINDOWS) && JSCUTILS_OS_WINDOWS

#include <string>
#include <exception>
#include <tchar.h>

#include "SmartPointer.h"
#include "StringBuffer.h"

namespace JsCPPUtils
{

	class StringEncoding
	{
	public:
		static StringBuffer<char> StringEncoding::StringAToUTF8SB(const std::basic_string<char>& strInput);
		static StringBuffer<char> StringEncoding::StringWToUTF8SB(const std::basic_string<wchar_t>& strInput);
		static StringBuffer<char> StringEncoding::StringWToUTF8SB(const wchar_t* strInput, int len);
		static std::basic_string<char> UTF8ToStringA(const char *szUTF8Input, int len = -1);
		static std::basic_string<wchar_t> UTF8ToStringW(const char *szUTF8Input, int len = -1);
		static std::basic_string<char> UnicodeToStringA(const wchar_t *uncodeInput, int len = -1);
		static void UTF8ToStringABufferAppend(const char *szUTF8Input, int len, StringBuffer<char> &stringBuffer);
		static void UTF8ToStringWBufferAppend(const char *szUTF8Input, int len, StringBuffer<wchar_t> &stringBuffer);
		static void UnicodeToStringABufferAppend(const wchar_t *uncodeInput, int len, StringBuffer<char> &stringBuffer);
		
		static StringBuffer<char> StringToUTF8SB(const std::basic_string<char>& strInput);
		static StringBuffer<char> StringToUTF8SB(const std::basic_string<wchar_t>& strInput);
		static StringBuffer<char> StringToUTF8SB(const wchar_t* strInput, int len);
		static std::basic_string<wchar_t> StringToUnicode(const std::basic_string<char>& strInput);
		static std::basic_string<wchar_t> StringToUnicode(const std::basic_string<wchar_t>& strInput);
		static std::basic_string<TCHAR> UTF8ToString(const char *szUTF8Input, int len = -1);
		static std::basic_string<TCHAR> UTF8ToString(const std::basic_string<char>& strInput);
		static std::basic_string<TCHAR> UnicodeToString(const wchar_t *uncodeInput, int len = -1);
		static std::basic_string<TCHAR> MultibyteToString(const char *szInput, int len = -1);
		static std::basic_string<TCHAR> MultibyteToString(std::basic_string<char> strInput);
		static std::basic_string<char> StringToMultibyte(std::basic_string<wchar_t> strInput);
		static std::basic_string<char> StringToMultibyte(std::basic_string<char> strInput);
		static void UTF8ToStringBufferAppend(const char *szUTF8Input, int len, StringBuffer<char> &stringBuffer);
		static void UTF8ToStringBufferAppend(const char *szUTF8Input, int len, StringBuffer<wchar_t> &stringBuffer);

		template <typename inputT, typename retT> class AutoMWStringToString
		{
		};

		template <typename inputT> class AutoMWStringToString<inputT, char>
		{
		public:
			static std::basic_string<char> encode(const std::basic_string<inputT> &strInput)
			{
				return StringToMultibyte(strInput);
			}

			static std::basic_string<char> encode(const char* strInput, int len = -1)
			{
				return std::basic_string<char>(strInput, len);
			}

			static std::basic_string<char> encode(const wchar_t* strInput, int len = -1)
			{
				return UnicodeToStringA(strInput, len);
			}
		};

		template <typename inputT> class AutoMWStringToString<inputT, wchar_t>
		{
		public:
			static std::basic_string<wchar_t> encode(const std::basic_string<inputT> &strInpu)
			{
				return StringToUnicode(strInput);
			}

			static std::basic_string<wchar_t> encode(const char* strInput, int len = -1)
			{
				return MultibyteToUnicode(strInput, len);
			}

			static std::basic_string<wchar_t> encode(const wchar_t* strInput, int len = -1)
			{
				return std::basic_string<wchar_t>(strInput, len);
			}
		};
	};

}

#endif /* JSCUTILS_OS_WINDOWS */
