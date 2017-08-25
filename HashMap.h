/**
 * @file	HashMap.h
 * @class	HashMap
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2017/04/12
 * @brief	thread-safe
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_HASHMAP_H__
#define __JSCPPUTILS_HASHMAP_H__

#include <new>
#include <exception>

#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "Lockable.h"

namespace JsCPPUtils
{
#define JsCPPUtils_HashMap_FREECACHESIZE 64
	
	/**
	 * TKEY						Key의 type
	 * TVALUE					Value의 type
	 * _initial_numofbuckets		Key에 대한 Bucket수
	 * _initial_numofblocks		초기 블록 수 (데이터 수)
	 * _incblocksize			지정한 블록 수 보다 데이터가 많아질 때 증가시길 블록 수
	 * _conf_incbucketsthresholdratio	버킷을 증가할 데이터개수/현재버킷개수 비율의 한계점
	 * _conf_incbucketfactor			버킷 증가 비율
	 * _conf_limitnumofbuckets			최대 버킷 수
	 */
	template<typename TKEY, typename TVALUE>
		class HashMap : private Lockable
		{
		private:
			template<typename T>
				struct Check_If_T_Is_Class_Type
				{
					template<typename C> static char func(char C::*p);
					template<typename C> static int func(...);
					enum{val = sizeof(Check_If_T_Is_Class_Type<T>::template func<T>(0)) == 1};
				};
			
			template<typename T, bool bIsClass = Check_If_T_Is_Class_Type<T>::val>
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
			
			typedef uint32_t blockindex_t;
			struct _tag_block;
			typedef struct _tag_block
			{
				int used;
				blockindex_t next;
				TKEY key;
				TVALUE value;
			} block_t;

			float m_conf_incbucketsthresholdratio; ///< 버킷을 증가할 데이터개수/현재버킷개수 비율의 한계점
			float m_conf_incbucketfactor; ///< 버킷 증가 비율
			int m_conf_limitnumofbuckets; ///< 최대 버킷 수
			int m_conf_incblocksize; ///< 지정한 블록 수 보다 데이터가 많아질 때 증가시길 블록 수

			int m_numofbuckets;
			blockindex_t m_blocksize;
			blockindex_t m_blockcount;
			
			block_t *m_blocks;
			blockindex_t *m_buckets;
			uint32_t m_poly;
			
			blockindex_t m_freecache[JsCPPUtils_HashMap_FREECACHESIZE];
			blockindex_t m_lastfoundfreeblockidx;
			
			bool m_freed;
		
			int _hash(const TKEY &key)
			{
				int i;
				uint32_t poly = m_poly;
				uint32_t hash = 0;
				unsigned char *pkey = (unsigned char*)&key;
				for (i = 0; i < sizeof(key); i++)
				{
					poly = (poly << 1) | (poly >> (32 - 1)); // 1bit Left Shift
					hash = (uint32_t)(poly * hash + pkey[i]);
				}
				return hash % m_numofbuckets;
			}
		
			int _hash2(const TKEY &key, uint32_t numofbuckets)
			{
				int i;
				uint32_t poly = m_poly;
				uint32_t hash = 0;
				unsigned char *pkey = (unsigned char*)&key;
				for (i = 0; i < sizeof(key); i++)
				{
					poly = (poly << 1) | (poly >> (32 - 1)); // 1bit Left Shift
					hash = (uint32_t)(poly * hash + pkey[i]);
				}
				return hash % numofbuckets;
			}

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
			explicit HashMap(int _initial_numofbuckets = 127, int _initial_numofblocks = 256, int _conf_incblocksize = 16, float _conf_incbucketsthresholdratio = 0.8, float _conf_incbucketfactor = 2.0, int _conf_limitnumofbuckets = 4194304)
				: m_poly(0x741B8CD7)
				, m_buckets(NULL)
				, m_blocks(NULL)
				, m_freed(false)
				, m_lastfoundfreeblockidx(0)
				, m_conf_incblocksize(_conf_incblocksize)
				, m_conf_incbucketsthresholdratio(_conf_incbucketsthresholdratio)
				, m_conf_incbucketfactor(_conf_incbucketfactor)
				, m_conf_limitnumofbuckets(_conf_limitnumofbuckets)
			{
				blockindex_t bi;
				
				m_numofbuckets = _initial_numofbuckets;
				m_blocksize = _initial_numofblocks;
				m_blockcount = 0;
				
				m_buckets = (blockindex_t*)malloc(sizeof(blockindex_t) * m_numofbuckets); // An exception may occur / std::bad_alloc
				if (m_buckets == NULL)
					throw std::bad_alloc();
				memset(m_buckets, 0, sizeof(blockindex_t) * m_numofbuckets);
				
				m_blocks = (block_t*)malloc(sizeof(block_t) * m_blocksize); // An exception may occur / std::bad_alloc
				if (m_blocks == NULL)
				{
					free(m_buckets); m_buckets = NULL;
					throw std::bad_alloc();
				}
				memset(m_blocks, 0, sizeof(block_t) * m_blocksize);
				
				memset(m_freecache, 0, sizeof(blockindex_t) * JsCPPUtils_HashMap_FREECACHESIZE);
				for (bi = 0; (bi < JsCPPUtils_HashMap_FREECACHESIZE) && (bi < m_blocksize); bi++)
				{
					m_freecache[bi] = bi + 1;
				}
				if (bi < m_blocksize)
					m_lastfoundfreeblockidx = bi + 1;
			}
			
			~HashMap()
			{
				m_freed = true;
				//m_blocksize = 0;
				//m_blockcount = 0;
				if (m_buckets != NULL)
				{
					free(m_buckets);
					m_buckets = NULL;
				}
				if (m_blocks != NULL)
				{
					free(m_blocks);
					m_blocks = NULL;
				}
			}
			
			/*
			// Move constructor.  
			basic_HashMap(const basic_HashMap<TKEY, TVALUE, _numofbuckets, _numofinitialblocks, _incblocksize>&& _ref)
			{
				m_blocks = _ref.m_blocks;
				m_blocksize = _ref.m_blocksize;
				m_blockcount = _ref.m_blockcount;
				m_buckets = _ref.m_buckets;
				m_poly = _ref.m_poly;
			}
			*/
		
			// std::bad_alloc
			const TVALUE& operator[](const TKEY &key) const
			{
				block_t *pblock;
				
				lock();
				
				pblock = _getblock(key);
				const TVALUE& ref_value = pblock->value;
				
				unlock();
				
				return ref_value;
			}
		
			TVALUE& operator[](const TKEY &key)
			{
				block_t *pblock;
				
				lock();
				
				pblock = _getblock(key);
				TVALUE& ref_value = pblock->value;
				
				unlock();
				
				return ref_value;
			}
			
			void erase(const TKEY &key)
			{
				int i;
				int hash = _hash(key);
				blockindex_t *pbucket;
				blockindex_t blockindex = 0;
				block_t *pblock;
				block_t *pprevblock = NULL;
				
				lock();
				
				pbucket = &m_buckets[hash];
				pblock = _findblock(*pbucket, key, &pprevblock, &blockindex);
				if (pblock != NULL)
				{
					if (pprevblock == NULL)
						*pbucket = pblock->next;
					else
						pprevblock->next = pblock->next;
					
					for (i = 0; i < JsCPPUtils_HashMap_FREECACHESIZE; i++)
					{
						if (m_freecache[i] == 0)
						{
							m_freecache[i] = blockindex;
							break;
						}
					}
					
					if (Check_If_T_Is_Class_Type<TVALUE>::val)
						pblock->value.~TVALUE();
					
					//blockindex
					
					pblock->used = 0;
					pblock->next = 0;
					m_blockcount--;
				}
				
				unlock();
			}
			
		private:
			bool _checkbucketstate()
			{
				float curbdr = (float)((double)m_blockcount / (double)m_numofbuckets);
				if (curbdr >= m_conf_incbucketsthresholdratio)
				{
					int nresult = 0;
					do
					{
						blockindex_t *new_buckets;
						int new_numofbuckets = (int)((float)m_numofbuckets * m_conf_incbucketfactor);
						blockindex_t i;
						
						if (new_numofbuckets > m_conf_limitnumofbuckets)
						{
							nresult = 2;
							break;
						}
						
						new_buckets = (blockindex_t*)malloc(sizeof(blockindex_t) * new_numofbuckets); // An exception may occur / std::bad_alloc
						if (new_buckets == NULL)
						{
							nresult = -1;
							//nresult = -ENOMEM;
							break;
						}
						memset(new_buckets, 0, sizeof(blockindex_t) * new_numofbuckets);
						
						for (i = 0; i < m_blocksize; i++)
						{
							block_t *pblock = &m_blocks[i];
							if (pblock->used)
							{
								int newhash = _hash2(pblock->key, new_numofbuckets);
								blockindex_t *pbucket = &new_buckets[newhash];
								pblock->next = 0;
								if (*pbucket == 0)
								{
									*pbucket = i + 1;
								}
								else {
									blockindex_t tmpblockidx = *pbucket;
									block_t *tmppblock = NULL;
									while (tmpblockidx)
									{
										tmppblock = &m_blocks[tmpblockidx - 1];
										tmpblockidx = tmppblock->next;
									}
									tmppblock->next = i + 1;
								}
							}
						}
						
						if (m_buckets != NULL)
						{
							free(m_buckets);
						}
						m_buckets = new_buckets;
						m_numofbuckets = new_numofbuckets;
						nresult = 1;
					} while (0);
					if (nresult != 1)
					{
						//printf("inc bucket failed\n");
					}
					else {
						return true;
					}
				}
				return false;
			}
			
			block_t *_getblock(const TKEY &key)
			{
				int hash = _hash(key);
				
				blockindex_t *pbucket = &m_buckets[hash];
				block_t *pblock = _findblock(*pbucket, key);
				
				// new block
				if (pblock == NULL)
				{
					blockindex_t blockindex = 0;
					
					if (_checkbucketstate())
					{
						hash = _hash(key);
						pbucket = &m_buckets[hash];
						pblock = _findblock(*pbucket, key);
					}
					
					pblock = _getunusedblock(&blockindex); // An exception may occur / std::bad_alloc
					
					{
						WrappedClass<TVALUE> wrappedCls;
						wrappedCls.callconstructor(&pblock->value);
					}
					
					m_blockcount++;
					
					pblock->used = 1;
					memcpy(&pblock->key, &key, sizeof(TKEY));
					pblock->next = 0;
					memset(&pblock->value, 0, sizeof(TVALUE));
					
					if (*pbucket == 0)
					{
						*pbucket = blockindex;
					}
					else {
						blockindex_t tmpblockidx = *pbucket;
						while (tmpblockidx)
						{
							block_t *ptmpblock = &m_blocks[tmpblockidx - 1];
							if (ptmpblock->next == 0)
							{
								ptmpblock->next = blockindex;
								break;
							}
							tmpblockidx = ptmpblock->next;
						}
					}
				}
				
				return pblock;
			}
			
			block_t *_findblock(blockindex_t bucket, const TKEY &key, block_t **pprevblock = NULL, blockindex_t *pretblockindex = NULL)
			{
				if (bucket != 0)
				{
					block_t *ptmpprevblock = NULL;
					blockindex_t tmpblockidx = bucket;
					while (tmpblockidx)
					{
						block_t *ptmpblock = &m_blocks[tmpblockidx - 1];
						if (ptmpblock->key == key)
						{
							if (pprevblock)
								*pprevblock = ptmpprevblock;
							if (pretblockindex)
								*pretblockindex = tmpblockidx;
							return ptmpblock;
						}
						ptmpprevblock = ptmpblock;
						tmpblockidx = ptmpblock->next;
					}
				}
				return NULL;
			}
			
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
					
					for (i = 1; (i <= JsCPPUtils_HashMap_FREECACHESIZE) && (i < m_conf_incblocksize); i++)
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
					
					for (i = 0; i < JsCPPUtils_HashMap_FREECACHESIZE; i++)
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
					for (bi = m_lastfoundfreeblockidx; (bi <= m_blocksize) && (cacheidx <= JsCPPUtils_HashMap_FREECACHESIZE); bi++, tmppblock++)
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
						for (bi = 1; (bi < m_lastfoundfreeblockidx) && (cacheidx <= JsCPPUtils_HashMap_FREECACHESIZE); bi++, tmppblock++)
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
		};
}

#endif /* __JSCPPUTILS_HASHMAP_H__ */
