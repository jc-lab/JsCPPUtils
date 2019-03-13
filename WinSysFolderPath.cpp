/**
 * @file	WinSysFolderPath.h
 * @class	WinSysFolderPath
 * @author	Jichan (development@jc-lab.net)
 * @date	2016/12/05
 * @brief	WinSysFolderPath
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "WinSysFolderPath.h"

#pragma comment(lib, "Shlwapi.lib")

namespace JsCPPUtils
{

	WinSysFolderPath *WinSysFolderPath::m_pstaticthis = NULL;

	WinSysFolderPath::WinSysFolderPath()
	{
		m_hShell = LoadLibraryA("shell32.dll");

		m_pGetKnownFolderPath = (LP_SHGetKnownFolderPath)GetProcAddress(m_hShell, "SHGetKnownFolderPath");
		m_pGetFolderPathA = (LP_SHGetFolderPathA)GetProcAddress(m_hShell, "SHGetFolderPathA");
		m_pGetFolderPathW = (LP_SHGetFolderPathW)GetProcAddress(m_hShell, "SHGetFolderPathW");
	}

	WinSysFolderPath::~WinSysFolderPath()
	{
		if(m_hShell != NULL)
		{
			FreeLibrary(m_hShell);
			m_hShell = NULL;
		}
	}

	void WinSysFolderPath::init()
	{
		if(m_pstaticthis == NULL)
		{
			m_pstaticthis = new WinSysFolderPath();
		}
	}

	BOOL WinSysFolderPath::GetFolderPathA(int nFolder, REFKNOWNFOLDERID rfid, std::basic_string<char> &retPath)
	{
		BOOL bRst = FALSE;
		PWSTR pszPath;
		CHAR cpath[MAX_PATH+1];
	
		retPath.clear();

		init();

		do{
			if (m_pstaticthis->m_pGetKnownFolderPath)
			{
				if (SUCCEEDED(m_pstaticthis->m_pGetKnownFolderPath(rfid, 0, NULL, &pszPath)))
				{
					retPath = CW2A(pszPath);
					CoTaskMemFree(pszPath);
					bRst = TRUE;
					break;
				}
			}
			if(m_pstaticthis->m_pGetFolderPathA)
			{
				if(m_pstaticthis->m_pGetFolderPathA(NULL, nFolder, NULL, 0, cpath) == S_OK)
				{
					retPath = cpath;
					bRst = TRUE;
					break;
				}
			}
		}while(0);

		return bRst;
	}

	BOOL WinSysFolderPath::GetFolderPathW(int nFolder, REFKNOWNFOLDERID rfid, std::basic_string<wchar_t> &retPath)
	{
		BOOL bRst = FALSE;
		PWSTR pszPath;
		WCHAR cpath[MAX_PATH+1];
	
		retPath.clear();

		init();

		do{
			if (m_pstaticthis->m_pGetKnownFolderPath)
			{
				if (SUCCEEDED(m_pstaticthis->m_pGetKnownFolderPath(rfid, 0, NULL, &pszPath)))
				{
					retPath = pszPath;
					CoTaskMemFree(pszPath);
					bRst = TRUE;
				}
				break;
			}
			if(m_pstaticthis->m_pGetFolderPathW)
			{
				if(m_pstaticthis->m_pGetFolderPathW(NULL, nFolder, NULL, 0, cpath) == S_OK)
				{
					retPath = cpath;
					bRst = TRUE;
				}
				break;
			}
		}while(0);

		return bRst;
	}

}
