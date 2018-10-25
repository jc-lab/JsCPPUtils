/**
 * @file	StringEncoding.h
 * @class	StringEncoding
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/01
 * @brief	StringEncoding
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "StringEncoding.h"

#if defined(JSCUTILS_OS_WINDOWS) && JSCUTILS_OS_WINDOWS

#include <stdlib.h>
#include <string.h>
#include <Windows.h>

namespace JsCPPUtils
{
	
	StringBuffer<char> StringEncoding::StringAToUTF8SB(const std::basic_string<char>& strInput)
	{
		int n;
		int wLen;
		int outLen;
		wchar_t *wszOutput;
		StringBuffer<char> sbResult;

		wLen = MultiByteToWideChar(CP_ACP, 0, strInput.c_str(), strInput.length(), NULL, NULL);
		n = sizeof(wchar_t)*(wLen + 1);
		wszOutput = (wchar_t*)malloc(n);
		if(wszOutput == NULL)
		{
			throw std::bad_alloc();
			return sbResult;
		}
		memset(wszOutput, 0, n);
		MultiByteToWideChar(CP_ACP, 0, strInput.c_str(), strInput.length(), wszOutput, wLen);
		
		outLen = WideCharToMultiByte(CP_UTF8, 0, wszOutput, wLen, NULL, 0, NULL, NULL);
		sbResult.clear();
		sbResult.increaseBuffer(outLen);
		WideCharToMultiByte(CP_UTF8, 0, wszOutput, wLen, sbResult.getBuffer(), outLen, NULL, NULL);
		sbResult.increasePos(outLen);
		free(wszOutput);

		return sbResult;
	}

	StringBuffer<char> StringEncoding::StringWToUTF8SB(const std::basic_string<wchar_t>& strInput)
	{
		int n;
		int outLen;
		StringBuffer<char> sbResult;

		outLen = WideCharToMultiByte(CP_UTF8, 0, strInput.c_str(), strInput.length(), NULL, 0, NULL, NULL);
		sbResult.increaseBuffer(outLen);
		WideCharToMultiByte(CP_UTF8, 0, strInput.c_str(), strInput.length(), sbResult.getBuffer(), outLen, NULL, NULL);
		sbResult.increasePos(outLen);

		return sbResult;
	}

	StringBuffer<char> StringEncoding::StringWToUTF8SB(const wchar_t* strInput, int len)
	{
		int n;
		int outLen;
		StringBuffer<char> sbResult;

		outLen = WideCharToMultiByte(CP_UTF8, 0, strInput, len, NULL, 0, NULL, NULL);
		sbResult.increaseBuffer(outLen);
		WideCharToMultiByte(CP_UTF8, 0, strInput, len, sbResult.getBuffer(), outLen, NULL, NULL);
		sbResult.increasePos(outLen);

		return sbResult;
	}

	std::basic_string<char> StringEncoding::UTF8ToStringA(const char *szUTF8Input, int len)
	{
		int n;
		int wLen;
		int outLen;
		wchar_t *wszOutput;
		char *szOutput;

		std::string strOutput;

		wLen = MultiByteToWideChar(CP_UTF8, 0, szUTF8Input, len, NULL, NULL);
		n = sizeof(wchar_t)*(wLen + 1);
		wszOutput = (wchar_t*)malloc(n);
		if(wszOutput == NULL)
		{
			throw std::bad_alloc();
		}
		memset(wszOutput, 0, n);
		MultiByteToWideChar(CP_UTF8, 0, szUTF8Input, len, wszOutput, wLen);

		outLen = WideCharToMultiByte(CP_ACP, 0, wszOutput, wLen, NULL, 0, NULL, NULL);
		n = outLen + 1;
		szOutput = (char*)malloc(n);
		if(szOutput == NULL)
		{
			throw std::bad_alloc();
		}
		memset(szOutput, 0, n);
		WideCharToMultiByte(CP_ACP, 0, wszOutput, wLen, szOutput, outLen, NULL, NULL);

		free(wszOutput);

		strOutput = std::string(szOutput);

		free(szOutput);

		return strOutput;
	}

	std::basic_string<wchar_t> StringEncoding::UTF8ToStringW(const char *szUTF8Input, int len)
	{
		int n;
		int wLen;
		wchar_t *wszOutput;

		std::basic_string<wchar_t> strOutput;

		wLen = MultiByteToWideChar(CP_UTF8, 0, szUTF8Input, len, NULL, NULL);
		n = sizeof(wchar_t)*(wLen + 1);
		wszOutput = (wchar_t*)malloc(n);
		if(wszOutput == NULL)
		{
			throw std::bad_alloc();
		}
		memset(wszOutput, 0, n);
		MultiByteToWideChar(CP_UTF8, 0, szUTF8Input, len, wszOutput, wLen);

		strOutput = wszOutput;
		free(wszOutput);

		return strOutput;
	}

	std::basic_string<char> StringEncoding::UnicodeToStringA(const wchar_t *wszInput, int len)
	{
		int n;
		int outLen;
		char *szOutput;
		std::string strOutput;

		outLen = WideCharToMultiByte(CP_ACP, 0, wszInput, len, NULL, 0, NULL, NULL);
		n = outLen + 1;
		szOutput = (char*)malloc(n);
		if(szOutput == NULL)
		{
			throw std::bad_alloc();
		}
		memset(szOutput, 0, n);
		WideCharToMultiByte(CP_ACP, 0, wszInput, len, szOutput, outLen, NULL, NULL);

		strOutput = std::string(szOutput);

		free(szOutput);

		return strOutput;
	}
	
	void StringEncoding::UTF8ToStringABufferAppend(const char *szUTF8Input, int len, StringBuffer<char> &stringBuffer)
	{
		int n;
		int wLen;
		int outLen;
		wchar_t *wszOutput;
		char *szOutput;

		wLen = MultiByteToWideChar(CP_UTF8, 0, szUTF8Input, len, NULL, NULL);
		n = sizeof(wchar_t)*wLen;
		wszOutput = (wchar_t*)malloc(n);
		if(wszOutput == NULL)
		{
			throw std::bad_alloc();
		}
		memset(wszOutput, 0, n);
		MultiByteToWideChar(CP_UTF8, 0, szUTF8Input, len, wszOutput, wLen);

		outLen = WideCharToMultiByte(CP_ACP, 0, wszOutput, wLen, NULL, 0, NULL, NULL);
		n = outLen + 1;
		szOutput = (char*)malloc(n);
		if(szOutput == NULL)
		{
			throw std::bad_alloc();
		}
		memset(szOutput, 0, n);
		WideCharToMultiByte(CP_ACP, 0, wszOutput, wLen, szOutput, outLen, NULL, NULL);

		free(wszOutput);

		stringBuffer.append(szOutput, outLen);

		free(szOutput);
	}
	
	void StringEncoding::UTF8ToStringWBufferAppend(const char *szUTF8Input, int len, StringBuffer<wchar_t> &stringBuffer)
	{
		int n;
		int wLen;
		int outLen;
		wchar_t *wszOutput;
		char *szOutput;

		wLen = MultiByteToWideChar(CP_UTF8, 0, szUTF8Input, len, NULL, NULL);
		n = sizeof(wchar_t)*wLen;
		wszOutput = (wchar_t*)malloc(n);
		if(wszOutput == NULL)
		{
			throw std::bad_alloc();
		}
		memset(wszOutput, 0, n);
		MultiByteToWideChar(CP_UTF8, 0, szUTF8Input, len, wszOutput, wLen);

		stringBuffer.append(wszOutput, wLen);

		free(wszOutput);
	}

	void StringEncoding::UnicodeToStringABufferAppend(const wchar_t *wszInput, int len, StringBuffer<char> &stringBuffer)
	{
		int n;
		int outLen;
		char *szOutput;
		std::string strOutput;

		outLen = WideCharToMultiByte(CP_ACP, 0, wszInput, len, NULL, 0, NULL, NULL);
		n = outLen + 1;
		szOutput = (char*)malloc(n);
		if(szOutput == NULL)
		{
			throw std::bad_alloc();
		}
		memset(szOutput, 0, n);
		WideCharToMultiByte(CP_ACP, 0, wszInput, len, szOutput, outLen, NULL, NULL);

		strOutput = std::string(szOutput);
		stringBuffer.append(szOutput);

		free(szOutput);
	}
	
	std::basic_string<wchar_t> StringEncoding::StringToUnicode(const std::basic_string<char>& strInput)
	{
		int wLen;
		int outLen;
		wchar_t *wszOutput;
		std::basic_string<wchar_t> strOutput;

		wLen = MultiByteToWideChar(CP_ACP, 0, strInput.c_str(), strInput.length(), NULL, NULL);
		wszOutput = new wchar_t[wLen + 1];
		memset(wszOutput, 0, sizeof(wchar_t) * (wLen + 1));
		MultiByteToWideChar(CP_ACP, 0, strInput.c_str(), strInput.length(), wszOutput, wLen);
		strOutput = std::basic_string<wchar_t>(wszOutput, wLen);
		delete wszOutput;

		return strOutput;
	}

	std::basic_string<wchar_t> StringEncoding::StringToUnicode(const std::basic_string<wchar_t>& strInput)
	{
		return strInput;
	}
	
	StringBuffer<char> StringEncoding::StringToUTF8SB(const std::basic_string<char>& strInput)
	{
		return StringAToUTF8SB(strInput);
	}
	
	StringBuffer<char> StringEncoding::StringToUTF8SB(const std::basic_string<wchar_t>& strInput)
	{
		return StringWToUTF8SB(strInput);
	}

	StringBuffer<char> StringEncoding::StringToUTF8SB(const wchar_t* strInput, int len)
	{
		return StringWToUTF8SB(strInput, len);
	}

	std::basic_string<TCHAR> StringEncoding::UTF8ToString(const char *szUTF8Input, int len)
	{
#ifdef _UNICODE
		return UTF8ToStringW(szUTF8Input, len);
#else
		return UTF8ToStringA(szUTF8Input, len);
#endif
	}

	std::basic_string<TCHAR> StringEncoding::UTF8ToString(const std::basic_string<char>& strInput)
	{
#ifdef _UNICODE
		return UTF8ToStringW(strInput.c_str(), strInput.length());
#else
		return UTF8ToStringA(strInput.c_str(), strInput.length());
#endif
	}

	std::basic_string<TCHAR> StringEncoding::UnicodeToString(const wchar_t *unicodeInput, int len)
	{
#ifdef _UNICODE
		if(len < 0)
		{
			len = wcslen(unicodeInput);
		}
		return std::basic_string<wchar_t>(unicodeInput, len);
#else
		return UnicodeToStringA(unicodeInput, len);
#endif
	}
	
	std::basic_string<TCHAR> StringEncoding::MultibyteToString(const char *szInput, int len)
	{
#ifdef _UNICODE
#if 1
		std::basic_string<char> szTemp = szInput;
		std::basic_string<wchar_t> w(szTemp.begin(), szTemp.end());
		return w;
#else
		int n;
		int outLen;
		wchar_t *wzOutput;

		std::basic_string<TCHAR> strOutput;

		outLen = MultiByteToWideChar(CP_ACP, 0, szInput, len, NULL, 0);
		n = outLen + 1;
		wzOutput = (wchar_t*)malloc(n * sizeof(wchar_t));
		if(wzOutput == NULL)
		{
			throw std::bad_alloc();
		}
		memset(wzOutput, 0, n);
		MultiByteToWideChar(CP_ACP, 0, szInput, len, wzOutput, outLen);

		strOutput = std::basic_string<TCHAR>(wzOutput/*, outLen*/);
		free(wzOutput);

		return strOutput;
#endif
#else
		return std::basic_string<char>(szInput, len);
#endif
	}
	
	std::basic_string<TCHAR> StringEncoding::MultibyteToString(std::basic_string<char> strInput)
	{
		return MultibyteToString(strInput.c_str(), strInput.length());
	}
	
	std::basic_string<char> StringEncoding::StringToMultibyte(std::basic_string<wchar_t> strInput)
	{
		return UnicodeToStringA(strInput.c_str(), strInput.length());
	}

	std::basic_string<char> StringEncoding::StringToMultibyte(std::basic_string<char> strInput)
	{
		return strInput;
	}
	
	void StringEncoding::UTF8ToStringBufferAppend(const char *szUTF8Input, int len, StringBuffer<char> &stringBuffer)
	{
		UTF8ToStringABufferAppend(szUTF8Input, len, stringBuffer);
	}

	void StringEncoding::UTF8ToStringBufferAppend(const char *szUTF8Input, int len, StringBuffer<wchar_t> &stringBuffer)
	{
		UTF8ToStringWBufferAppend(szUTF8Input, len, stringBuffer);
	}
}

#endif /* JSCUTILS_OS_WINDOWS */
