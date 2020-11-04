/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-04 17:55:03
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-04 19:04:14
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include "dora_mpool.h"
#include "dora_log.h"
#include "dora_error.h"

namespace doralib {

using doralib::mpool;

mpool::mpool()
    : _mpool_base(NULL) {}

mpool::mpool(uint32_t blk_size, uint32_t cnt)
    : _mpool_base(NULL)
{
    mpool_create(blk_size, cnt);
}

mpool::~mpool()
{
    if (NULL != _mpool_base)
        delete _mpool_base;
}


/**
 * create a memory pool
 * @param   mpool_size:  memory pool size
 * @param   blk_size:   memory block size
 */
int mpool::mpool_create(uint32_t blk_size, uint32_t cnt)
{
    mpool_node_t *node;
    mpool_node_t *next_node;

    if ((blk_size == 0) || (cnt == 0)) {
        DORA_LOG_ERROR("the parameter is incorrect and cannot continue to create a memory pool");
        return DORA_VALUE_INVALID;
    }

    std::unique_lock<std::mutex> lock(_mutex);

    _mpool_size = ((sizeof(mpool_node_t) + blk_size) * cnt);
    _mpool_base = new char[_mpool_size];
    _total_cnt = cnt;
    _free_cnt = cnt;

    DORA_ROBUSTNESS_CHECK(_mpool_base, DORA_MEM_NOT_ENOUGH);

    node = (mpool_node_t *)_mpool_base;
    _free.next = node;
    for (uint32_t i = 1; i < _total_cnt; i++) {
        next_node = (mpool_node_t *)((size_t)node + sizeof(mpool_node_t) + blk_size);
        node->next = next_node;
        node = next_node;
    }
    node->next = NULL;
    return DORA_SUCCESS;
}


void* mpool::mpool_alloc()
{
    mpool_node_t *node;

    if ((_free_cnt <= 0) || (_free.next == NULL)) {
        DORA_LOG_WARN("the number of free memory blocks in the memory pool is 0");
        return NULL;
    }

    std::unique_lock<std::mutex> lock(_mutex);

    _free_cnt--;

    node = _free.next;
    node->status = MPOOL_STATUS_USED;

    _free.next = node->next;

    return (void*)((size_t)node + sizeof(mpool_node_t));
}

void mpool::mpool_free(void *ptr)
{
    mpool_node_t *node;
    mpool_node_t *next_node;

    node = (mpool_node_t *)((size_t)ptr - sizeof(mpool_node_t));

    if ((node->status == MPOOL_STATUS_UNUSED) || (_free_cnt >= _total_cnt)) {
        DORA_LOG_WARN("the memory block to be free is not in the correct format");
        return;
    }

    std::unique_lock<std::mutex> lock(_mutex);

    next_node = _free.next;
    _free.next = node;
    node->next = next_node;

    _free_cnt++;
    node->status = MPOOL_STATUS_UNUSED;
}

}




