/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-03 17:33:18
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-08 11:06:16
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
**	http://tlsf.baisoku.org
**
** Based on the original documentation by Miguel Masmano:
**	http://www.gii.upv.es/tlsf/main/docs
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

#include "dora_mheap.h"
#include "dora_error.h"

namespace doralib {

using doralib::mheap;

#if defined(DORA_USING_SOFTWARE_CLZ)
static int generic_fls(uint32_t x)
{
    int r = 32;
    if (!x)
        return 0;

    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}
#else
static int generic_fls(uint32_t x)
{
    return 32 - __builtin_clz(x);
}
#endif

static int __ffs(uint32_t word)
{
    return generic_fls(word & (~word + 1)) - 1;
}

static int __fls(uint32_t word)
{
    return generic_fls(word) - 1;
}

/*
** TLSF utility functions. In most cases, these are direct translations of
** the documentation found in the white paper.
*/
static void mapping_insert(size_t size, int *fli, int *sli)
{
    int fl, sl;

    if (size < MHEAP_SMALL_BLOCK_SIZE) {
        /* Store small blocks in first list. */
        fl = 0;
        sl = (int)size / (MHEAP_SMALL_BLOCK_SIZE / MHEAP_SL_INDEX_COUNT);
    } else {
        fl = __fls(size);
        sl = ((int)size >> (fl - MHEAP_SL_INDEX_COUNT_LOG2)) ^ (1 << MHEAP_SL_INDEX_COUNT_LOG2);
        fl -= (MHEAP_FL_INDEX_SHIFT - 1);
    }
    *fli = fl;
    *sli = sl;
}

/* This version rounds up to the next block size (for allocations) */
static void mapping_search(size_t size, int *fli, int *sli)
{
    size_t round;

    if (size >= MHEAP_SMALL_BLOCK_SIZE) {
        round = (1 << (__fls(size) - MHEAP_SL_INDEX_COUNT_LOG2)) - 1;
        size += round;
    }
    mapping_insert(size, fli, sli);
}

static size_t blk_size(const mheap_blk_t *blk)
{
    return blk->size & MHEAP_BLOCK_SIZE_MASK;
}

static int blk_is_last(const mheap_blk_t* blk)
{
	return blk_size(blk) == 0;
}

static void blk_set_size(mheap_blk_t *blk, size_t size)
{
    blk->size = size | (blk->size & MHEAP_BLOCK_STATE_MASK);
}

static int blk_is_free(const mheap_blk_t *blk)
{
    return blk->size & MHEAP_BLOCK_CURR_FREE;
}

static void blk_set_free(mheap_blk_t *blk)
{
    blk->size |= MHEAP_BLOCK_CURR_FREE;
}

static void blk_set_used(mheap_blk_t *blk)
{
    blk->size &= ~MHEAP_BLOCK_CURR_FREE;
}

static int blk_is_prev_free(const mheap_blk_t *blk)
{
    return blk->size & MHEAP_BLOCK_PREV_FREE;
}

static void blk_set_prev_free(mheap_blk_t *blk)
{
    blk->size |= MHEAP_BLOCK_PREV_FREE;
}

static void blk_set_prev_used(mheap_blk_t *blk)
{
    blk->size &= ~MHEAP_BLOCK_PREV_FREE;
}

static mheap_blk_t *blk_from_ptr(const void *ptr)
{
    return (mheap_blk_t *)((size_t)ptr - MHEAP_BLK_START_OFFSET);
}

static void *blk_to_ptr(const mheap_blk_t *blk)
{
	return (void *)((size_t)blk + MHEAP_BLK_START_OFFSET);
}

/* Return location of next block after block of given size. */
static mheap_blk_t *offset_to_blk(const void *ptr, int diff)
{
    return (mheap_blk_t *)((size_t)ptr + diff);
}

/* Return location of previous block. */
static mheap_blk_t *blk_prev(const mheap_blk_t *blk)
{
    return blk->prev_phys_blk;
}

/* Return location of next existing block. */
static mheap_blk_t *blk_next(const mheap_blk_t *blk)
{
    mheap_blk_t *next_blk;

    next_blk = offset_to_blk(blk_to_ptr(blk), blk_size(blk) - MHEAP_BLK_HEADER_OVERHEAD);
    return next_blk;
}

/* Link a new block with its physical neighbor, return the neighbor. */
static mheap_blk_t *blk_link_next(mheap_blk_t *blk)
{
    mheap_blk_t *next_blk;

    next_blk = blk_next(blk);
    next_blk->prev_phys_blk = blk;
    return next_blk;
}

static void blk_mark_as_free(mheap_blk_t *blk)
{
    mheap_blk_t *next_blk;

    /* Link the block to the next block, first. */
    next_blk = blk_link_next(blk);
    blk_set_prev_free(next_blk);
    blk_set_free(blk);
}

static void blk_mark_as_used(mheap_blk_t *blk)
{
    mheap_blk_t *next_blk;

    next_blk = blk_next(blk);
    blk_set_prev_used(next_blk);
    blk_set_used(blk);
}

static size_t align_up(size_t x, size_t align)
{
    return (x + (align - 1)) & ~(align - 1);
}

static size_t align_down(size_t x, size_t align)
{
    return x - (x & (align - 1));
}

static void *align_ptr(const void *ptr, size_t align)
{
    return (void *)(((size_t)ptr + (size_t)(align - 1)) & ~(align -1));
}

static int blk_can_split(mheap_blk_t *blk, size_t size)
{
    return blk_size(blk) >= sizeof(mheap_blk_t) + size;
}

/* Split a block into two, the second of which is free. */
static mheap_blk_t *blk_split(mheap_blk_t *blk, size_t size)
{
    mheap_blk_t   *remaining;
    size_t          remain_size;

    /* Calculate the amount of space left in the remaining block. */
    remaining   = offset_to_blk(blk_to_ptr(blk), size - MHEAP_BLK_HEADER_OVERHEAD);
    remain_size = blk_size(blk) - (size + MHEAP_BLK_HEADER_OVERHEAD);

    blk_set_size(remaining, remain_size);

    blk_set_size(blk, size);
    blk_mark_as_free(remaining);

    return remaining;
}

/* Absorb a free block's storage into an adjacent previous free block. */
static mheap_blk_t *blk_absorb(mheap_blk_t *prev_blk, mheap_blk_t *blk)
{
    prev_blk->size += blk_size(blk) + MHEAP_BLK_HEADER_OVERHEAD;
    blk_link_next(prev_blk);
    return prev_blk;
}

/*
** Adjust an allocation size to be aligned to word size, and no smaller
** than internal minimum.
*/
static size_t adjust_request_size(size_t size, size_t align)
{
    size_t adjust_size = 0;

    if (!size) {
        return 0;
    }

    adjust_size = align_up(size, align);
    if (adjust_size > MHEAP_BLK_SIZE_MAX) {
        return 0;
    }

    /* aligned sized must not exceed block_size_max or we'll go out of bounds on sl_bitmap */
    return adjust_size > MHEAP_BLK_SIZE_MIN ? adjust_size : MHEAP_BLK_SIZE_MIN;
}

mheap::mheap()
{
    char *ptr = new char[MHEAP_DEFAULT_SIZE];
    _mheap_ptr.push_back(ptr);
    DORA_ROBUSTNESS_CHECK(ptr, DORA_VOID);
    mheap_init(ptr, MHEAP_DEFAULT_SIZE);
    mheap_print_info();
}

mheap::mheap(size_t size)
{
    char *ptr = new char[size];
    _mheap_ptr.push_back(ptr);
    DORA_ROBUSTNESS_CHECK(ptr, DORA_VOID);
    mheap_init(ptr, size);
    mheap_print_info();
}

mheap::~mheap()
{
    mheap_print_info();
    for (int i = 0; i < (int)_mheap_ptr.size(); i++) {
        if (_mheap_ptr[i]) {
            // DORA_LOG_DEBUG("delet [{}] memory {:p}", i, _mheap_ptr[i]);
            delete _mheap_ptr[i];
        }
    }
}


/* Insert a free block into the free block list. */
void mheap::insert_free_block(mheap_blk_t *blk, int fl, int sl)
{
    mheap_blk_t *curr;
    std::unique_lock<std::mutex> lock(_mutex);

    curr = _blocks[fl][sl];
    blk->next_free = curr;
    blk->prev_free = &_block_null;
    curr->prev_free = blk;

	/*
	** Insert the new block at the head of the list, and mark the first-
	** and second-level bitmaps appropriately.
	*/
    _blocks[fl][sl] = blk;
    _fl_bitmap |= (1 << fl);
    _sl_bitmap[fl] |= (1 << sl);
}

/* Remove a free block from the free list.*/
void mheap::remove_free_block(mheap_blk_t *blk, int fl, int sl)
{
    mheap_blk_t *prev_blk;
    mheap_blk_t *next_blk;
    std::unique_lock<std::mutex> lock(_mutex);

    prev_blk = blk->prev_free;
    next_blk = blk->next_free;
    next_blk->prev_free = prev_blk;
    prev_blk->next_free = next_blk;

    /* If this block is the head of the free list, set new head. */
    if (_blocks[fl][sl] == blk) {
        _blocks[fl][sl] = next_blk;

        /* If the new head is null, clear the bitmap. */
        if (next_blk == &_block_null) {
            _sl_bitmap[fl] &= ~(1 << sl);

            /* If the second bitmap is now empty, clear the fl bitmap. */
            if (!_sl_bitmap[fl]) {
                _fl_bitmap &= ~(1 << fl);
            }
        }
    }
}

/* Remove a given block from the free list. */
void mheap::blk_remove(mheap_blk_t *blk)
{
    int fl, sl;

    mapping_insert(blk_size(blk), &fl, &sl);
    remove_free_block(blk, fl, sl);
}

/* Insert a given block into the free list. */
void mheap::blk_insert(mheap_blk_t *blk)
{
    int fl, sl;

    mapping_insert(blk_size(blk), &fl, &sl);
    insert_free_block(blk, fl, sl);
}

/* Merge a just-freed block with an adjacent previous free block. */
mheap_blk_t* mheap::blk_merge_prev(mheap_blk_t *blk)
{
    mheap_blk_t *prev_blk;

    if (blk_is_prev_free(blk)) {
        prev_blk = blk_prev(blk);
        blk_remove(prev_blk);
        blk = blk_absorb(prev_blk, blk);
    }

    return blk;
}

/* Merge a just-freed block with an adjacent free block. */
mheap_blk_t* mheap::blk_merge_next(mheap_blk_t *blk)
{
    mheap_blk_t *next_blk;

    next_blk = blk_next(blk);
    if (blk_is_free(next_blk)) {
        blk_remove(next_blk);
        blk = blk_absorb(blk, next_blk);
    }

    return blk;
}

/* Trim any trailing block space off the end of a block, return to pool. */
void mheap::blk_trim_free(mheap_blk_t *blk, size_t size)
{
    mheap_blk_t *remaining_blk;

    if (blk_can_split(blk, size)) {
        remaining_blk = blk_split(blk, size);
        blk_link_next(blk);
        blk_set_prev_free(remaining_blk);
        blk_insert(remaining_blk);
    }
}

/* Trim any trailing block space off the end of a used block, return to pool. */
void mheap::blk_trim_used(mheap_blk_t *blk, size_t size)
{
    mheap_blk_t *remaining_blk;

    if (blk_can_split(blk, size)) {
        /* If the next block is free, we must coalesce. */
        remaining_blk = blk_split(blk, size);
        blk_set_prev_used(remaining_blk);

        remaining_blk = blk_merge_next(remaining_blk);
        blk_insert(remaining_blk);
    }
}

mheap_blk_t* mheap::blk_trim_free_leading(mheap_blk_t *blk, size_t size)
{
    mheap_blk_t *remaining_blk;

    remaining_blk = blk;
    if (blk_can_split(blk, size)) {
        /* We want the 2nd block. */
        remaining_blk = blk_split(blk, size - MHEAP_BLK_HEADER_OVERHEAD);
        blk_set_prev_free(remaining_blk);

        blk_link_next(blk);
        blk_insert(blk);
    }

    return remaining_blk;
}

mheap_blk_t* mheap::blk_search_suitable(int *fli, int *sli)
{
    int fl, sl;
    uint32_t sl_map, fl_map;

    fl = *fli;
    sl = *sli;

	/*
	** First, search for a block in the list associated with the given
	** fl/sl index.
	*/
    sl_map = _sl_bitmap[fl] & (~0U << sl);
    if (!sl_map) {
        /* No block exists. Search in the next largest first-level list. */
        fl_map = _fl_bitmap & (~0U << (fl + 1));
        if (!fl_map) {
            /* No free blocks available, memory has been exhausted. */
            return 0;
        }

        fl = __ffs(fl_map);
        *fli = fl;
        sl_map = _sl_bitmap[fl];
    }
    sl = __ffs(sl_map);
    *sli = sl;

    /* Return the first block in the free list. */
    return _blocks[fl][sl];
}

mheap_blk_t* mheap::blk_locate_free(size_t size)
{
    int fl = 0, sl = 0;
    mheap_blk_t *blk = NULL;

    if (!size) {
        return NULL;
    }

    mapping_search(size, &fl, &sl);

    /*
    ** mapping_search can futz with the size, so for excessively large sizes it can sometimes wind up
    ** with indices that are off the end of the block array.
    ** So, we protect against that here, since this is the only callsite of mapping_search.
    ** Note that we don't need to check sl, since it comes from a modulo operation that guarantees it's always in range.
    */
again_search:
    if (fl < MHEAP_FL_INDEX_COUNT) {
        blk = blk_search_suitable(&fl, &sl);
    }

    if (blk) {
        remove_free_block(blk, fl, sl);
    } 
    else {
        /* jiejie: found that the memory is insufficient, need to expand the heap capacity and reallocate the memory */
        char * ptr = new char[MHEAP_DEFAULT_SIZE];
        DORA_ROBUSTNESS_CHECK(ptr, NULL);
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _mheap_ptr.push_back(ptr);
        }
        mheap_pool_add(ptr, MHEAP_DEFAULT_SIZE);
        goto again_search;
    }
    return blk;
}

void* mheap::blk_prepare_used(mheap_blk_t *blk, size_t size)
{
    if (!blk) {
        return NULL;
    }
    blk_trim_free(blk, size);
    blk_mark_as_used(blk);
    return blk_to_ptr(blk);
}

bool mheap::mheap_pool_is_exist(void *ps)
{
    int i = 0;

    for (i = 0; i < (int)_pool_start.size(); ++i) {
        if (_pool_start[i] == ps) {
            return true;
        }
    }
    return false;
}

void mheap::mheap_pool_record(void *ps)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _pool_start.push_back(ps);
}

void mheap::mheap_pool_unrecord(void *ps)
{
    int i = 0;
    std::unique_lock<std::mutex> lock(_mutex);

    for (i = 0; i < (int)_pool_start.size(); ++i) {
        if (_pool_start[i] == ps) {
            break;
        }
    }
    if (i != (int)_pool_start.size() - 1) {
        _pool_start[i] = _pool_start[(int)_pool_start.size() - 1];
        _pool_start.pop_back();
    } else {
        _pool_start.pop_back();
    }
}

void mheap::mheap_ctl_init(void)
{
    int i, j;
    std::unique_lock<std::mutex> lock(_mutex);

    _block_null.next_free = &_block_null;
    _block_null.prev_free = &_block_null;

    _fl_bitmap = 0;
    for (i = 0; i < MHEAP_FL_INDEX_COUNT; ++i) {
        _sl_bitmap[i] = 0;
        for (j = 0; j < MHEAP_SL_INDEX_COUNT; ++j) {
            _blocks[i][j] = &_block_null;
        }
    }
}

int mheap::mheap_init(void *pool_start, size_t pool_size)
{
    mheap_ctl_init();
    return mheap_pool_add(pool_start, pool_size);
}

void* mheap::mheap_alloc(size_t size)
{
    size_t          adjust_size;
    mheap_blk_t   *blk;

    adjust_size     = adjust_request_size(size, MHEAP_ALIGN_SIZE);
    blk             = blk_locate_free(adjust_size);
    if (!blk) {
        return NULL;
    }

    return blk_prepare_used(blk, adjust_size);
}

void* mheap::mheap_calloc(size_t num, size_t size)
{
    void *ptr;

    ptr = mheap_alloc(num * size);
    if (ptr) {
        memset(ptr, 0, num * size);
    }

    return ptr;
}

void* mheap::mheap_aligned_alloc(size_t size, size_t align)
{
    mheap_blk_t *blk;
    void *ptr, *aligned, *next_aligned;
    size_t adjust_size, aligned_size;
    size_t gap_minimum, size_with_gap, gap, gap_remain, offset;

    adjust_size     = adjust_request_size(size, MHEAP_ALIGN_SIZE);
    gap_minimum     = sizeof(mheap_blk_t);
    size_with_gap   = adjust_request_size(adjust_size + align + gap_minimum, align);
    aligned_size    = (adjust_size && align > MHEAP_ALIGN_SIZE) ? size_with_gap : adjust_size;

    blk = blk_locate_free(aligned_size);
    if (!blk) {
        return NULL;
    }

    ptr = blk_to_ptr(blk);
    aligned = align_ptr(ptr, align);
    gap = (size_t)((size_t)aligned - (size_t)ptr);

    if (gap && gap < gap_minimum) {
        gap_remain = gap_minimum - gap;
        offset = gap_remain > align ? gap_remain : align;
        next_aligned = (void *)((size_t)aligned + offset);

        aligned = align_ptr(next_aligned, align);
        gap = (size_t)((size_t)aligned - (size_t)ptr);
    }

    if (gap) {
        blk = blk_trim_free_leading(blk, gap);
    }

    return blk_prepare_used(blk, adjust_size);
}

void mheap::mheap_free(void *ptr)
{
    mheap_blk_t *blk;

    if (!ptr) {
        return;
    }

    blk = blk_from_ptr(ptr);
    blk_mark_as_free(blk);
    blk = blk_merge_prev(blk);
    blk = blk_merge_next(blk);
    blk_insert(blk);
}

void* mheap::mheap_realloc(void *ptr, size_t size)
{
    void *p = 0;
    mheap_blk_t *curr_blk, *next_blk;
    size_t curr_size, combined_size, adjust_size, min_size;

    if (ptr && size == 0) {
        mheap_free(ptr);
        return NULL;
    }

    if (!ptr) {
        return mheap_alloc(size);
    }

    curr_blk = blk_from_ptr(ptr);
    next_blk = blk_next(curr_blk);

    curr_size = blk_size(curr_blk);
    combined_size = curr_size + blk_size(next_blk) + MHEAP_BLK_HEADER_OVERHEAD;
    adjust_size = adjust_request_size(size, MHEAP_ALIGN_SIZE);

    if (adjust_size > curr_size && (!blk_is_free(next_blk) || adjust_size > combined_size)) {
        p = mheap_alloc(size);
        if (p) {
            min_size = curr_size < size ? curr_size : size;
            memcpy(p, ptr, min_size);
            mheap_free(ptr);
        }
    } else {
        if (adjust_size > curr_size) {
            blk_merge_next(curr_blk);
            blk_mark_as_used(curr_blk);
        }

        blk_trim_used(curr_blk, adjust_size);
        p = ptr;
    }

    return p;
}

int mheap::mheap_pool_add(void *pool_start, size_t pool_size)
{
    mheap_blk_t   *curr_blk;
    mheap_blk_t   *next_blk;
    size_t          size_aligned;

    if (mheap_pool_is_exist(pool_start)) {
        return DORA_VALUE_ALREADY_EXIST;
    }

    size_aligned = align_down(pool_size - 2 * MHEAP_BLK_HEADER_OVERHEAD, MHEAP_ALIGN_SIZE);

    if (((size_t)pool_start % MHEAP_ALIGN_SIZE) != 0u) {
        return DORA_VALUE_INVALID;
    }

    if (size_aligned < MHEAP_BLK_SIZE_MIN ||
        size_aligned > MHEAP_BLK_SIZE_MAX) {
        return DORA_VALUE_INVALID;
    }

    /*
     ** Create the main free block. Offset the start of the block slightly
     ** so that the prev_phys_block field falls outside of the pool -
     ** it will never be used.
     */
    curr_blk = offset_to_blk(pool_start, (int)(-MHEAP_BLK_HEADER_OVERHEAD));
    blk_set_size(curr_blk, size_aligned);
    blk_set_free(curr_blk);
    blk_set_prev_used(curr_blk);
    blk_insert(curr_blk);

    /* Split the block to create a zero-size sentinel block. */
    next_blk = blk_link_next(curr_blk);
    blk_set_size(next_blk, 0);
    blk_set_used(next_blk);
    blk_set_prev_free(next_blk);

    mheap_pool_record(pool_start);

    return DORA_SUCCESS;
}

int mheap::mheap_pool_del(void *pool_start)
{
    int fl = 0, sl = 0;
    mheap_blk_t *blk;

    DORA_ROBUSTNESS_CHECK(pool_start, DORA_VALUE_INVALID);

    if (!mheap_pool_is_exist(pool_start)) {
        return DORA_VALUE_ALREADY_NOT_EXIST;
    }

    blk = offset_to_blk(pool_start, (int)(-MHEAP_BLK_HEADER_OVERHEAD));
    mapping_insert(blk_size(blk), &fl, &sl);
    remove_free_block(blk, fl, sl);

    mheap_pool_unrecord(pool_start);
    return DORA_SUCCESS;
}

int mheap::mheap_pool_check(void *pool_start, mheap_info_t *info)
{
    mheap_blk_t* blk;

    DORA_ROBUSTNESS_CHECK(pool_start, DORA_VALUE_INVALID);
    DORA_ROBUSTNESS_CHECK(info, DORA_VALUE_INVALID);

    memset(info, 0, sizeof(mheap_info_t));

    blk = offset_to_blk(pool_start, (int)(-MHEAP_BLK_HEADER_OVERHEAD));

    while (blk && !blk_is_last(blk)) {
        if (blk_is_free(blk)) {
            info->free += blk_size(blk);
        } else {
            info->used += blk_size(blk);
        }
        blk = blk_next(blk);
    }

    return DORA_SUCCESS;
}

int mheap::mheap_check(mheap_info_t *info)
{
    int i;
    int err;
    mheap_info_t pool_info;

    DORA_ROBUSTNESS_CHECK(info, DORA_VALUE_INVALID);

    memset(info, 0, sizeof(mheap_info_t));

    for (i = 0; i < (int)_pool_start.size(); ++i) {
        err = mheap_pool_check(_pool_start[i], &pool_info);
        if (err != DORA_SUCCESS) {
            return err;
        }

        info->free += pool_info.free;
        info->used += pool_info.used;
    }
    
    return DORA_SUCCESS;
}

void mheap::mheap_print_info(void)
{
    mheap_info_t info;
    mheap_check(&info);

    DORA_LOG_INFO("pool count : {}, used space : {}, free space : {}", 
                (int)_pool_start.size(), info.used, info.free);
}

}
