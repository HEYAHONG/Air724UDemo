# 简介

本工程是一个使用[Luat_CSDK_Air724U（非官方修改版）](https://github.com/HEYAHONG/Luat_CSDK_Air724U.git)进行开发的Demo程序。

## 源代码下载

由于本源代码包含第三方源代码,故直接下载可能有部分源代码缺失，需要通过以下方法解决:

- 在进行git clone 使用--recurse-submodules参数。

- 若已通过git clone下载,则在源代码目录中执行以下命令下载子模块:

  ```bash
   git submodule update --init --recursive
  ```

# 编译环境

- 操作系统: Windows  (sdk虽有部分linux系统支持代码，但支持不全，此时无法使用（20210804))
- SDK：Luat_CSDK_Air724U 。

## 脚本说明

- build.bat:构建工程文件(condeblocks工程文件在build目录中)并编译。
- menuconfig.bat:配置Kconfig。

# 软件说明

## 调试输出

采用UART2输出调试信息，通过串口调试查看。波特率:921600 8N1。





