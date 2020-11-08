![C/C++ CI](https://github.com/jiejieTop/doralib/workflows/C/C++%20CI/badge.svg)
[![issues](https://img.shields.io/github/issues/jiejieTop/doralib)](https://github.com/jiejieTop/doralib/issues)
![forks](https://img.shields.io/github/forks/jiejieTop/doralib)
![stars](https://img.shields.io/github/stars/jiejieTop/doralib)
[![license](https://img.shields.io/github/license/jiejieTop/doralib)](https://github.com/jiejieTop/doralib/blob/master/LICENSE)
![](https://img.shields.io/badge/platform-Linux|Windows|Mac|Embedded-orange.svg)

# doralib

一个简单易用的 C++ 库。

## 快速使用

### 依赖
- camek 2.8 及以上
- C++ 11
### 编译

```bash
    git clone https://github.com/jiejieTop/doralib.git
    cd doralib
    ./build.sh
```

### 安装

使用编译脚本的 **`-i`** 参数进行安装，**`-i`** 参数后支持指定安装目录，默认安装目录是 **`/usr/local`**, 请放心，**doralib** 不会污染您系统本身的库，运行该命令需要 **`sudo`** 权限（它会提示您输入密码）。

```bash
    ./build.sh -i
```

指定安装目录`/home/jie`

```bash
    ./build.sh -i/home/jie
```

### 编译example

使用编译脚本的 **`-e`** 参数进行编译 **example** 代码。

```bash
    ./build.sh -e
```

### 运行example

所有的 **example** 代码文件在编译后会产生对应的可执行文件，位于 **`build/bin/`** 目录下，执行该文件即可。

也可以运行 **`run_example.sh`** 测试脚本。
```bash
    ./run_example.sh
```

### 指定编译器

使用编译脚本的 **`-c`** 参数指定C++编译器进行编译，**`-c`** 参数支持指定编译器、编译器路径等。

  - 指定编译器
  ```bash
      ./build.sh -cg++
  ```

  - 指定路径下的编译器
  ```bash
      ./build.sh -c/usr/bin/g++
  ```

## common

通用的依赖组件。

  - mheap ： 内存堆管理实现，使用 **tlsf** 内存管理算法，时间复杂度 **`O(1)`**，支持自动增长管理的内存块，无内存泄漏与内存碎片风险，支持多线程。
  - mpool ： 内存池管理实现，支持多线程，时间复杂度为 **`O(1)`**，无内存泄漏与内存碎片风险。
  - timer ： 定时器实现，使用一个线程调度定时事件，线程采用阻塞的方式等待超时，在插入定时器或者删除定时器的时候使用**POSIX信号量**作为通知，实现了最小堆，查找超时的定时器时间复杂度为 **`O(1)`**。
  - utils ： 通用的小工具实现，如字符串哈希等。
  - error ： 错误代码管理。
  - exception ： 异常管理。
  - spdlog ： 开源高效的日志管理库。

## thpool

线程池库，实现了线程池与工作队列，根据CPU核心数创建对应的线程池，向线程池中提交的函数支持参数可变与返回值。

## net
    
C++ socket 网络库（使用了epoll）。

  - socket ： 封装了socket的所有功能。
  - dgram ： UDP协议传输管理。
  - epoller ：  epoll管理器，高效管理并非socket。

