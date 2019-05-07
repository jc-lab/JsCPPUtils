/**
* @file	URL.h
* @class	URL
* @author	Jichan (jic5760@naver.com)
* @date	2016/09/30
* @copyright Copyright (C) 2016 jichan.\n
*            This software may be modified and distributed under the terms
*            of the MIT license.  See the LICENSE file for details.
*/

#pragma once

#include <string>
#include <tchar.h>

namespace JsCPPUtils
{
	template<typename CHARTYPE>
	class URL {
	private:
		std::basic_string<CHARTYPE> m_strFullUrl;
		std::basic_string<CHARTYPE> m_strProtocol;
		std::basic_string<CHARTYPE> m_strUserInfo;
		std::basic_string<CHARTYPE> m_strHost;
		int m_port;
		std::basic_string<CHARTYPE> m_strPath;
		std::basic_string<CHARTYPE> m_strQuery;
		std::basic_string<CHARTYPE> m_strFilename;
		std::basic_string<CHARTYPE> m_strRef;

		bool m_isValid;
		

		bool _strcpy(std::basic_string<CHARTYPE>& pptr, const CHARTYPE *str, int len)
		{
			CHARTYPE *ptr;
			int srclen = ::_tcslen(str);
			int copylen = (len < 0) ? srclen : len;
			if(srclen < copylen)
				copylen = srclen;
			pptr = std::basic_string<CHARTYPE>(str, copylen);
			return true;
		}

		bool _stricmp_bool(const TCHAR *stra, const TCHAR *strb)
		{
			int i;
			int len;
			int a, b;

			if(stra == NULL && strb == NULL)
				return true;

			if(stra == NULL || strb == NULL)
				return false;

			len = _tcslen(stra);

			if(len != _tcslen(strb))
				return false;

			if(len == 0)
				return true;

			i = 0;
			do
			{
				a = tolower(*stra++);
				b = tolower(*strb++);
				i++;
			}while((i < len) && (a==b));

			return (a==b);
		}

		void _init(const TCHAR *szUrl)
		{
			TCHAR *pprev;
			TCHAR *pfind1;
			TCHAR *pfind2;

			m_strFullUrl.clear();
			m_strProtocol.clear();
			m_strUserInfo.clear();
			m_strHost.clear();
			m_port = -1;
			m_strPath.clear();
			m_strQuery.clear();
			m_strFilename.clear();
			m_isValid = false;

			if(!_strcpy(m_strFullUrl, szUrl, -1))
				return ;

			pprev = (TCHAR*)m_strFullUrl.c_str();

			pfind1 = _tcsstr(pprev, _T("://"));
			if(pfind1 == NULL)
				return;

			if(!_strcpy(m_strProtocol, pprev, ((int)((size_t)(pfind1 - pprev)))))
				return ;
			pprev = pfind1+3;
			// 1 = ok

			pfind1 = _tcsstr(pprev, _T("/"));
			pfind2 = _tcsstr(pprev, _T("@"));
			if(pfind2 != NULL)
			{
				if(pfind1 == NULL)
				{
					if(!_strcpy(m_strUserInfo, pprev, ((int)((size_t)(pfind2 - pprev)))))
						return ;
					if(!_parseHost(pfind2 + 1, -1))
						return ;
					m_isValid = true;
					return;
				}else{
					if(!_strcpy(m_strUserInfo, pprev, ((int)((size_t)(pfind2 - pprev)))))
						return ;
					if(!_parseHost(pfind2 + 1, ((int)((size_t)(pfind1 - pfind2))) - 1))
						return ;
					pprev = pfind1;
				}
			}else{
				if(pfind1 == NULL)
				{
					if(!_strcpy(m_strHost, pprev, -1))
						return ;
					if(!_parseHost(pprev, -1))
						return ;
					m_isValid = true;
					return;
				}else{
					if(!_parseHost(pprev, ((int)((size_t)(pfind1 - pprev)))))
						return ;
					pprev = pfind1;
				}
			}

			if(!_strcpy(m_strFilename, pprev, -1))
				return ;

			pfind1 = _tcsstr(pprev, _T("?"));
			//pfind2 = _tcsstr(pprev, _T("#"));
			if(pfind1 == NULL)
			{
				if(!_strcpy(m_strPath, pprev, -1))
					return ;
			}else{
				if(!_strcpy(m_strPath, pprev, ((int)((size_t)(pfind1 - pprev)))))
					return ;
				if(!_strcpy(m_strQuery, pfind1 + 1, -1))
					return ;
			}

			m_isValid = true;
		}

		bool _parseHost(const TCHAR *str, int len)
		{
			const TCHAR *pfind = _tcsstr(str, _T(":"));
			int ifind = (pfind == NULL) ? -1 : ((int)((size_t)(pfind - str)));
			int nlen;
			TCHAR tmpbuf[8] = {0};
			if(len < 0)
				len = _tcslen(str);
			if(ifind >= len || ifind < 0)
			{
				if(!_strcpy(m_strHost, str, len))
					return false;
			}
			else
			{
				_strcpy(m_strHost, str, ifind);
				nlen = len - ifind - 1;
				if(nlen >= 8)
					return false;
				memcpy(tmpbuf, str+ifind+1, nlen * sizeof(TCHAR));
				tmpbuf[nlen] = 0;
				m_port = _ttoi(tmpbuf);
			}
			return true;
		}
	public:
		URL(const char *szUrl)
		{
			_init(szUrl);
		}

		URL(std::basic_string<CHARTYPE> strUrl)	
		{
			_init(strUrl.c_str());
		}

		~URL()
		{

		}

		bool isValid()
		{
			return m_isValid;
		}

		std::basic_string<CHARTYPE> getUrl()
		{
			return std::basic_string<CHARTYPE>(m_strFullUrl);
		}

		std::basic_string<CHARTYPE> getProtocol()
		{
			return std::basic_string<CHARTYPE>(m_strProtocol);
		}

		std::basic_string<CHARTYPE> getHost()
		{
			return std::basic_string<CHARTYPE>(m_strHost);
		}

		int getPort()
		{
			return m_port;
		}

		std::basic_string<CHARTYPE> getUserInfo()
		{
			return std::basic_string<CHARTYPE>(m_strUserInfo);
		}

		std::basic_string<CHARTYPE> getPath()
		{
			return std::basic_string<CHARTYPE>(m_strPath);
		}

		std::basic_string<CHARTYPE> getQuery()
		{
			return std::basic_string<CHARTYPE>(m_strQuery);
		}

		std::basic_string<CHARTYPE> getFilename()
		{
			return std::basic_string<CHARTYPE>(m_strFilename);
		}

		/*
		std::basic_string<CHARTYPE> getRef()
		{
		return std::basic_string<CHARTYPE>(m_strRef);
		}
		*/

		int getDefaultPort()
		{
			const CHARTYPE *sz = m_strProtocol.c_str();
			
			if(typeid(CHARTYPE) == typeid(wchar_t))
			{
				if(sz == NULL)
					return -1;
				if(_stricmp_bool(sz, (const CHARTYPE*)(L"http")))
					return 80;
				else if(_stricmp_bool(sz, (const CHARTYPE*)(L"https")))
					return 443;
				else if(_stricmp_bool(sz, (const CHARTYPE*)(L"ftp")))
					return 21;
				else if(_stricmp_bool(sz, (const CHARTYPE*)(L"sftp")))
					return 22;
				else
					return -1;
			}else{
				if(sz == NULL)
					return -1;
				if(_stricmp_bool(sz, (const CHARTYPE*)("http")))
					return 80;
				else if(_stricmp_bool(sz, (const CHARTYPE*)("https")))
					return 443;
				else if(_stricmp_bool(sz, (const CHARTYPE*)("ftp")))
					return 21;
				else if(_stricmp_bool(sz, (const CHARTYPE*)("sftp")))
					return 22;
				else
					return -1;
			}
		}

		std::basic_string<CHARTYPE> getUserInfo_Username()
		{
			CHARTYPE *sz = (CHARTYPE*)m_strUserInfo.c_str();
			CHARTYPE *pfind;

			if(sz == NULL)
				return std::basic_string<CHARTYPE>();

			if(typeid(CHARTYPE) == typeid(wchar_t))
				pfind = (CHARTYPE*)wcsstr((wchar_t*)sz, L":");
			else
				pfind = (CHARTYPE*)strstr((char*)sz, ":");
			if(pfind == NULL)
				return std::basic_string<CHARTYPE>(sz);

			return std::basic_string<CHARTYPE>(sz, pfind - sz);
		}

		std::basic_string<CHARTYPE> getUserInfo_Password()
		{
			CHARTYPE *sz = (CHARTYPE*)m_strUserInfo.c_str();
			CHARTYPE *pfind;

			if(sz == NULL)
				return std::basic_string<CHARTYPE>();

			if(typeid(CHARTYPE) == typeid(wchar_t))
				pfind = (CHARTYPE*)wcsstr((wchar_t*)sz, L":");
			else
				pfind = (CHARTYPE*)strstr((char*)sz, ":");
			if(pfind == NULL)
				return std::basic_string<CHARTYPE>();

			return std::basic_string<CHARTYPE>(pfind + 1);
		}

		bool isSecure()
		{
			const TCHAR *sz = m_strProtocol.c_str();
			if(typeid(CHARTYPE) == typeid(wchar_t))
			{
				if(_stricmp_bool(sz, (const CHARTYPE *)(L"https")))
					return true;
			}else{
				if(_stricmp_bool(sz, (const CHARTYPE *)("https")))
					return true;
			}
			return false;
		}
	};
	
}

