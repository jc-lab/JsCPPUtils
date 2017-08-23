/**
 * @file	StringBuffer.h
 * @class	StringBuffer
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/09/29
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_STRINGBUFFER_H__
#define __JSCPPUTILS_STRINGBUFFER_H__

#include <stdlib.h>
#include <string.h>
#include <string>
#include <typeinfo>
#include <math.h>

#include "Common.h"

namespace JsCPPUtils
{

	template<typename CHARTYPE>
	class StringBuffer
	{
	private:
		size_t m_bufsize;
		size_t m_bufpos;
		CHARTYPE *m_pbuf;
		int m_blocksize;

		bool putData(const CHARTYPE *data, size_t size)
		{
			size_t needsize = m_bufpos + size + 1;
			if(needsize > m_bufsize)
			{
				CHARTYPE *pnewptr;
				size_t newsize = needsize;
				if(newsize % m_blocksize)
					newsize += m_blocksize - (newsize % m_blocksize);
				pnewptr = (CHARTYPE*)realloc(m_pbuf, newsize * sizeof(CHARTYPE));
				if(pnewptr == NULL)
				{
					return false;
				}
				m_bufsize = newsize;
				m_pbuf = pnewptr;
			}

			memcpy(&m_pbuf[m_bufpos], data, size * sizeof(CHARTYPE));
			m_bufpos += size;
			m_pbuf[m_bufpos] = 0;

			return true;
		}

	public:
		StringBuffer() : 
		m_blocksize(128),
			m_bufsize(0),
			m_bufpos(0),
			m_pbuf(NULL)
		{
		}

		StringBuffer(bool bconfig, int blocksize) : 
		m_blocksize(128),
			m_bufsize(0),
			m_bufpos(0),
			m_pbuf(NULL)
		{
			if(bconfig)
				m_blocksize = blocksize;
		}

		StringBuffer(const CHARTYPE *szText) : 
		m_blocksize(128),
			m_bufsize(0),
			m_bufpos(0),
			m_pbuf(NULL)
		{
			append(szText);
		}

		StringBuffer(const CHARTYPE *szText, int len): 
		m_blocksize(128),
			m_bufsize(0),
			m_bufpos(0),
			m_pbuf(NULL)
		{
			append(szText, len);
		}

		StringBuffer(const std::basic_string<CHARTYPE>& strText): 
		m_blocksize(128),
			m_bufsize(0),
			m_bufpos(0),
			m_pbuf(NULL)
		{
			append(strText.c_str(), strText.length());
		}

		StringBuffer(const StringBuffer& sbText): 
		m_blocksize(sbText.m_blocksize),
			m_bufsize(0),
			m_bufpos(0),
			m_pbuf(NULL)
		{
			append(sbText);
		}

#if __cplusplus >= 201103L
		// move constructor (C++11)
		StringBuffer(StringBuffer&& sbText): 
		m_blocksize(sbText.m_blocksize),
			m_bufsize(0),
			m_bufpos(0),
			m_pbuf(NULL)
		{
			m_bufsize = sbText.m_bufsize;
			m_bufpos = sbText.m_bufpos;
			m_pbuf = sbText.m_pbuf;
			sbText.m_bufsize = 0;
			sbText.m_bufpos = 0;
			sbText.m_pbuf = NULL;
		}
#endif

		~StringBuffer()
		{
			if(m_pbuf != NULL)
			{
				free(m_pbuf);
				m_pbuf = NULL;
			}
			m_bufsize = 0;
			m_bufpos = 0;

		}

		void clear()
		{
			m_bufpos = 0;
		}

		void append(const CHARTYPE *szAppend, int len)
		{
			if(len < 0)
			{
				if(typeid(CHARTYPE) == typeid(wchar_t))
					len = wcslen((const wchar_t*)szAppend);
				else
					len = strlen((const char*)szAppend);
			}
			putData(szAppend, len);
		}

		void append(const StringBuffer& sbAppend)
		{
			putData(sbAppend.m_pbuf, sbAppend.m_bufpos);
		}

		void append(const CHARTYPE *szAppend)
		{
			size_t len;
			if(typeid(CHARTYPE) == typeid(wchar_t))
				len = wcslen((const wchar_t*)szAppend);
			else
				len = strlen((const char*)szAppend);
			putData(szAppend, len);
		}

		void append(const std::basic_string<CHARTYPE>& strAppend)
		{
			putData(strAppend.c_str(), strAppend.length());
		}

		void append(bool bvalue)
		{
			const CHARTYPE *cszTrue;
			const CHARTYPE *cszFalse;

			if(typeid(CHARTYPE) == typeid(wchar_t))
			{
				cszTrue = (const CHARTYPE*)(L"true");
				cszFalse = (const CHARTYPE*)(L"false");
			}else{
				cszTrue = (const CHARTYPE*)("true");
				cszFalse = (const CHARTYPE*)("false");
			}

			if(bvalue)
			{
				putData(cszTrue, 4);
			}else{
				putData(cszFalse, 5);
			}
		}

		void appendHex(unsigned char x, bool uppercase)
		{
			static const char cszHex_lower[] = "0123456789abcdef";
			static const char cszHex_upper[] = "0123456789ABCDEF";
			const char *cszHex;
			char tmpbuf[2];
			if(uppercase)
				cszHex = cszHex_upper;
			else
				cszHex = cszHex_lower;
			tmpbuf[0] = (CHARTYPE)cszHex[(x >> 4) & 0x0F];
			tmpbuf[1] = (CHARTYPE)cszHex[(x ) & 0x0F];
			putData(tmpbuf, 2);
		}

		void appendHex(unsigned char x)
		{
			appendHex(x, false);
		}

		bool append(uint32_t unvalue, int radix)
		{
			static const char cszHex_lower[] = "0123456789abcdef";

			CHARTYPE tmpbuf[32];
			CHARTYPE *ppos = &tmpbuf[32];
			int n = 0;

			if((radix <= 1) || (radix > 16))
				return false;

			if(unvalue == 0)
			{
				*--ppos = (CHARTYPE)'0';
				n = 1;
			}else{
				while(unvalue != 0)
				{
					int x = unvalue % radix;
					*--ppos = (CHARTYPE)cszHex_lower[x];
					n++;
					unvalue /= radix;
				}
			}

			putData(ppos, n);

			return true;
		}

		bool append(int32_t nvalue, int radix)
		{
			static const char cszHex_lower[] = "0123456789abcdef";

			CHARTYPE tmpbuf[32];
			CHARTYPE *ppos = &tmpbuf[32];
			int n = 0;

			if((radix <= 1) || (radix > 16))
				return false;

			if(nvalue == 0)
			{
				*--ppos = (CHARTYPE)'0';
				n = 1;
			}else{
				bool isNegNum = (nvalue < 0);
				while(nvalue != 0)
				{
					int x = nvalue % radix;
					if(x<0) x *= -1;
					*--ppos = (CHARTYPE)cszHex_lower[x];
					n++;
					nvalue /= radix;
				}
				if(isNegNum)
				{
					*--ppos = (CHARTYPE)'-';
					n++;
				}
			}

			putData(ppos, n);

			return true;
		}

		bool append(uint64_t unvalue, int radix)
		{
			static const char cszHex_lower[] = "0123456789abcdef";

			CHARTYPE tmpbuf[32];
			CHARTYPE *ppos = &tmpbuf[32];
			int n = 0;

			if((radix <= 1) || (radix > 16))
				return false;

			if(unvalue == 0)
			{
				*--ppos = (CHARTYPE)'0';
				n = 1;
			}else{
				while(unvalue != 0)
				{
					int x = unvalue % radix;
					*--ppos = (CHARTYPE)cszHex_lower[x];
					n++;
					unvalue /= radix;
				}
			}

			putData(ppos, n);

			return true;
		}

		bool append(int64_t nvalue, int radix)
		{
			static const char cszHex_lower[] = "0123456789abcdef";

			CHARTYPE tmpbuf[32];
			CHARTYPE *ppos = &tmpbuf[32];
			int n = 0;

			if((radix <= 1) || (radix > 16))
				return false;

			if(nvalue == 0)
			{
				*--ppos = (CHARTYPE)'0';
				n = 1;
			}else{
				bool isNegNum = (nvalue < 0);
				while(nvalue != 0)
				{
					int x = nvalue % radix;
					if(x<0) x *= -1;
					*--ppos = (CHARTYPE)cszHex_lower[x];
					n++;
					nvalue /= radix;
				}
				if(isNegNum)
				{
					*--ppos = (CHARTYPE)'-';
					n++;
				}
			}

			putData(ppos, n);

			return true;
		}

		bool append(uint32_t unvalue)
		{
			return append(unvalue, 10);
		}
		bool append(int32_t nvalue)
		{
			return append(nvalue, 10);
		}
		bool append(uint64_t unvalue)
		{
			return append(unvalue, 10);
		}
		bool append(int64_t nvalue)
		{
			return append(nvalue, 10);
		}

		void append(float fltvalue, int fraclen)
		{
			long part_int = (long)fltvalue;
			float part_flt = fltvalue - part_int;
			float conv_fltpow = ::pow((float)10, (float)fraclen);
			long conv_flt = ((long)(part_flt * conv_fltpow));
			CHARTYPE cDot = (CHARTYPE)'.';
			append(part_int);
			append(&cDot, 1);
			append(conv_flt);
		}

		void appendHexBytes(unsigned char *pdata, int length, bool uppercase)
		{
			static const char cszHex_lower[] = "0123456789abcdef";
			static const char cszHex_upper[] = "0123456789ABCDEF";
			const char *cszHex;
			int i;
			CHARTYPE tmpbuf[2];
			if(uppercase)
				cszHex = cszHex_upper;
			else
				cszHex = cszHex_lower;
			for(i=0; i<length; i++)
			{
				unsigned char x = pdata[i];
				tmpbuf[0] = (CHARTYPE)cszHex[(x >> 4) & 0x0F];
				tmpbuf[1] = (CHARTYPE)cszHex[(x ) & 0x0F];
				putData(tmpbuf, 2);
			}
		}

		void appendHexBytes(unsigned char *pdata, int length)
		{
			appendHexBytes(pdata, length, false);
		}

		void operator=(const StringBuffer<CHARTYPE>& sbAppend)
		{
			clear();
			putData(sbAppend.m_pbuf, sbAppend.m_bufpos);
		}
		void operator=(const std::basic_string<CHARTYPE>& strAppend)
		{
			clear();
			putData(strAppend.c_str(), strAppend.length());
		}
		void operator=(const CHARTYPE *szAppend)
		{
			clear();
			if(typeid(CHARTYPE) == typeid(wchar_t))
			{
				putData(szAppend, wcslen((const wchar_t*)szAppend));
			}else{
				putData(szAppend, strlen((const char*)szAppend));
			}
		}
		void operator=(bool value)
		{
			clear();
			append(value);
		}
		void operator=(uint32_t value)
		{
			clear();
			append(value, 10);
		}
		void operator=(int32_t value)
		{
			clear();
			append(value, 10);
		}
		void operator=(uint64_t value)
		{
			clear();
			append(value, 10);
		}
		void operator=(int64_t value)
		{
			clear();
			append(value, 10);
		}

		void operator+=(const StringBuffer& sbAppend)
		{
			append(sbAppend);
		}
		void operator+=(const std::string& strAppend)
		{
			append(strAppend);
		}
		void operator+=(const CHARTYPE *szAppend)
		{
			append(szAppend);
		}
		void operator+=(bool value)
		{
			append(value);
		}
		void operator+=(uint32_t value)
		{
			append(value, 10);
		}
		void operator+=(int32_t value)
		{
			append(value, 10);
		}
		void operator+=(uint64_t value)
		{
			append(value, 10);
		}
		void operator+=(int64_t value)
		{
			append(value, 10);
		}

		const StringBuffer& operator+(const StringBuffer& sbAppend)
		{
			append(sbAppend);
			return *this;
		}
		const StringBuffer& operator+(const std::string& strAppend)
		{
			append(strAppend);
			return *this;
		}
		const StringBuffer& operator+(const char *szAppend)
		{
			append(szAppend);
			return *this;
		}
		const StringBuffer& operator+(bool value)
		{
			append(value);
			return *this;
		}
		const StringBuffer& operator+(uint32_t value)
		{
			append(value, 10);
			return *this;
		}
		const StringBuffer& operator+(int32_t value)
		{
			append(value, 10);
			return *this;
		}
		const StringBuffer& operator+(uint64_t value)
		{
			append(value, 10);
			return *this;
		}
		const StringBuffer& operator+(int64_t value)
		{
			append(value, 10);
			return *this;
		}

		bool operator==(const StringBuffer &ref) const
		{
			if(m_bufpos != ref.m_bufpos)
				return false;
			return (::memcmp(m_pbuf, ref.m_pbuf, m_bufpos) == 0);
		}

		bool operator==(const CHARTYPE *szText) const
		{
			int i;
			int len;
			if(typeid(CHARTYPE) == typeid(wchar_t))
				len = wcslen((const wchar_t*)szText);
			else
				len = strlen((const char*)szText);
			if(m_bufpos != len)
				return false;
			for(i=0; i<len; i++)
			{
				if(szText[i] != m_pbuf[i])
					return false;
			}
			return true;
		}

		std::basic_string<CHARTYPE> toString() const
		{
			return std::basic_string<CHARTYPE>(m_pbuf, m_bufpos);
		}

		int length() const
		{
			return m_bufpos;
		}

		const CHARTYPE *c_str() const
		{
			return m_pbuf;
		}

		void replaceToUpper()
		{
			size_t i;

			if(m_pbuf == NULL)
				return;

			for(i=0; i<m_bufpos; i++)
			{
				m_pbuf[i] = toupper(m_pbuf[i]);
			}
		}

		void replaceToLower()
		{
			size_t i;

			if(m_pbuf == NULL)
				return;

			for(i=0; i<m_bufpos; i++)
			{
				m_pbuf[i] = tolower(m_pbuf[i]);
			}
		}

		const CHARTYPE *trim2_cstr()
		{
			CHARTYPE *startptr = m_pbuf;
			CHARTYPE *lastptr;

			if(m_pbuf == NULL)
				return NULL;

			if(m_bufpos <= 0)
			{
				m_pbuf[0] = 0;
				return m_pbuf;
			}

			while((*startptr == (CHARTYPE)' ') || (*startptr == (CHARTYPE)'\t') || (*startptr == (CHARTYPE)'\r') || (*startptr == (CHARTYPE)'\n'))
				startptr++;

			lastptr = &startptr[m_bufpos-1];

			while((*lastptr == (CHARTYPE)' ') || (*lastptr == (CHARTYPE)'\t') || (*lastptr == (CHARTYPE)'\r') || (*lastptr == (CHARTYPE)'\n'))
				*lastptr-- = 0;

			return startptr;
		}
		
		bool increaseBuffer(int increaseSize)
		{
			size_t needsize = m_bufpos + increaseSize + 1;
			if(needsize > m_bufsize)
			{
				CHARTYPE *pnewptr;
				size_t newsize = needsize;
				if(newsize % m_blocksize)
					newsize += m_blocksize - (newsize % m_blocksize);
				pnewptr = (CHARTYPE*)realloc(m_pbuf, newsize * sizeof(CHARTYPE));
				if(pnewptr == NULL)
				{
					return false;
				}
				m_bufsize = newsize;
				m_pbuf = pnewptr;
			}

			memset(&m_pbuf[m_bufpos], 0, sizeof(CHARTYPE) * increaseSize);

			return true;
		}

		bool increasePos(int increaseSize)
		{
			size_t needsize = m_bufpos + increaseSize;
			if(needsize > m_bufsize)
				return false;
			m_bufpos = needsize;
			return true;
		}

		CHARTYPE *getBuffer()
		{
			return m_pbuf;
		}

		size_t getBufferSize()
		{
			return m_bufsize;
		}

		CHARTYPE *getCurPosPtr()
		{
			return &m_pbuf[m_bufpos];
		}
	};

}

#define StringBufferA StringBuffer<char>
#define StringBufferW StringBuffer<wchar_t>

#endif /* __JSCPPUTILS_STRINGBUFFER_H__ */
