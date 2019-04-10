/**
 * @file	Base64.cpp
 * @class	Base64
 * @author	Jichan (jic5760@naver.com)
 * @date	2016/10/02
 * @brief	Base64
 *			원본 출처 : https://kldp.org/node/109436
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "Endian.h"

#include "Base64.h"

namespace JsCPPUtils
{

	const char Base64::c_MimeBase64[64] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};

	const int Base64::c_DecodeMimeBase64[256] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
		52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
		41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
	};

	SmartPointer< std::vector<char> > Base64::encode(const char *src, int srclen)
	{
		SmartPointer< std::vector<char> > result = new std::vector<char>();
		int endian = Endian::getEndian();

		int i, j;
		BF temp;

		int needsize = (4 * (srclen / 3)) + ((srclen % 3) ? 4 : 0) + 1;
		result->resize(needsize);

		if(endian == 0){ // little endian(intel)
			for(i = 0, j = 0; i < srclen ; i = i+3){
				temp.c3 = src[i];
				if((i+1) >= srclen) temp.c2 = 0x00;
				else temp.c2 = src[i+1];
				if((i+2) >= srclen) temp.c1 = 0x00;
				else temp.c1 = src[i+2];

				(*result)[j++] = c_MimeBase64[temp.e4];
				(*result)[j++] = c_MimeBase64[temp.e3];
				(*result)[j++] = c_MimeBase64[temp.e2];
				(*result)[j++] = c_MimeBase64[temp.e1];

				if((i+1) >= srclen) (*result)[j-2] = '=';
				if((i+2) >= srclen) (*result)[j-1] = '=';
			}
		} else { // big endian(sun)
			for(i = 0, j = 0; i < srclen ; i = i+3){
				temp.c1 = src[i];
				if((i+1) >= srclen) temp.c2 = 0x00;
				else temp.c2 = src[i+1];
				if((i+2) >= srclen) temp.c3 = 0x00;
				else temp.c3 = src[i+2];

				(*result)[j++] = c_MimeBase64[temp.e4];
				(*result)[j++] = c_MimeBase64[temp.e3];
				(*result)[j++] = c_MimeBase64[temp.e2];
				(*result)[j++] = c_MimeBase64[temp.e1];

				if((i+1) >= srclen) (*result)[j-2] = '=';
				if((i+2) >= srclen) (*result)[j-1] = '=';
			}
		}

		return result;
	}

	SmartPointer< std::vector<unsigned char> > Base64::decode(const char *src, int srclen)
	{
		SmartPointer<std::vector<unsigned char> > result = new std::vector<unsigned char>();
		if (decode(*result, src, srclen))
			return result;
	}

	std::string Base64::encodeToText(const char *src, int srclen)
	{
		int endian = Endian::getEndian();

		int i, j;
		BF temp;

		int needsize = (4 * (srclen / 3)) + ((srclen % 3) ? 4 : 0) + 1;

		std::vector<char> resultArr((size_t)needsize, (char)0);
		char *result = &resultArr[0];

		if (endian == 0) { // little endian(intel)
			for (i = 0, j = 0; i < srclen; i = i + 3) {
				temp.c3 = src[i];
				if ((i + 1) >= srclen) temp.c2 = 0x00;
				else temp.c2 = src[i + 1];
				if ((i + 2) >= srclen) temp.c1 = 0x00;
				else temp.c1 = src[i + 2];

				result[j++] = c_MimeBase64[temp.e4];
				result[j++] = c_MimeBase64[temp.e3];
				result[j++] = c_MimeBase64[temp.e2];
				result[j++] = c_MimeBase64[temp.e1];

				if ((i + 1) >= srclen) result[j - 2] = '=';
				if ((i + 2) >= srclen) result[j - 1] = '=';
			}
		}
		else { // big endian(sun)
			for (i = 0, j = 0; i < srclen; i = i + 3) {
				temp.c1 = src[i];
				if ((i + 1) >= srclen) temp.c2 = 0x00;
				else temp.c2 = src[i + 1];
				if ((i + 2) >= srclen) temp.c3 = 0x00;
				else temp.c3 = src[i + 2];

				result[j++] = c_MimeBase64[temp.e4];
				result[j++] = c_MimeBase64[temp.e3];
				result[j++] = c_MimeBase64[temp.e2];
				result[j++] = c_MimeBase64[temp.e1];

				if ((i + 1) >= srclen) result[j - 2] = '=';
				if ((i + 2) >= srclen) result[j - 1] = '=';
			}
		}

		for (i = 0, j = resultArr.size() - j; i < j; i++)
			resultArr.pop_back();

		return std::string(resultArr.begin(), resultArr.end());
	}

	bool Base64::decode(std::vector<unsigned char> &retvector, const char *src, int srclen)
	{
		int endian = Endian::getEndian();

		int i, j, blank = 0;
		BF temp;

		unsigned char *result;
		int needsize = (srclen / 4) * 3;

		if (srclen % 4)
		{
			return false;
		}

		retvector.clear();
		retvector.assign((size_t)needsize, (unsigned char)0);
		result = &retvector[0];

		if (endian == 0) { // little endian(intel)
			for (i = 0, j = 0; i < srclen; i = i + 4, j = j + 3) {
				temp.e4 = c_DecodeMimeBase64[src[i]];
				temp.e3 = c_DecodeMimeBase64[src[i + 1]];
				temp.e2 = 0;
				temp.e1 = 0x00;
				if (src[i + 2] == '=') {
					blank++;
				}
				else temp.e2 = c_DecodeMimeBase64[src[i + 2]];
				if (src[i + 3] == '=') {
					blank++;
				}
				else temp.e1 = c_DecodeMimeBase64[src[i + 3]];

				if (temp.e1 < 0 || temp.e2 < 0 || temp.e3 < 0 || temp.e4 < 0)
					return false;

				result[j] = temp.c3;
				result[j + 1] = temp.c2;
				result[j + 2] = temp.c1;
			}
		}
		else { // big endian(sun)
			for (i = 0, j = 0; i < srclen; i = i + 4, j = j + 3) {
				temp.e4 = c_DecodeMimeBase64[src[i]];
				temp.e3 = c_DecodeMimeBase64[src[i + 1]];
				temp.e2 = 0x00;
				temp.e1 = 0x00;
				if (src[i + 2] == '=') {
					blank++;
				}
				else temp.e2 = c_DecodeMimeBase64[src[i + 2]];
				if (src[i + 3] == '=') {
					blank++;
				}
				else temp.e1 = c_DecodeMimeBase64[src[i + 3]];

				if (temp.e1 < 0 || temp.e2 < 0 || temp.e3 < 0 || temp.e4 < 0)
					return false;

				result[j] = temp.c1;
				result[j + 1] = temp.c2;
				result[j + 2] = temp.c3;
			}
		}

		for (i = 0; i < blank; i++)
			retvector.pop_back();

		return true;
	}

}
