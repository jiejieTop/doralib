/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-27 12:02:45
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-01 14:04:14
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#ifndef _DORA_SINGLETON_H_
#define _DORA_SINGLETON_H_

#include <mutex>

namespace doralib {

/// This is a helper template class for managing
/// singleton objects allocated on the heap.
template <typename T>
class singleton {
private:
    static T* _ptr;
    static std::mutex _m;   
public:

    /// Destroys the singleton wrapper and the managed
    /// singleton instance it holds.
    ~singleton() {
        if (_ptr)
            delete _ptr;
    }

    /// Returns a pointer to the singleton object hold by the singleton.
    /// The first call to get on a nullptr singleton will instantiate
    /// the singleton.
    static T* get() {
        if (!_ptr) {
            std::lock_guard<std::mutex> guard(_m);
            printf("++++++++++++++++++++\n");
            _ptr = new T;
        }
        printf("++++++++++++++++++++1\n");
        return _ptr;
    }

    /// Swaps the old pointer with the new one and returns the old instance.
    T* swap(T* newPtr) {
        std::lock_guard<std::mutex> guard(_m);
        T* oldPtr = _ptr;
        _ptr = newPtr;
        return oldPtr;
    }

    /// Destroys the managed singleton instance.
    void destroy() {
        printf("------------------\n");
        std::lock_guard<std::mutex> guard(_m);
        if (_ptr)
            delete _ptr;
        _ptr = nullptr;
    }
};

} // namespace doralib


#endif // 
