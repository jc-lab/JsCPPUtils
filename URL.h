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
			if(len >= 0)
				pptr = std::basic_string<CHARTYPE>(str, len);
			else
				pptr = std::basic_string<CHARTYPE>(str);
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

		void _init(const CHARTYPE *szUrl)
		{
			std::string ncstr_1("://");
			std::basic_string<CHARTYPE> cstr_1(ncstr_1.begin(), ncstr_1.end());

			std::string::size_type nfind1;
			std::string::size_type nfind2;
			std::string::size_type nprev;

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

			nfind1 = m_strFullUrl.find(cstr_1);
			if (nfind1 == std::string::npos)
				return;

			if(!_strcpy(m_strProtocol, m_strFullUrl.c_str(), nfind1))
				return ;

			nprev = nfind1 + 3;

			nfind1 = m_strFullUrl.find((CHARTYPE)'/', nprev);
			nfind2 = m_strFullUrl.find((CHARTYPE)'@', nprev);
			if(nfind2 != std::string::npos)
			{
				if(nfind1 == std::string::npos)
				{
					if(!_strcpy(m_strUserInfo, m_strFullUrl.c_str() + nprev, nfind1 - nprev))
						return ;
					if(!_parseHost(m_strFullUrl.c_str() + nfind2 + 1, -1))
						return ;
					m_isValid = true;
					return;
				}else{
					if(!_strcpy(m_strUserInfo, m_strFullUrl.c_str() + nprev, nfind2 - nprev))
						return ;
					if(!_parseHost(m_strFullUrl.c_str() + nfind2 + 1, nfind1 - nfind2 - 1))
						return ;
					nprev = nfind1;
				}
			}else{
				if(nfind1 == std::string::npos)
				{
					if(!_strcpy(m_strHost, m_strFullUrl.c_str() + nprev, -1))
						return ;
					if(!_parseHost(m_strFullUrl.c_str() + nprev, -1))
						return ;
					m_isValid = true;
					return;
				}else{
					if(!_parseHost(m_strFullUrl.c_str() + nprev, nfind1 - nprev))
						return ;
					nprev = nfind1;
				}
			}

			if(!_strcpy(m_strFilename, m_strFullUrl.c_str() + nprev, -1))
				return ;

			nfind1 = m_strFullUrl.find((CHARTYPE)'?', nprev);
			if(nfind1 == std::string::npos)
			{
				if(!_strcpy(m_strPath, m_strFullUrl.c_str() + nprev, -1))
					return ;
			}else{
				if(!_strcpy(m_strPath, m_strFullUrl.c_str() + nprev, nfind1 - nprev))
					return ;
				if(!_strcpy(m_strQuery, m_strFullUrl.c_str() + nfind1 + 1, -1))
					return ;
			}

			m_isValid = true;
		}

		bool _parseHost(const CHARTYPE *str, int len)
		{
			std::basic_string<CHARTYPE> buffer;
			if (len < 0)
				buffer = str;
			else
				buffer.append(str, len);
			std::string::size_type nfind = buffer.find((CHARTYPE)':');
			int nlen;
			char tmpbuf[8] = {0};
			if(nfind == std::string::npos)
			{
				m_strHost = buffer;
			}
			else
			{
				int i;
				_strcpy(m_strHost, str, nfind);
				nlen = len - nfind - 1;
				if(nlen >= 8)
					return false;
				for (i = 0; i < nlen; i++)
				{
					tmpbuf[i] = *(str + nfind + 1 + i);
				}
				tmpbuf[nlen] = 0;
				m_port = atoi(tmpbuf);
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

