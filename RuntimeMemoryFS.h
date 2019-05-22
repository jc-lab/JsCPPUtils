/**
 * @file	RuntimeMemoryFS.h
 * @class	RuntimeMemoryFS
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/03/14
 * @brief	RuntimeMemoryFS
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_RUNTIMEMEMORYFS_H__
#define __JSCPPUTILS_RUNTIMEMEMORYFS_H__

#include "SmartPointer.h"

#include <stdlib.h>

#include <string>
#include <list>
#include <map>

namespace JsCPPUtils
{

	class RuntimeMemoryFS
	{
	public:
		struct FileEntry
		{
			void *dataptr;
			long datasize;

			FileEntry() {
				dataptr = 0;
				datasize = 0;
			}
			virtual ~FileEntry() {
				if (dataptr) {
					free(dataptr);
					dataptr = NULL;
				}
				datasize = 0;
			}
		};

		RuntimeMemoryFS();
		~RuntimeMemoryFS();

		int setFile(const char *cwdpath, const char *filename, void *dataptr, long datasize);
		SmartPointer<FileEntry> getFile(const char *cwdpath, const char *filename);

	private:
		std::map<std::basic_string<char>, SmartPointer<FileEntry> > m_filemap;

		void addEntry(const char *cwdpath, const char *filename, const SmartPointer<FileEntry> &spFileEntry);
		SmartPointer<FileEntry> findEntry(const std::basic_string<char> &strrename);

	public:
		static std::basic_string<char> refilename(const char *cwdpath, const char *filename);
		static void parsePath(std::list<std::basic_string<char> > &lstpaths, const char *path);
	};

}

#endif /* __JSCPPUTILS_RUNTIMEMEMORYFS__ */
