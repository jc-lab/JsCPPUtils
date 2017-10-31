/**
 * @file	MemoryBuffer.h
 * @class	MemoryBuffer
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/09/29
 * @brief	MemoryBuffer
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <stdlib.h>

namespace JsCPPUtils
{

	class MemoryBuffer {
	protected:
		size_t m_bufsize;
		size_t m_bufpos;
		char *m_pbuf;
		int   m_blocksize;

	public:
		MemoryBuffer();
		MemoryBuffer(int blocksize);
		~MemoryBuffer();
		bool setCurrentPos(size_t currentPos);
		bool increaseBuffer(size_t increaseSize);
		bool increasePos(size_t increaseSize);
		bool putData(const char *data, size_t size);
		char *getBuffer();
		size_t getLength();
		size_t getBufferSize();
		char *getCurPosPtr();
		void clear();
	};

}
