/**
 * @file	Base64.h
 * @class	Base64
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/02
 * @brief	Base64
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#pragma once

#include <string>
#include <vector>
#include <stdlib.h>

#include "SmartPointer.h"

namespace JsCPPUtils
{

	class Base64
	{
	private:
		typedef union{
			struct{
				unsigned char c1,c2,c3;
			};
			struct{
				unsigned int e1:6,e2:6,e3:6,e4:6;
			};
		} BF;

		static const char c_MimeBase64[64];
		static const int c_DecodeMimeBase64[256];

	public:
		static SmartPointer< std::vector<char> > encode(const char *src, int srclen);
		static SmartPointer< std::vector<unsigned char> > decode(const char *src, int srclen);
		static std::string encodeToText(const char *src, int srclen);
		static bool decode(std::vector<unsigned char> &retvector, const char *src, int srclen);

	};

}