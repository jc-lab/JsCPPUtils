/**
* @file	TextFileReader.h
* @class	TextFileReader
* @author	Jichan (jic5760@naver.com)
* @date	2016/10/02
* @copyright Copyright (C) 2016 jichan.\n
*            This software may be modified and distributed under the terms
*            of the MIT license.  See the LICENSE file for details.
*/

#pragma once

#include <fstream>
#include <string>

#include "Common.h"
#include "StringBuffer.h"
#include "MemoryBuffer.h"

namespace JsCPPUtils
{
	class _TextFileReaderUtils
	{
	private:
		static const unsigned char _JSTFS_UTF_firstByteMark[7];

	public:
		static int _utf32_to_utf8(uint32_t ch, char *utf8dat);
		static int _utf16_to_utf8(uint16_t ch, char *utf8dat);
		static int _u32_to_utf8(uint32_t ch, char *utf8dat);
		static const char *_removeBlanks(const char *cszbuf, int *premainlen);
	};
}

namespace JsCPPUtils
{
	class basic_TextFileReader
	{
	public:
		enum Encoding
		{
			ANSI = 0,
			UTF8 = 1,
			UTF16BE = 2,
			UTF16LE = 3,
			UTF32BE = 4,
			UTF32LE = 5,
		};

	private:
		Encoding m_encoding;
		int m_encoding_charsize;

	protected:
		virtual void onReadLine(char *utf8Text, int utf8Length) {}

	public:
		basic_TextFileReader() :
		  m_encoding(ANSI),
			  m_encoding_charsize(1)
		  {

		  }

		  ~basic_TextFileReader()
		  {

		  }

		  int readFile(const char *szFilePath, Encoding defaultEncoding = ANSI)
		  {
			  int retval = 0;
			  FILE *fp = NULL;
			  errno_t eno;
			  int filelen = -1;
			  char *pbuf = NULL;

			  eno = fopen_s(&fp, szFilePath, "rb");
			  if(eno != 0)
				  return -((int)eno);

			  fseek(fp, 0, FILE_END);
			  filelen = ftell(fp);
			  fseek(fp, 0, FILE_BEGIN);

			  do {
				  pbuf = (char*)malloc(filelen);
				  if(pbuf == NULL)
				  {
					  break;
				  }

				  fread(pbuf, 1, filelen, fp);
				  setData(pbuf, filelen, defaultEncoding);

				  retval = 1;
			  }while(0);
			  if(pbuf != NULL)
			  {
				  free(pbuf);
			  }
			  if(fp != NULL)
			  {
				  fclose(fp);
			  }

			  return retval;
		  }

		  int readFile(const wchar_t *szFilePath, Encoding defaultEncoding = ANSI)
		  {
			  int retval = 0;
			  FILE *fp = NULL;
			  errno_t eno;
			  int filelen = -1;
			  char *pbuf = NULL;

			  eno = _wfopen_s(&fp, szFilePath, _T("rb"));
			  if(eno != 0)
				  return -((int)eno);

			  fseek(fp, 0, FILE_END);
			  filelen = ftell(fp);
			  fseek(fp, 0, FILE_BEGIN);

			  do {
				  pbuf = (char*)malloc(filelen);
				  if(pbuf == NULL)
				  {
					  break;
				  }

				  fread(pbuf, 1, filelen, fp);
				  setData(pbuf, filelen, defaultEncoding);

				  retval = 1;
			  }while(0);
			  if(pbuf != NULL)
			  {
				  free(pbuf);
			  }
			  if(fp != NULL)
			  {
				  fclose(fp);
			  }

			  return retval;
		  }

		  void setData(const char *pdata, int datalen, Encoding defaultEncoding)
		  {
			  int etlen = 0;
			  int remainlen = datalen;

			  volatile char charbuf[4];
			  volatile char utf8buf[4];
			  volatile int  utf8_len;

			  MemoryBuffer memBuf(128);

			  const char *pdataptr = pdata;

			  m_encoding = defaultEncoding;

			  if(datalen >= 2)
			  {
				  etlen = 2;
				  if((pdata[0] == (char)0xEF) && (pdata[1] == (char)0xFF))
				  {
					  m_encoding = UTF16BE;
				  }else if((pdata[0] == (char)0xFF) && (pdata[1] == (char)0xFE))
				  {
					  m_encoding = UTF16LE;
				  }else if(datalen >= 3)
				  {
					  etlen = 3;
					  if((pdata[0] == (char)0xEF) && (pdata[1] == (char)0xBB) && (pdata[2] == (char)0xBF))
					  {
						  m_encoding = UTF8;
					  }else if(datalen >= 4)
					  {
						  etlen = 4;
						  if((pdata[0] == (char)0x00) && (pdata[1] == (char)0x00) && (pdata[2] == (char)0xFE) && (pdata[3] == (char)0xFF))
						  {
							  m_encoding = UTF32BE;
						  }else if((pdata[0] == (char)0xFF) && (pdata[1] == (char)0xFE) && (pdata[2] == (char)0x00) && (pdata[3] == (char)0x00))
						  {
							  m_encoding = UTF32LE;
						  }else{
							  etlen = 0;
						  }
					  }else{
						  etlen = 0;
					  }
				  }else{
					  etlen = 0;
				  }
			  }else{
				  etlen = 0;
			  }

			  pdataptr += etlen;
			  remainlen -= etlen;

			  switch(m_encoding)
			  {
			  case ANSI:
			  case UTF8:
				  m_encoding_charsize = 1;
				  break;
			  case UTF16BE:
			  case UTF16LE:
				  m_encoding_charsize = 2;
				  break;
			  case UTF32BE:
			  case UTF32LE:
				  m_encoding_charsize = 4;
				  break;
			  }

			  while(remainlen > 0)
			  {
				  memBuf.clear();
				  if(m_encoding == UTF8)
				  {
					  while(remainlen > 0)
					  {
						  utf8buf[0] = *pdataptr++; remainlen--;
						  if((utf8buf[0] == '\r') || (utf8buf[0] == '\n'))
						  {
							  pdataptr = _TextFileReaderUtils::_removeBlanks(pdataptr, &remainlen);
							  break;
						  }
						  memBuf.putData((const char*)utf8buf, 1);
					  }
					  onReadLine(memBuf.getBuffer(), memBuf.getLength());
				  }else if(m_encoding == UTF16BE)
				  {
					  while(remainlen > 0)
					  {
						  volatile uint16_t ch;
						  charbuf[0] = *pdataptr++; remainlen--;
						  charbuf[1] = *pdataptr++; remainlen--;
						  ch = ((((uint16_t)charbuf[0])&0x00FF) << 8) | ((((uint16_t)charbuf[1])&0x00FF) << 0);
						  utf8_len = _TextFileReaderUtils::_utf16_to_utf8(ch, (char*)utf8buf);
						  if(utf8_len < 0)
							  continue;
						  if((utf8_len == 1) && ((utf8buf[0] == '\r') || (utf8buf[0] == '\n')))
						  {
							  break;
						  }
						  memBuf.putData((const char*)utf8buf, utf8_len);
					  }
					  onReadLine(memBuf.getBuffer(), memBuf.getLength());
				  }else if(m_encoding == UTF16LE)
				  {
					  while(remainlen > 0)
					  {
						  volatile uint16_t ch;
						  charbuf[0] = *pdataptr++; remainlen--;
						  charbuf[1] = *pdataptr++; remainlen--;
						  ch = ((((uint16_t)charbuf[0])&0x00FF) << 0) | ((((uint16_t)charbuf[1])&0x00FF) << 8);
						  utf8_len = _TextFileReaderUtils::_utf16_to_utf8(ch, (char*)utf8buf);
						  if(utf8_len < 0)
							  continue;
						  if((utf8_len == 1) && ((utf8buf[0] == '\r') || (utf8buf[0] == '\n')))
						  {
							  break;
						  }
						  memBuf.putData((const char*)utf8buf, utf8_len);
					  }
					  onReadLine(memBuf.getBuffer(), memBuf.getLength());
				  }else if(m_encoding == UTF32BE)
				  {
					  while(remainlen > 0)
					  {
						  volatile uint32_t ch;
						  charbuf[0] = *pdataptr++; remainlen--;
						  charbuf[1] = *pdataptr++; remainlen--;
						  charbuf[2] = *pdataptr++; remainlen--;
						  charbuf[3] = *pdataptr++; remainlen--;
						  ch = ((((uint32_t)charbuf[0])&0x00FF) << 24) | ((((uint32_t)charbuf[1])&0x00FF) << 16) | ((((uint32_t)charbuf[2])&0x00FF) << 8) | ((((uint32_t)charbuf[3])&0x00FF) << 0);
						  utf8_len = _TextFileReaderUtils::_utf32_to_utf8(ch, (char*)utf8buf);
						  if(utf8_len < 0)
							  continue;
						  if((utf8_len == 1) && ((utf8buf[0] == '\r') || (utf8buf[0] == '\n')))
						  {
							  break;
						  }
						  memBuf.putData((const char*)utf8buf, utf8_len);
					  }
					  onReadLine(memBuf.getBuffer(), memBuf.getLength());
				  }else if(m_encoding == UTF32LE)
				  {
					  while(remainlen > 0)
					  {
						  volatile uint32_t ch;
						  charbuf[0] = *pdataptr++; remainlen--;
						  charbuf[1] = *pdataptr++; remainlen--;
						  charbuf[2] = *pdataptr++; remainlen--;
						  charbuf[3] = *pdataptr++; remainlen--;
						  ch = ((((uint32_t)charbuf[0])&0x00FF) << 0) | ((((uint32_t)charbuf[1])&0x00FF) << 8) | ((((uint32_t)charbuf[2])&0x00FF) << 16) | ((((uint32_t)charbuf[3])&0x00FF) << 24);
						  utf8_len = _TextFileReaderUtils::_utf32_to_utf8(ch, (char*)utf8buf);
						  if(utf8_len < 0)
							  continue;
						  if((utf8_len == 1) && ((utf8buf[0] == '\r') || (utf8buf[0] == '\n')))
						  {
							  break;
						  }
						  memBuf.putData((const char*)utf8buf, utf8_len);
					  }
					  onReadLine(memBuf.getBuffer(), memBuf.getLength());
				  }else{
					  while(remainlen > 0)
					  {
						  charbuf[0] = *pdataptr++; remainlen--;
						  if((charbuf[0] == '\r') || (charbuf[0] == '\n'))
						  {
							  pdataptr = _TextFileReaderUtils::_removeBlanks(pdataptr, &remainlen);
							  break;
						  }
						  memBuf.putData((const char*)charbuf, 1);
					  }
					  onReadLine(memBuf.getBuffer(), memBuf.getLength());
				  }
			  }
		  }
	};

}