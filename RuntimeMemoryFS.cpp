/**
 * @file	RuntimeMemoryFS.cpp
 * @class	RuntimeMemoryFS
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2018/03/14
 * @copyright Copyright (C) 2018 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "RuntimeMemoryFS.h"

namespace JsCPPUtils
{

	RuntimeMemoryFS::RuntimeMemoryFS()
	{
	}

	RuntimeMemoryFS::~RuntimeMemoryFS()
	{
	}

	int RuntimeMemoryFS::setFile(const char *cwdpath, const char *filename, void *dataptr, long datasize)
	{
		std::basic_string<char> strrefilepath = refilename(cwdpath, filename);
		SmartPointer<RuntimeMemoryFS::FileEntry> spFileEntry = findEntry(strrefilepath);
		if (spFileEntry == NULL)
		{
			spFileEntry = new FileEntry();
		}
		if (spFileEntry->dataptr)
		{
			free(spFileEntry->dataptr);
			spFileEntry->dataptr = NULL;
		}
		spFileEntry->dataptr = malloc(datasize);
		memcpy(spFileEntry->dataptr, dataptr, datasize);
		spFileEntry->datasize = datasize;

		m_filemap[strrefilepath] = spFileEntry;
		return 1;
	}

	SmartPointer<RuntimeMemoryFS::FileEntry> RuntimeMemoryFS::getFile(const char *cwdpath, const char *filename)
	{
		std::basic_string<char> strReFilename = refilename(cwdpath, filename);
		return findEntry(strReFilename);
	}

	SmartPointer<RuntimeMemoryFS::FileEntry> RuntimeMemoryFS::findEntry(const std::basic_string<char> &strrename)
	{
		std::map<std::basic_string<char>, SmartPointer<FileEntry> >::iterator iterEntry;

		iterEntry = m_filemap.find(strrename);
		if (iterEntry == m_filemap.end())
		{
			return NULL;
		}

		return iterEntry->second;
	}

	void RuntimeMemoryFS::addEntry(const char *cwdpath, const char *filename, const SmartPointer<FileEntry> &spFileEntry)
	{
		std::basic_string<char> strReFilename = refilename(cwdpath, filename);
		m_filemap[strReFilename] = spFileEntry;
	}

	void RuntimeMemoryFS::parsePath(std::list<std::basic_string<char> > &lstpaths, const char *path)
	{
		int templen = 0;
		char tempbuf[64];

		while (*path)
		{
			int j;
			int remunits = 1;
			char nxt = *(path++);
			if (nxt & 0x80) {
				unsigned char msk = 0xe0;
				tempbuf[templen++] = nxt;
				for (remunits = 1; (nxt & msk) != (msk << 1); ++remunits)
				{
					msk = (msk >> 1) | 0x80;
					tempbuf[templen++] = *(path++);
				}
			}
			else {
				if ((nxt == '/') || (nxt == '\\'))
				{
					if (templen > 0)
					{
						lstpaths.push_back(std::basic_string<char>(tempbuf, templen));
						templen = 0;
					}
				}
				else {
					tempbuf[templen++] = nxt;
				}
			}
		}

		if (templen > 0)
		{
			tempbuf[templen] = 0;
			if (strcmp(tempbuf, "..") == 0)
			{
				lstpaths.pop_back();
			}
			else {
				lstpaths.push_back(std::basic_string<char>(tempbuf, templen));
			}
			templen = 0;
		}
	}

	std::basic_string<char> RuntimeMemoryFS::refilename(const char *cwdpath, const char *filename)
	{
		std::list<std::basic_string<char> > lstpaths;
		std::basic_string<char> strReFilename;

		int templen = 0;
		char tempbuf[MAX_PATH];

		if ((*filename != '/') && (*filename != '\\') && cwdpath)
		{
			parsePath(lstpaths, cwdpath);
		}

		while (*filename)
		{
			int j;
			int remunits = 1;
			char nxt = *(filename++);
			if (nxt & 0x80) {
				unsigned char msk = 0xe0;
				tempbuf[templen++] = nxt;
				for (remunits = 1; (nxt & msk) != (msk << 1); ++remunits)
				{
					msk = (msk >> 1) | 0x80;
					tempbuf[templen++] = *(filename++);
				}
			}
			else {
				if ((nxt == '/') || (nxt == '\\'))
				{
					if (templen > 0)
					{
						tempbuf[templen] = 0;
						if (strcmp(tempbuf, "..") == 0)
						{
							lstpaths.pop_back();
						}
						else {
							lstpaths.push_back(std::basic_string<char>(tempbuf, templen));
						}
						templen = 0;
					}
				}
				else {
					tempbuf[templen++] = nxt;
				}
			}
		}

		if (templen > 0)
		{
			tempbuf[templen] = 0;
			if (strcmp(tempbuf, "..") == 0)
			{
				lstpaths.pop_back();
			}
			else {
				lstpaths.push_back(std::basic_string<char>(tempbuf, templen));
			}
			templen = 0;
		}

		for (std::list< std::basic_string<char> >::iterator iter = lstpaths.begin(); iter != lstpaths.end(); iter++)
		{
			if (strReFilename.size())
			{
				strReFilename.append("/");
			}
			strReFilename.append(*iter);
		}

		return strReFilename;
	}

}
