/**
 * @file	CompositeKey.h
 * @class	CompositeKey
 * @date	2017/08/25
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @copyright Copyright (C) 2017 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_COMPOSITEKEY_H__
#define __JSCPPUTILS_COMPOSITEKEY_H__

#include "Common.h"

#include <string.h>

namespace JsCPPUtils
{
	template<typename TKEY1, typename TKEY2>
		class CompositeKeyDual
		{
		public:
			char ckeydata[sizeof(TKEY1) + sizeof(TKEY2)];
			
			CompositeKeyDual()
			{
				memset(ckeydata, 0, sizeof(ckeydata));
			}
			
			CompositeKeyDual(TKEY1 key1, TKEY2 key2)
			{
				memcpy(&ckeydata[0], &key1, sizeof(TKEY1));
				memcpy(&ckeydata[sizeof(TKEY1)], &key2, sizeof(TKEY2));
			}
			
			CompositeKeyDual(const CompositeKeyDual<TKEY1, TKEY2> &_ref)
			{
				memcpy(ckeydata, _ref.ckeydata, sizeof(ckeydata));
			}
			
			inline bool operator<(const CompositeKeyDual<TKEY1, TKEY2>& rhs) const 
			{
				int _compare = memcmp(ckeydata, rhs.ckeydata, sizeof(ckeydata));
				if (_compare < 0)
					return true;
				else
					return false;
			}
			inline bool operator>(const CompositeKeyDual<TKEY1, TKEY2>& rhs) const
			{
				int _compare = memcmp(ckeydata, rhs.ckeydata, sizeof(ckeydata));
				if (_compare > 0)
					return true;
				else
					return false;
			}
			inline bool operator<=(const CompositeKeyDual<TKEY1, TKEY2>& rhs) const 
			{
				int _compare = memcmp(ckeydata, rhs.ckeydata, sizeof(ckeydata));
				if (_compare > 0)
					return false;
				else
					return true;
			}
			inline bool operator>=(const CompositeKeyDual<TKEY1, TKEY2>& rhs) const
			{
				int _compare = memcmp(ckeydata, rhs.ckeydata, sizeof(ckeydata));
				if (_compare < 0)
					return false;
				else
					return true;
			}
		};
}

#endif /* __JSCPPUTILS_COMPOSITEKEY_H__ */
