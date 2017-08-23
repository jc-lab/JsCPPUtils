/**
 * @file	Random.h
 * @class	Random
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2016/09/30
 * @brief	Random
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_RANDOM_H__
#define __JSCPPUTILS_RANDOM_H__

#include "Common.h"

namespace JsCPPUtils
{

	class Random
	{
	public:
		virtual bool nextBoolean() = 0;
		virtual uint8_t nextByte() = 0;
		virtual void nextBytes(char *bytes, int len) = 0;
		virtual float nextFloat() = 0;
		virtual double nextDouble() = 0;
		virtual int32_t nextInt() = 0;
		virtual int32_t nextInt(int32_t n) = 0;
	};

}

#endif /* __JSCPPUTILS_RANDOM_H__ */
