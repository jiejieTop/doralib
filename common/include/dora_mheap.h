/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-03 17:33:18
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-04 17:52:20
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

/*----------------------------------------------------------------------------
 * Tencent is pleased to support the open source community by making TencentOS
 * available.
 *
 * Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
 * If you have downloaded a copy of the TencentOS binary from Tencent, please
 * note that the TencentOS binary is licensed under the BSD 3-Clause License.
 *
 * If you have downloaded a copy of the TencentOS source code from Tencent,
 * please note that TencentOS source code is licensed under the BSD 3-Clause
 * License, except for the third-party components listed below which are
 * subject to different license terms. Your integration of TencentOS into your
 * own projects may require compliance with the BSD 3-Clause License, as well
 * as the other licenses applicable to the third-party components included
 * within TencentOS.
 *---------------------------------------------------------------------------*/

/*
** Two Level Segregated Fit memory allocator, version 3.1.
** Written by Matthew Conte
** http://tlsf.baisoku.org
**
** Based on the original documentation by Miguel Masmano:
** http://www.gii.upv.es/tlsf/main/docs
**
** This implementation was written to the specification
** of the document, therefore no GPL restrictions apply.
**
** Copyright (c) 2006-2016, Matthew Conte
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the copyright holder nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL MATTHEW CONTE BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _DORA_MHEAP_H_
#define _DORA_MHEAP_H_

#include "stddef.h"
#include "stdint.h"
#include <mutex>
#include <vector>

namespace doralib {

#define MHEAP_DEFAULT_SIZE          (50 * 1024)

/**
 * log2 of number of linear subdivisions of block sizes. Larger
 * values require more memory in the control structure. Values of
 * 4 or 5 are typical.
 */
#define MHEAP_SL_INDEX_COUNT_LOG2    5

/* All allocation sizes and addresses are aligned to 4 bytes. */
#define MHEAP_ALIGN_SIZE_LOG2        2
#define MHEAP_ALIGN_SIZE             (1 << MHEAP_ALIGN_SIZE_LOG2)

/*
 * We support allocations of sizes up to (1 << MHEAP_FL_INDEX_MAX) bits.
 * However, because we linearly subdivide the second-level lists, and
 * our minimum size granularity is 4 bytes, it doesn't make sense to
 * create first-level lists for sizes smaller than MHEAP_SL_INDEX_COUNT * 4,
 * or (1 << (MHEAP_SL_INDEX_COUNT_LOG2 + 2)) bytes, as there we will be
 * trying to split size ranges into more slots than we have available.
 * Instead, we calculate the minimum threshold size, and place all
 * blocks below that size into the 0th first-level list.
 */
#define MHEAP_FL_INDEX_MAX           30
#define MHEAP_SL_INDEX_COUNT         (1 << MHEAP_SL_INDEX_COUNT_LOG2)
#define MHEAP_FL_INDEX_SHIFT         (MHEAP_SL_INDEX_COUNT_LOG2 + MHEAP_ALIGN_SIZE_LOG2)
#define MHEAP_FL_INDEX_COUNT         (MHEAP_FL_INDEX_MAX - MHEAP_FL_INDEX_SHIFT + 1)

#define MHEAP_SMALL_BLOCK_SIZE       (1 << MHEAP_FL_INDEX_SHIFT)

#define MHEAP_BLOCK_CURR_FREE        (1 << 0)
#define MHEAP_BLOCK_PREV_FREE        (1 << 1)
#define MHEAP_BLOCK_SIZE_MASK        ~(MHEAP_BLOCK_CURR_FREE | MHEAP_BLOCK_PREV_FREE)
#define MHEAP_BLOCK_STATE_MASK       (MHEAP_BLOCK_CURR_FREE | MHEAP_BLOCK_PREV_FREE)

typedef struct mheap_information_st {
    uint32_t    used; /* space is used */
    uint32_t    free; /* space is free */
} mheap_info_t;

/**
 * Block structure.
 *
 * There are several implementation subtleties involved:
 * - The prev_phys_block field is only valid if the previous block is free.
 * - The prev_phys_block field is actually stored at the end of the
 *   previous block. It appears at the beginning of this structure only to
 *   simplify the implementation.
 * - The next_free / prev_free fields are only valid if the block is free.
 */
typedef struct mheap_blk {
    struct mheap_blk *prev_phys_blk;

    size_t size;

    struct mheap_blk *next_free;
    struct mheap_blk *prev_free;
} mheap_blk_t;

/**
 * A free block must be large enough to store its header minus the size of
 * the prev_phys_block field, and no larger than the number of addressable
 * bits for FL_INDEX.
 */
#define DORA_OFFSET_OF_FIELD(type, field)    \
    ((size_t)&(((type *)0)->field))

#define DORA_CONTAINER_OF_FIELD(ptr, type, field)    \
    ((type *)((size_t)(ptr) - DORA_OFFSET_OF_FIELD(type, field)))

#define DORA_COUNT_OF(array)        (sizeof(array) / sizeof(array[0]))

#define MHEAP_BLK_SIZE_MIN          (sizeof(mheap_blk_t) - sizeof(mheap_blk_t *))
#define MHEAP_BLK_SIZE_MAX          (1 << MHEAP_FL_INDEX_MAX)

#define MHEAP_BLK_HEADER_OVERHEAD   (sizeof(size_t))
#define MHEAP_BLK_START_OFFSET      (DORA_OFFSET_OF_FIELD(mheap_blk_t, size) + sizeof(size_t))

class mheap {
private:
    std::vector<char *>     _mheap_ptr;
    std::vector<void *>     _pool_start;
    mheap_blk_t             _block_null;    /* empty lists point at this block to indicate they are free. */
    uint32_t                _fl_bitmap;     /* bitmaps for free lists. */
    uint32_t                _sl_bitmap[MHEAP_FL_INDEX_COUNT];
    mheap_blk_t             *_blocks[MHEAP_FL_INDEX_COUNT][MHEAP_SL_INDEX_COUNT]; /**< Head of free lists. */
    std::mutex              _mutex;

    void insert_free_block(mheap_blk_t *blk, int fl, int sl);
    void remove_free_block(mheap_blk_t *blk, int fl, int sl);
    void blk_remove(mheap_blk_t *blk);
    void blk_insert(mheap_blk_t *blk);
    mheap_blk_t *blk_merge_prev(mheap_blk_t *blk);
    mheap_blk_t* blk_merge_next(mheap_blk_t *blk);
    void blk_trim_free(mheap_blk_t *blk, size_t size);
    void blk_trim_used(mheap_blk_t *blk, size_t size);
    mheap_blk_t* blk_trim_free_leading(mheap_blk_t *blk, size_t size);
    mheap_blk_t *blk_search_suitable(int *fli, int *sli);
    mheap_blk_t* blk_locate_free(size_t size);
    void* blk_prepare_used(mheap_blk_t *blk, size_t size);
    bool mheap_pool_is_exist(void *pool_start);
    void mheap_pool_record(void *pool_start);
    void mheap_pool_unrecord(void *pool_start);
    void mheap_ctl_init(void);
    
public:
    mheap();
    mheap(size_t size);
    mheap(const mheap&) = delete;
    ~mheap();

    /**
     * @brief add a pool.
     * add addtional pool to the heap.
     * @param[in]   pool_start  start address of the pool.
     * @param[in]   pool_size   size of the pool.
     * @retval  DORA_VALUE_INVALID : start address of the pool is invalid or size of the pool is invalid.
     * @retval  DORA_VALUE_ALREADY_EXIST : the pool is already exist.
     * @retval  DORA_SUCCESS : return successfully.
     */
    int mheap_pool_add(void *pool_start, size_t pool_size);

    /**
     * @brief delete a pool.
     * delete a pool from the heap.
     * @param[in]   pool_start  start address of the pool.
     * @retval  DORA_VALUE_INVALID : start address of the pool is invalid
     * @retval  DORA_VALUE_ALREADY_NOT_EXIST : the pool is not exist
     * @retval  DORA_SUCCESS : return successfully.
     */
    int mheap_pool_del(void *pool_start);

    /**
     * @brief alloc memory.
     * allocate size bytes and returns a pointer to the allocated memory.
     * @attention size should no bigger than MHEAP_BLK_SIZE_MAX.
     * @param[in]   size    size of the memory.
     * @return  the pointer to the allocated memory.
     */
    void   *mheap_alloc(size_t size);
    void   *mheap_calloc(size_t num, size_t size);

    /**
     * @brief alloc start address aligned memory from the heap.
     * alloc aligned address and specified size memory from the heap.
     * @param[in]   size    size of the memory.
     * @param[in]   align   address align mask of the memory.
     * @return  the pointer to the allocated memory.
     */
    void   *mheap_aligned_alloc(size_t size, size_t align);

    /**
     * @brief realloc memory from the heap.
     * change the size of the memory block pointed to by ptr to size bytes.
     * if ptr is NULL, then the call is equivalent to mheap_alloc(size), for all values of size.
     * if ptr is if size is equal to zero, and ptr is not NULL, then the call is equivalent to mheap_free(ptr).
     * @param[in]   ptr     old pointer to the memory space.
     * @param[in]   size    new size of the memory space.
     * @return  the new pointer to the allocated memory.
     */
    void   *mheap_realloc(void *ptr, size_t size);

    /**
     * @brief free the memory.
     * free the memory space pointed to by ptr, which must have been returned by a previous call to mheap_alloc(), mheap_aligned_alloc(), or mheap_realloc().
     * @param[in]   ptr     pointer to the memory.
     */
    void    mheap_free(void *ptr);

    /**
     * @brief check the pool.
     * @param[in]   pool_start  start address of the pool.
     * @param[out]  info        pointer to the information struct.
     */
    int mheap_pool_check(void *pool_start, mheap_info_t *info);

    /**
     * @brief check the heap.
     * @param[out]  info    pointer to the information struct.
     */
    int mheap_check(mheap_info_t *info);
    int mheap_init(void *pool_start, size_t pool_size);

    void mheap_print_info(void);
};

#else /* if mheap is not enabled, use libc instead */

#define mheap_alloc            malloc
#define mheap_calloc           calloc
#define mheap_realloc          realloc
#define mheap_free             free

#endif /* _DORA_MHEAP_H_ */

}
