/**
 * @file	RandomWell512.h
 * @class	RandomWell512
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2017/04/04
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "SecureRandom.h"

#if defined(JSCUTILS_HAS_TIME_H)
#include <time.h>
#endif

#if defined(JSCUTILS_OS_WINDOWS)
#include <Windows.h>
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")
#elif defined(JSCUTILS_OS_LINUX)
#include <sys/types.h> /* for open(2) */
#include <sys/stat.h> /* for open(2) */
#include <fcntl.h> /* for open(2) */
#include <unistd.h> /* for read(2), close(2) */
#endif

namespace JsCPPUtils
{

#define W 32
#define P 0
#define M1 13
#define M2 9
#define M3 5

#define MAT0POS(t,v) (v^(v>>t))
#define MAT0NEG(t,v) (v^(v<<(-(t))))
#define MAT3NEG(t,v) (v<<(-(t)))
#define MAT4NEG(t,b,v) (v ^ ((v<<(-(t))) & b))

#define V0            m_state_data[m_state_i                   ]
#define VM1           m_state_data[(m_state_i+M1) & 0x0000000fU]
#define VM2           m_state_data[(m_state_i+M2) & 0x0000000fU]
#define VM3           m_state_data[(m_state_i+M3) & 0x0000000fU]
#define VRm1          m_state_data[(m_state_i+15) & 0x0000000fU]
#define VRm2          m_state_data[(m_state_i+14) & 0x0000000fU]
#define newV0         m_state_data[(m_state_i+15) & 0x0000000fU]
#define newV1         m_state_data[m_state_i                   ]
#define newVRm1       m_state_data[(m_state_i+14) & 0x0000000fU]
	
	SecureRandom::SecureRandom()
	{
		uint32_t tseed[1] = {0};
#if defined(JSCUTILS_HAS_TIME_H)
		tseed[0] = (uint32_t)time(NULL);
#endif
		init(tseed, 1);
	}


	SecureRandom::~SecureRandom()
	{

	}
	
	
	int SecureRandom::getSystemRandom(unsigned char *pbuf, int len)
	{
		int retval = 0;
		
#if defined(JSCUTILS_OS_WINDOWS)
		HCRYPTPROV   hCryptProv;
		if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) 
		{    
			if (CryptGenRandom(hCryptProv, len, pbuf)) 
			{
				retval = 1;
			}
			CryptReleaseContext(hCryptProv, 0);
		}
#elif defined(JSCUTILS_OS_LINUX)
		int fd;
		fd = open("/dev/urandom", O_RDONLY);
		if (fd != -1)
		{
			read(fd, pbuf, len);
			close(fd);
			retval = 1;
		}
#endif
		return retval;
	}

	int SecureRandom::init(uint32_t *seeds, int len)
	{
		int retval = 0;
		int i;
		uint32_t sysrndbuf[16] = {0};
		
		m_state_i = 0;
		
		for (i = 0; i < len; i++)
		{
			m_state_data[i % 16] = seeds[i % 16];
		}
		for (i = len; i < 16; i++)
		{
			uint32_t x = (m_state_data[i - 1] + i*i);
			m_state_data[i] = x * x;
		}
		
		getSystemRandom((unsigned char*)sysrndbuf, sizeof(sysrndbuf));
		for (i = 0; i < 16; i++)
		{
			m_state_data[i] ^= sysrndbuf[i];
		}
		
		return retval;
	}
	
	uint32_t SecureRandom::_next()
	{
		uint32_t z0, z1, z2;
		z0    = VRm1;
		z1    = MAT0NEG(-16, V0)    ^ MAT0NEG(-15, VM1);
		z2    = MAT0POS(11, VM2);
		newV1 = z1                  ^ z2; 
		newV0 = MAT0NEG(-2, z0)     ^ MAT0NEG(-18, z1)    ^ MAT3NEG(-28, z2) ^ MAT4NEG(-5, 0xda442d24U, newV1);
		m_state_i = (m_state_i + 15) & 0x0000000fU;
		return m_state_data[m_state_i];
	}

	bool SecureRandom::nextBoolean()
	{
		if (_next() % 2)
			return true;
		else
			return false;
	}

	uint8_t SecureRandom::nextByte()
	{
		return ((uint8_t)_next());
	}

	void SecureRandom::nextBytes(char *bytes, int len)
	{
		int i;
		for (i = 0; i < len; i++)
		{
			bytes[i] = ((char)(_next() & 0xFF));
		}
	}
	float SecureRandom::nextFloat()
	{
		return (((float)_next()) / 4294967295.0f);
	}
	double SecureRandom::nextDouble()
	{
		return (((double)_next()) / 4294967295.0f);
	}
	int32_t SecureRandom::nextInt()
	{
		return (int32_t)_next();
	}
	int32_t SecureRandom::nextInt(int32_t n)
	{
		return (_next() % n);
	}

}