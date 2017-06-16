# Tinix
===============================

forked from http://code.taobao.org/p/tinixdev/src/

**一个小型的操作系统。**

作者博客： http://bbs.pediy.com/thread-182967.htm，我只是搬运工。
本工程只能在linux下编译，如果要在windows下编译，请下载作者原代码。

## 目录
```text
|-- floppy.img                                        #最终完成的软盘镜像
|-- tinix                                             #内核目录
|   |-- boot                                          #内核加载器目录
|   |-- kernel                                        #内核代码目录
|   |   |-- dev                                       #设备驱动目录
|   |   |-- fs                                        #文件系统目录
|   |   |-- include                                   #头文件
|   |   |-- init                                      #内核代码入口
|   |   |-- kernel                                    #任务管理、陷阱门、时钟、系统调用等
|   |   |-- lib                                       #内核lib
|   |   |-- mm                                        #内存管理目录
|   |   `-- Makefile.linux                            #用于linux下编译的makefile
|   |-- compile.sh                                    #编译并运行
|   |-- build.sh                                      #编译生成
|   |-- clean.sh                                      #清理
|   |-- run.sh                                        #用qemu运行操作系统
|   |-- loader.map                                    #内核加载器符号文件
|   |-- system.map                                    #内核符号文件
|   |-- system.idc                                    #用于IDA的内核符号脚本
|   |-- map2idc.pl                                    #生成IDA脚本的perl脚本
|   `-- oskernel                                      #最终编译出的内核镜像
`-- user                                              #用户态代码目录
    |-- app                                           #用户态应用源代码
    |-- include                                       #用户态头文件
    |-- lib                                           #用户态lib函数源代码
    |-- Makefile.eval                                 #用户态应用的几个makefile
    |-- Makefile.pi
    |-- Makefile.queens
    |-- Makefile.sh
`-- objs                                              #避免重新编译的obj文件
```


## 构建工具

```bash
sudo apt install make gcc binutils cgdb qemu
sudo ln -s /usr/bin/qemu-system-i386 /usr/bin/qemu
```

## 编译并运行
```shell
./build.sh
./run.sh
./clean.sh
```
