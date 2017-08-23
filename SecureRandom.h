/**
 * @file	RandomWell512.h
 * @class	RandomWell512
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2017/04/04
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include "Common.h"
#include "Random.h"

namespace JsCPPUtils
{

	class SecureRandom : public Random
	{
	private:
		uint32_t m_state_i;
		uint32_t m_state_data[16];
		
		int getSystemRandom(unsigned char *pbuf, int len);
		
	public:
		SecureRandom();
		~SecureRandom();
		
		int init(uint32_t *seeds, int len);
		
		uint32_t _next();

		bool nextBoolean();
		uint8_t nextByte();
		void nextBytes(char *bytes, int len);
		float nextFloat();
		double nextDouble();
		int32_t nextInt();
		int32_t nextInt(int32_t n);
	};

}

