/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-11-04 17:55:25
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-04 18:59:21
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#ifndef _DORA_MPOOL_H_
#define _DORA_MPOOL_H_

#include "stddef.h"
#include "stdint.h"
#include <mutex>
#include <vector>

namespace doralib {

#define     MPOOL_STATUS_UNUSED         0x00
#define     MPOOL_STATUS_USED           0x01

typedef struct mpool_node {
    uint32_t    status;
    struct mpool_node *next;
} mpool_node_t;

class mpool {
private:
    /* mpool_base address */
    char                *_mpool_base;
    /* memory pool size */
    uint32_t            _mpool_size;
    /* count of total memory block */
    uint32_t            _total_cnt;
    /* count of free memory block */
    uint32_t            _free_cnt;
    /* first free memory block of memory pool. memory block form a linked list. */
    mpool_node_t        _free;
    /* mutex */
    std::mutex          _mutex;

public:
    mpool();
    mpool(uint32_t blk_size, uint32_t cnt);
    ~mpool();

    int mpool_create(uint32_t blk_size, uint32_t cnt);
    void* mpool_alloc(void);
    void mpool_free(void *ptr);
};

}

#endif 
