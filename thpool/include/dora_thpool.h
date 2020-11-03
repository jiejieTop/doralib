/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-26 09:54:52
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-02 18:49:02
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#ifndef _DORA_THPOOL_H_
#define _DORA_THPOOL_H_

#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include "dora_queue.h"
#include "dora_log.h"

namespace doralib {

class thpool {
private:
    class worker {
    private:
        int _id;
        thpool * _pool;
    public:
        worker(thpool * pool, const int id)
            :  _id(id), _pool(pool) {
        }

        void operator()() {
            std::function<void()> func;
            bool dequeued;
            while (!_pool->_shutdown) {
                {
                    std::unique_lock<std::mutex> lock(_pool->_mutex);
                    if (_pool->_queue.empty()) {
                        _pool->_lock.wait(lock);
                    }
                    dequeued = _pool->_queue.dequeue(func);
                }
                if (dequeued) {
                    func();
                }
            }
        }
    };

    bool _shutdown;
    doralib::queue<std::function<void()>> _queue;
    std::vector<std::thread> _threads;
    std::mutex _mutex;
    std::condition_variable _lock;

    void init();
    void deinit();

public:
    thpool();
    thpool(int n);
    ~thpool();
    
    void shutdown(void);

    thpool(const thpool &) = delete;
    thpool(thpool &&) = delete;

    thpool & operator=(const thpool &) = delete;
    thpool & operator=(thpool &&) = delete;

    // Submit a function to be executed asynchronously by the pool
    template<typename F, typename...Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // Create a function with bounded parameters ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // Encapsulate it into a shared ptr in order to be able to copy construct / assign 
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Wrap packaged task into void function
        std::function<void()> wrapper_func = [task_ptr]() {
            (*task_ptr)(); 
        };

        // Enqueue generic wrapper function
        _queue.enqueue(wrapper_func);

        // Wake up one thread if its waiting
        _lock.notify_one();

        // Return future from promise
        return task_ptr->get_future();
    }
};

}

#endif
