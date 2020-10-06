/**
 * @file	WinSysFolderPath.h
 * @class	WinSysFolderPath
 * @author	Jichan (development@jc-lab.net)
 * @date	2016/12/05
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#ifndef __JSCPPUTILS_WINSYSFOLDERPATH_H__
#define __JSCPPUTILS_WINSYSFOLDERPATH_H__

#include <windows.h>
#include <Shlwapi.h>
#include <atlstr.h>

#include <Commdlg.h>
#include <Shlobj.h>

#include <string>

namespace JsCPPUtils
{
	class WinSysFolderPath
	{
	private:
		typedef UINT (CALLBACK* LP_SHGetKnownFolderPath) (REFKNOWNFOLDERID /*rfid*/, DWORD /*dwFlags*/, HANDLE /*hToken*/, __out PWSTR * /*ppszPath*/);
		
		typedef HRESULT (CALLBACK* LP_SHGetFolderPathA) (
		  _In_  HWND   hwndOwner,
		  _In_  int    nFolder,
		  _In_  HANDLE hToken,
		  _In_  DWORD  dwFlags,
		  _Out_ LPCSTR pszPath
		);

		typedef HRESULT (CALLBACK* LP_SHGetFolderPathW) (
		  _In_  HWND   hwndOwner,
		  _In_  int    nFolder,
		  _In_  HANDLE hToken,
		  _In_  DWORD  dwFlags,
		  _Out_ LPWSTR pszPath
		);

		static WinSysFolderPath *m_pstaticthis;

		HINSTANCE m_hShell;
		LP_SHGetKnownFolderPath m_pGetKnownFolderPath;
		LP_SHGetFolderPathA m_pGetFolderPathA;
		LP_SHGetFolderPathW m_pGetFolderPathW;

		static void init();

	public:
		WinSysFolderPath();
		~WinSysFolderPath();
		static BOOL GetFolderPathA(int nFolder, REFKNOWNFOLDERID rfid, std::basic_string<char> &retPath);
		static BOOL GetFolderPathW(int nFolder, REFKNOWNFOLDERID rfid, std::basic_string<wchar_t> &retPath);
        static BOOL GetSpecialFolderPath(std::basic_string<wchar_t>& findPath, std::basic_string<wchar_t>& retPath);
#ifdef _UNICODE
#define GetFolderPath GetFolderPathW
#else
#define GetFolderPath GetFolderPathA
#endif
	};

}

#endif /* __JSCPPUTILS_WINSYSFOLDERPATH_H__ */