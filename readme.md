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

使用编译脚本的 **`-i`** 参数进行安装，**`-i`** 参数后支持指定安装目录，默认安装目录是 **`/usr/local`**, 请放心，**doralib** 不会污染您系统本身的库，运行该命令需要 **`sudo`** 权限。

```bash
    sudo ./build.sh -i
```

指定安装目录`/home/jie`

```bash
    sudo ./build.sh -i/home/jie
```

### 编译example

使用编译脚本的 **`-e`** 参数进行编译 **example** 代码。

```bash
    ./build.sh -e
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

通用的依赖组件，如异常、日志（使用了spdlog）。

## thpool

线程池库，实现了线程池与工作队列，根据CPU核心数创建对应的线程池，向线程池中提交的函数支持参数可变与返回值。

## net
    
C++ socket 网络库（使用了epoll）。

  - dgram：UDP协议传输。

