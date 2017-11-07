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
		class basic_HashMapNTS
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
		
			inline int _hash(const TKEY &key)
			{
				int i;
				uint32_t poly = m_poly;
				uint32_t hval = 0;
				const unsigned char *pkey = (unsigned char*)&key;
				const unsigned char *pkeye = pkey + sizeof(key);

				// FNV-A
	
				if (sizeof(key) > 4)
				{
					for (i = 0; i < (sizeof(key) >> 2); i++)
					{
						hval ^= *((uint32_t*)pkey);
						hval *= 0x01000193;
						pkey += 4;
					}
				}
				while (pkey < pkeye)
				{
					hval ^= *pkey++;
					hval *= 0x01000193;
				}
	
				return hval % m_numofbuckets;
			}
		
			inline int _hash2(const TKEY &key, uint32_t numofbuckets)
			{
				int i;
				uint32_t poly = m_poly;
				uint32_t hval = 0;
				const unsigned char *pkey = (unsigned char*)&key;
				const unsigned char *pkeye = pkey + sizeof(key);

				// FNV-A
	
				if (sizeof(key) > 4)
				{
					for (i = 0; i < (sizeof(key) >> 2); i++)
					{
						hval ^= *((uint32_t*)pkey);
						hval *= 0x01000193;
						pkey += 4;
					}
				}
				while (pkey < pkeye)
				{
					hval ^= *pkey++;
					hval *= 0x01000193;
				}
	
				return hval % numofbuckets;
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
			class Iterator
			{
			private:
				friend class basic_HashMapNTS<TKEY, TVALUE>;
				
				int m_itertype; // 0 : all, 1 : special item
				
				basic_HashMapNTS<TKEY, TVALUE> *m_pmap;
				blockindex_t m_nextidx;
				blockindex_t m_curidx;
				
				blockindex_t m_remaincount;
				
			private:
				void _findnext()
				{
					if (m_itertype == 0)
					{
						blockindex_t bi;
						blockindex_t nextbi = 0;
					
						for (bi = m_nextidx; bi < m_pmap->m_blocksize; bi++)
						{
							block_t *pblock = &m_pmap->m_blocks[bi];
							if (pblock->used == 1)
							{
								nextbi = bi + 1;
								break;
							}
						}
					
						if (nextbi == 0)
							m_nextidx = m_pmap->m_blocksize + 1;
						else
							m_nextidx = nextbi;
					}
					else // if(m_itertype == 1)
					{
						m_nextidx = 0;
					}
				}
				
			public:
				Iterator()
				{
					m_pmap = NULL;
					m_nextidx = 0;
					m_curidx = 0;
					m_itertype = -1;
					m_remaincount = 0;
				}
				
				Iterator(const Iterator& _ref)
				{
					m_pmap = _ref.m_pmap;
					m_nextidx = _ref.m_nextidx;
					m_curidx = _ref.m_curidx;
					m_itertype = _ref.m_itertype;
					m_remaincount = _ref.m_remaincount;
				}
				
				bool hasNext()
				{
					return (m_remaincount > 0); //((m_nextidx > 0) && (m_nextidx < (m_pmap->m_blocksize + 1))) ;
				}
				
				TVALUE &next()
				{
					m_curidx = m_nextidx;
					m_remaincount--;
					if (m_remaincount > 0)
						_findnext();
					return m_pmap->m_blocks[m_curidx - 1].value;
				}
				
				TVALUE *getValuePtr()
				{
					return &m_pmap->m_blocks[m_curidx - 1].value;
				}
				
				TKEY getKey()
				{
					return m_pmap->m_blocks[m_curidx - 1].key;
				}
				
				void erase()
				{
					block_t *pblock = &m_pmap->m_blocks[m_curidx - 1];
					block_t *pprevblock = NULL;
					block_t *pnextblock = NULL;
					int hash = m_pmap->_hash(pblock->key);
					blockindex_t *pbucket = &m_pmap->m_buckets[hash];
					int i;
					
					if (pblock->next > 0)
					{
						pnextblock = &m_pmap->m_blocks[pblock->next - 1];
						pnextblock->prev = pblock->prev;
					}
					if (pblock->prev > 0)
					{
						pprevblock = &m_pmap->m_blocks[pblock->prev - 1];
						pprevblock->next = pblock->next;
					}else{
						*pbucket = pblock->next;
					}
					
					if (is_class<TVALUE>::value)
						pblock->value.~TVALUE();
					
					pblock->used = 0;
					pblock->next = 0;
					m_pmap->m_blockcount--;
					
					for (i = 0; i < JsCPPUtils_HashMap_FREECACHESIZE; i++)
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
			explicit basic_HashMapNTS(int _initial_numofbuckets = 127, int _initial_numofblocks = 256, int _conf_incblocksize = 16, float _conf_incbucketsthresholdratio = 0.8, float _conf_incbucketfactor = 2.0, int _conf_limitnumofbuckets = 4194304)
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
			
			~basic_HashMapNTS()
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
					if (is_class<TKEY>::value)
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
			
			block_t *_getblock(const TKEY &key, blockindex_t *pretblockindex = NULL)
			{
				int hash = _hash(key);
				
				blockindex_t *pbucket = &m_buckets[hash];
				block_t *pblock = _findblock(*pbucket, key, pretblockindex);
				
				// new block
				if (pblock == NULL)
				{
					blockindex_t blockindex = 0;
					
					if (_checkbucketstate())
					{
						hash = _hash(key);
						pbucket = &m_buckets[hash];
					}
					
					pblock = _getunusedblock(&blockindex); // An exception may occur / std::bad_alloc
					
					pblock->used = 1;
					memcpy(&pblock->key, &key, sizeof(TKEY));
					pblock->next = 0;
					memset(&pblock->value, 0, sizeof(TVALUE));
					m_blockcount++;
					
					if (pretblockindex)
						*pretblockindex = blockindex;
					
					{
						WrappedClass<TVALUE> wrappedCls;
						wrappedCls.callconstructor(&pblock->value);
					}
					
					
					if (*pbucket == 0)
					{
						*pbucket = blockindex;
						pblock->prev = 0;
					}else{
						blockindex_t tmpblockidx = *pbucket;
						while (tmpblockidx)
						{
							block_t *ptmpblock = &m_blocks[tmpblockidx - 1];
							if (ptmpblock->next == 0)
							{
								ptmpblock->next = blockindex;
								pblock->prev = tmpblockidx;
								break;
							}
							tmpblockidx = ptmpblock->next;
						}
					}
				}
				
				return pblock;
			}
			
			block_t *_findblock(blockindex_t bucket, const TKEY &key, blockindex_t *pretblockindex = NULL)
			{
				if (bucket != 0)
				{
					block_t *ptmpprevblock = NULL;
					blockindex_t tmpblockidx = bucket;
					while (tmpblockidx)
					{
						block_t *ptmpblock = &m_blocks[tmpblockidx - 1];
						if (memcmp(&ptmpblock->key, &key, sizeof(TKEY)) == 0)
						{
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
							}else{
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
								}else{
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
		
			const TVALUE& operator[](const TKEY &key) const
			{
				const block_t *pblock;
				
				pblock = _getblock(key);
				const TVALUE& ref_value = pblock->value;
				
				return ref_value;
			}
		
			TVALUE& operator[](const TKEY &key)
			{
				block_t *pblock;
				
				pblock = _getblock(key);
				TVALUE& ref_value = pblock->value;
				
				return ref_value;
			}
			
			bool isContain(const TKEY &key)
			{
				int hash = _hash(key);
				
				blockindex_t *pbucket = &m_buckets[hash];
				block_t *pblock = _findblock(*pbucket, key, NULL);
				
				return (pblock != NULL);
			}
			
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
					}else{
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
			
			Iterator iterator()
			{
				Iterator iter;
				iter.m_itertype = 0;
				iter.m_pmap = this;
				iter.m_nextidx = 0;
				iter.m_remaincount = m_blockcount;
				iter._findnext();
				return iter;
			}
			
			Iterator find(const TKEY &key)
			{
				Iterator iter;
				blockindex_t nextbi = 0;
				
				int hash = _hash(key);
				blockindex_t bucket = m_buckets[hash];
				block_t *pblock = _findblock(bucket, key, &nextbi);
				
				iter.m_itertype = 1;
				iter.m_pmap = this;
				iter.m_nextidx = nextbi;
				iter.m_remaincount = (pblock != NULL) ? 1 : 0; 
				
				return iter;
			}
			
			Iterator use(const TKEY &key)
			{
				Iterator iter;
				block_t *pblock;
				blockindex_t nextbi = 0;
				
				pblock = _getblock(key, &nextbi);
				
				iter.m_itertype = 1;
				iter.m_pmap = this;
				iter.m_nextidx = nextbi;
				iter.m_remaincount = 1;
				
				return iter;
			}
	};
	
	
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
		class HashMap : public basic_HashMapNTS<TKEY, TVALUE>, private Lockable
		{
		public:
			explicit HashMap(int _initial_numofbuckets = 127, int _initial_numofblocks = 256, int _conf_incblocksize = 16, float _conf_incbucketsthresholdratio = 0.8, float _conf_incbucketfactor = 2.0, int _conf_limitnumofbuckets = 4194304)
				: basic_HashMapNTS<TKEY, TVALUE>(_initial_numofbuckets, _initial_numofblocks, _conf_incblocksize, _conf_incbucketsthresholdratio, _conf_incbucketfactor, _conf_limitnumofbuckets)
			{
			}
			
			~HashMap()
			{
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
			
			void iteratoring_lock()
			{
				lock();
			}
			
			void iteratoring_unlock()
			{
				unlock();
			}
			
			// std::bad_alloc
			const TVALUE& operator[](const TKEY &key) const
			{
				lock();
				
				const TVALUE& ref_value = basic_HashMapNTS<TKEY, TVALUE>::operator[](key);
				
				unlock();
				
				return ref_value;
			}
		
			TVALUE& operator[](const TKEY &key)
			{
				lock();
				
				TVALUE& ref_value = basic_HashMapNTS<TKEY, TVALUE>::operator[](key);
				
				unlock();
				
				return ref_value;
			}
			
			void erase(const TKEY &key)
			{
				lock();
				basic_HashMapNTS<TKEY, TVALUE>::erase(key);
				unlock();
			}
		};
	
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
		class HashMapRWLock : public basic_HashMapNTS<TKEY, TVALUE>, private LockableRW
		{
		public:
			explicit HashMapRWLock(int _initial_numofbuckets = 127, int _initial_numofblocks = 256, int _conf_incblocksize = 16, float _conf_incbucketsthresholdratio = 0.8, float _conf_incbucketfactor = 2.0, int _conf_limitnumofbuckets = 4194304)
				: basic_HashMapNTS<TKEY, TVALUE>(_initial_numofbuckets, _initial_numofblocks, _conf_incblocksize, _conf_incbucketsthresholdratio, _conf_incbucketfactor, _conf_limitnumofbuckets)
			{
			}
			
			~HashMapRWLock()
			{
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
			
			void iteratoring_readlock()
			{
				readlock();
			}
			
			void iteratoring_readunlock()
			{
				readunlock();
			}
			
			void iteratoring_writelock()
			{
				writelock();
			}
			
			void iteratoring_writeunlock()
			{
				writeunlock();
			}
			
			// std::bad_alloc
			const TVALUE& operator[](const TKEY &key) const
			{
				writelock();
				
				const TVALUE& ref_value = basic_HashMapNTS<TKEY, TVALUE>::operator[](key);
				
				writeunlock();
				
				return ref_value;
			}
		
			TVALUE& operator[](const TKEY &key)
			{
				writelock();
				
				TVALUE& ref_value = basic_HashMapNTS<TKEY, TVALUE>::operator[](key);
				
				writeunlock();
				
				return ref_value;
			}
			
			void erase(const TKEY &key)
			{
				writelock();
				basic_HashMapNTS<TKEY, TVALUE>::erase(key);
				writeunlock();
			}
		};
}

#endif /* __JSCPPUTILS_HASHMAP_H__ */
