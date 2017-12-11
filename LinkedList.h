/**
 * @file	LinkedList.h
 * @class	LinkedList
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2017/11/18
 * @copyright Copyright (C) 2017 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_LINKEDLIST_H__
#define __JSCPPUTILS_LINKEDLIST_H__

#include <new>
#include <exception>

#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "Lockable.h"

namespace JsCPPUtils
{
#define JsCPPUtils_LinkedList_FREECACHESIZE 64

	/**
	* TVALUE					Value의 type
	* _initial_numofblocks		초기 블록 수 (데이터 수)
	* _incblocksize				지정한 블록 수 보다 데이터가 많아질 때 증가시길 블록 수
	*/
	template<typename TVALUE>
	class basic_LinkedListNTS
	{
	private:
		template<typename T, bool bIsClass = is_class<T>::value>
		class WrappedClass
		{
		public:
			void callconstructor(T *ptr)
			{
			}
		};

		template<typename T>
		class WrappedClass<T, true>
		{
		public:
			void callconstructor(T *ptr)
			{
				T *p = new(ptr) T();
			}
		};

		typedef int32_t blockindex_t;
		struct _tag_block;
		typedef struct _tag_block
		{
			int used;
			blockindex_t prev;
			blockindex_t next;
			TVALUE value;
		} block_t;

#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
		JsCUtils_fnMalloc_t m_custom_malloc;
		JsCUtils_fnRealloc_t m_custom_realloc;
		JsCUtils_fnFree_t m_custom_free;
#endif

		int m_conf_incblocksize; ///< 지정한 블록 수 보다 데이터가 많아질 때 증가시길 블록 수

		blockindex_t m_blocksize;
		blockindex_t m_blockcount;

		block_t *m_blocks;
		blockindex_t m_first;
		blockindex_t m_last;

		blockindex_t m_freecache[JsCPPUtils_LinkedList_FREECACHESIZE];
		blockindex_t m_lastfoundfreeblockidx;

		bool m_freed;

		/*
		private:
		// Copy assignment operator.
		basic_HashMap& operator=(const basic_HashMap<TKEY, TVALUE, _numofbuckets, _numofinitialblocks, _incblocksize>& other)
		{
		}

		// Copy constructor.
		basic_HashMap(const basic_HashMap<TKEY, TVALUE, _numofbuckets, _numofinitialblocks, _incblocksize>& _ref)
		{
		}
		*/
	public:
		class Iterator
		{
		private:
			friend class basic_LinkedListNTS<TVALUE>;

			basic_LinkedListNTS<TVALUE> *m_pmap;
			blockindex_t m_curidx;
			blockindex_t m_nextidx;
			blockindex_t m_previdx;

		public:
			Iterator()
			{
				m_pmap = NULL;
				m_curidx = 0;
				m_previdx = 0;
				m_nextidx = 0;
			}

			Iterator(const Iterator& _ref)
			{
				m_pmap = _ref.m_pmap;
				m_curidx = _ref.m_curidx;
				m_previdx = _ref.m_previdx;
				m_nextidx = _ref.m_nextidx;
			}

			bool hasPrev()
			{
				return (m_previdx != 0);
			}

			bool hasNext()
			{
				return (m_nextidx != 0);
			}

			TVALUE &prev()
			{
				block_t *pblock = &m_pmap->m_blocks[m_previdx - 1];
				m_curidx = m_previdx;
				m_nextidx = pblock->next;
				m_previdx = pblock->prev;
				return pblock->value;
			}

			TVALUE &next()
			{
				block_t *pblock = &m_pmap->m_blocks[m_nextidx - 1];
				m_curidx = m_nextidx;
				m_nextidx = pblock->next;
				m_previdx = pblock->prev;
				return pblock->value;
			}

			TVALUE *getValuePtr()
			{
				block_t *pblock = &m_pmap->m_blocks[m_curidx - 1];
				return &pblock->value;
			}

			void erase()
			{
				int i;

				block_t *pcurblock = &m_pmap->m_blocks[m_curidx - 1];
				block_t *pprevblock = NULL;
				block_t *pnextblock = NULL;
				
				if (pcurblock->next > 0)
				{
					pnextblock = &m_pmap->m_blocks[pcurblock->next - 1];
					pnextblock->prev = pcurblock->prev;
				}
				if (pcurblock->prev > 0)
				{
					pprevblock = &m_pmap->m_blocks[pcurblock->prev - 1];
					pprevblock->next = pcurblock->next;
				}
				if(m_curidx == m_pmap->m_first)
					m_pmap->m_first = pcurblock->next;
				if(m_curidx == m_pmap->m_last)
					m_pmap->m_last = pcurblock->prev;

				if (is_class<TVALUE>::value)
					pcurblock->value.~TVALUE();

				pcurblock->used = 0;
				pcurblock->next = 0;
				pcurblock->prev = 0;
				m_pmap->m_blockcount--;

				for (i = 0; i < JsCPPUtils_LinkedList_FREECACHESIZE; i++)
				{
					if (m_pmap->m_freecache[i] == 0)
					{
						m_pmap->m_freecache[i] = m_curidx;
						break;
					}
				}
			}
		};

	public:
		explicit basic_LinkedListNTS(int _initial_numofblocks = 256, int _conf_incblocksize = 16
#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
			, JsCUtils_fnMalloc_t _custom_malloc = malloc
			, JsCUtils_fnRealloc_t _custom_realloc = realloc
			, JsCUtils_fnFree_t _custom_free = free
#endif
		) : 
			m_blocks(NULL)
			, m_freed(false)
			, m_lastfoundfreeblockidx(0)
			, m_conf_incblocksize(_conf_incblocksize)
			, m_first(0)
			, m_last(0)
#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
			, m_custom_malloc(_custom_malloc)
			, m_custom_realloc(_custom_realloc)
			, m_custom_free(_custom_free)
#endif
		{
			blockindex_t bi;

			if (_initial_numofblocks < 0)
				_initial_numofblocks = 256;
			if (_conf_incblocksize < 0)
				_conf_incblocksize = 16;

			m_blocksize = _initial_numofblocks;
			m_blockcount = 0;

#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
			m_blocks = (block_t*)m_custom_malloc(sizeof(block_t) * m_blocksize); // An exception may occur / std::bad_alloc
#else
			m_blocks = (block_t*)malloc(sizeof(block_t) * m_blocksize); // An exception may occur / std::bad_alloc
#endif

			if (m_blocks == NULL)
			{
				throw std::bad_alloc();
			}
			memset(m_blocks, 0, sizeof(block_t) * m_blocksize);

			memset(m_freecache, 0, sizeof(blockindex_t) * JsCPPUtils_LinkedList_FREECACHESIZE);
			for (bi = 0; (bi < JsCPPUtils_LinkedList_FREECACHESIZE) && (bi < m_blocksize); bi++)
			{
				m_freecache[bi] = bi + 1;
			}
			if (bi < m_blocksize)
				m_lastfoundfreeblockidx = bi + 1;
		}

		~basic_LinkedListNTS()
		{
			m_freed = true;
			//m_blocksize = 0;
			//m_blockcount = 0;
			if (m_blocks != NULL)
			{
				if (is_class<TVALUE>::value)
				{
					blockindex_t bi;
					for (bi = 0; bi < m_blocksize; bi++)
					{
						block_t *pblock = &m_blocks[bi];
						if (pblock->used = 1)
						{
							pblock->value.~TVALUE();
						}
						pblock->used = 0;
					}
				}
#ifdef _JSCUTILS_USE_CUSTOM_ALLOCATOR
				m_custom_free(m_blocks);
#else
				free(m_blocks);
#endif
				m_blocks = NULL;
			}
		}

		/*
		// Move constructor.
		basic_LinkedListNTS(const basic_HashMap<TKEY, TVALUE, _numofbuckets, _numofinitialblocks, _incblocksize>&& _ref)
		{
		m_blocks = _ref.m_blocks;
		m_blocksize = _ref.m_blocksize;
		m_blockcount = _ref.m_blockcount;
		m_buckets = _ref.m_buckets;
		m_poly = _ref.m_poly;
		}
		*/

	private:
		block_t *_getunusedblock(blockindex_t *pblockindex)
		{
			int i;

			if (m_blockcount == m_blocksize)
			{
				int new_blocksize = m_blocksize + m_conf_incblocksize;
				block_t *new_blocks = (block_t*)realloc(m_blocks, sizeof(block_t)*new_blocksize);
				if (new_blocks == NULL)
					throw std::bad_alloc();

				memset(new_blocks + m_blocksize, 0, sizeof(block_t)*m_conf_incblocksize);

				m_blocksize = new_blocksize;
				m_blocks = new_blocks;

				*pblockindex = m_blockcount + 1;

				for (i = 1; (i <= JsCPPUtils_LinkedList_FREECACHESIZE) && (i < m_conf_incblocksize); i++)
				{
					m_freecache[i - 1] = m_blockcount + 1 + i;
				}
				if (i < m_conf_incblocksize)
					m_lastfoundfreeblockidx = m_blockcount + 1 + i;

				return &new_blocks[m_blockcount];
			}
			else if (m_blockcount > m_blocksize) {
				throw std::exception();
			}
			else {
				blockindex_t bi;
				block_t *pblock = NULL;
				block_t *tmppblock;
				int cacheidx = 0;

				for (i = 0; i < JsCPPUtils_LinkedList_FREECACHESIZE; i++)
				{
					blockindex_t tempblockidx = m_freecache[i];
					if (tempblockidx)
					{
						*pblockindex = tempblockidx;
						pblock = &m_blocks[tempblockidx - 1];
						m_freecache[i] = 0;
						return pblock;
					}
				}

				// Cache is empty.

				tmppblock = m_blocks + (m_lastfoundfreeblockidx - 1);
				for (bi = m_lastfoundfreeblockidx; (bi <= m_blocksize) && (cacheidx <= JsCPPUtils_LinkedList_FREECACHESIZE); bi++, tmppblock++)
				{
					if (tmppblock->used == 0)
					{
						if (pblock == NULL)
						{
							pblock = tmppblock;
							memset(pblock, 0, sizeof(block_t));
							*pblockindex = bi;
						}
						else {
							m_freecache[cacheidx++] = bi;
						}
					}
				}
				if (pblock == NULL)
				{
					tmppblock = m_blocks;
					for (bi = 1; (bi < m_lastfoundfreeblockidx) && (cacheidx <= JsCPPUtils_LinkedList_FREECACHESIZE); bi++, tmppblock++)
					{
						if (tmppblock->used == 0)
						{
							if (pblock == NULL)
							{
								pblock = tmppblock;
								memset(pblock, 0, sizeof(block_t));
								*pblockindex = bi;
							}
							else {
								m_freecache[cacheidx++] = bi;
							}
						}
					}
				}

				if (pblock != NULL)
					return pblock;

				throw std::exception();
			}
		}

	public:

		/*
		void erase(const TKEY &key)
		{
			int i;
			int hash = _hash(key);
			blockindex_t *pbucket;
			blockindex_t blockindex = 0;
			block_t *pblock;
			block_t *pprevblock = NULL;
			block_t *pnextblock = NULL;

			pbucket = &m_buckets[hash];
			pblock = _findblock(*pbucket, key, &blockindex);
			if (pblock != NULL)
			{
				if (pblock->next > 0)
				{
					pnextblock = &m_blocks[pblock->next - 1];
					pnextblock->prev = pblock->prev;
				}
				if (pblock->prev > 0)
				{
					pprevblock = &m_blocks[pblock->prev - 1];
					pprevblock->next = pblock->next;
				}
				else {
					*pbucket = pblock->next;
				}

				if (is_class<TKEY>::value)
					pblock->value.~TVALUE();

				pblock->used = 0;
				pblock->next = 0;
				m_blockcount--;

				for (i = 0; i < JsCPPUtils_HashMap_FREECACHESIZE; i++)
				{
					if (m_freecache[i] == 0)
					{
						m_freecache[i] = blockindex;
						break;
					}
				}
			}
		}
		*/

		blockindex_t size()
		{
			return m_blockcount;
		}

		template<typename _Ty>
		Iterator find(const _Ty &value)
		{
			Iterator iter;
			blockindex_t idx = 0;
			blockindex_t foundidx = 0;

			for (idx = m_first; idx != 0; )
			{
				block_t *pblock = &m_blocks[idx - 1];
				if (pblock->value == value)
				{
					foundidx = idx;
					break;
				}
				idx = pblock->next;
			}

			iter.m_pmap = this;
			iter.m_curidx = 0;
			iter.m_nextidx = foundidx;
			iter.m_previdx = 0;

			return iter;
		}

		Iterator begin()
		{
			Iterator iter;

			iter.m_pmap = this;
			iter.m_curidx = 0;
			iter.m_nextidx = m_first;
			iter.m_previdx = 0;

			return iter;
		}

		Iterator end()
		{
			Iterator iter;

			iter.m_pmap = this;
			iter.m_curidx = 0;
			iter.m_nextidx = 0;
			iter.m_previdx = m_last;

			return iter;
		}

		void push_front(const TVALUE &value)
		{
			blockindex_t blockindex = 0;
			block_t *pblock;
			block_t *poldblock;
			pblock = _getunusedblock(&blockindex); // An exception may occur / std::bad_alloc
			pblock->used = 1;
			pblock->prev = 0;
			pblock->next = m_first;
			pblock->value = value;
			m_blockcount++;

			if (m_first != 0)
			{
				poldblock = &m_blocks[m_first - 1];
				poldblock->prev = blockindex;
			}
			
			if(m_last == 0)
				m_last = blockindex;
			m_first = blockindex;
		}

		void push_back(const TVALUE &value)
		{
			blockindex_t blockindex = 0;
			block_t *pblock;
			block_t *poldblock;
			pblock = _getunusedblock(&blockindex); // An exception may occur / std::bad_alloc
			pblock->used = 1;
			pblock->prev = m_last;
			pblock->next = 0;
			pblock->value = value;
			m_blockcount++;

			if (m_last != 0)
			{
				poldblock = &m_blocks[m_last - 1];
				poldblock->next = blockindex;
			}
			if(m_first == 0)
				m_first = blockindex;
			m_last = blockindex;
		}
	};
}

#endif /* __JSCPPUTILS_LINKEDLIST_H__ */
