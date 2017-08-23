/**
 * @file	Common.h
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/09/27
 * @brief	JsCUtils Common file
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCUTILS_COMMON_H__
#define __JSCUTILS_COMMON_H__

#if defined(__linux__)

#define JSCUTILS_OS_LINUX

#define JSCUTILS_TYPE_FLAG int
#define JSCUTILS_TYPE_DEFCHAR char
#define JSCUTILS_TYPE_ERRNO int

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#include <stdint.h>

#elif defined(_WIN32) || defined(_WIN64)

#define JSCUTILS_OS_WINDOWS

#include <tchar.h>

#define JSCUTILS_TYPE_FLAG DWORD
#define JSCUTILS_TYPE_DEFCHAR TCHAR
#define JSCUTILS_TYPE_ERRNO errno_t

#define likely(x)       (x)
#define unlikely(x)     (x)

#ifdef _MSC_VER

#if _MSC_VER >= 1400
#define _JSCUTILS_MSVC_CRT_SECURE 1
#endif

#if _MSC_VER < 1600 // MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)

#ifndef __JSSTDINT_TYPES__
#define __JSSTDINT_TYPES__
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif /* __JSSTDINT_TYPES__ */

#else
#include <stdint.h>
#endif /* _MSC_VER < 1600 */
#else
#include <stdint.h>
#endif /* _MSC_VER */

#else

#endif

#endif /* __JSCUTILS_COMMON_H__ */

#ifndef __JSCPPUTILS_COMMON_H__
#define __JSCPPUTILS_COMMON_H__



#if defined(JSCUTILS_OS_LINUX)
#define _JSCUTILS_DEFCHARTYPE char
#define _JSCUTILS_T(T) T
#elif defined(JSCUTILS_OS_WINDOWS)
#include <windows.h>
#include <tchar.h>
#define _JSCUTILS_DEFCHARTYPE TCHAR
#define _JSCUTILS_T(T) _T(T)
#endif


#ifdef __cplusplus

namespace JsCPPUtils
{
	class Common
	{
	public:
		static int64_t getTickCount();
	};
}

#endif

#endif /* __JSCPPUTILS_COMMON_H__ */