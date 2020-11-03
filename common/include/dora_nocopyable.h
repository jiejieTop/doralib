/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-25 17:00:10
 * @LastEditors: jiejie
 * @LastEditTime: 2020-10-25 17:00:47
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#ifndef _DORA_NONCOPYABLE_H_
#define _DORA_NONCOPYABLE_H_

namespace doralib
{

class noncopyable
{
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

}  // namespace doralib

#endif  // _DORA_NONCOPYABLE_H_
