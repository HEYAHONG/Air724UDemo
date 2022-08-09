# 简介

本工程是一个使用[Luat_CSDK_Air724U（非官方修改版）](https://github.com/HEYAHONG/Luat_CSDK_Air724U.git)进行开发的Demo程序。

## 资源文件

类似于桌面程序的资源文件。源代码实现的目录为 [rc](rc/)。

在固件编写中，很多时候需要大量的固定数据，直接手工嵌入到C文件里比较麻烦。

通过读取文件转换到对应C文件可大大节省时间，可添加常用的文件（如各种证书）或者不适宜放在可读写的文件系统中的文件(如需要在格式化中保留或者初始参数)。转换程序源代码为[rc/fsgen.cpp](rc/fsgen.cpp)。

使用步骤如下:

- 将待添加的文件放入 rc/fs目录下。

- 使用文件名调用以下函数(需包含相应头文件RC.h):

  ```c++
  //通过名称获取资源大小
  size_t RCGetSize(const char * name);
  
  //通过名称获取资源指针
  const unsigned char * RCGetHandle(const char * name);
  ```

## SFFS

SFFS是由CSDK支持的预先放置文件的方法。添加文件的目录为[sffs/sffs](sffs/sffs),即目录下的文件将添加至文件系统根目录中,通过iot_fs.h中的API访问。与资源文件不同的是，文件系统中文件可读写，也可存放较大的文件（受APP代码空间限制，资源文件不可存放较大的文件）。



## 已知问题

- C++静态类的构造函数不可用。
- C++全局类的构造函数不可用。

## 源代码下载

由于本源代码包含第三方源代码,故直接下载可能有部分源代码缺失，需要通过以下方法解决:

- 在进行git clone 使用--recurse-submodules参数。

- 若已通过git clone下载,则在源代码目录中执行以下命令下载子模块:

  ```bash
   git submodule update --init --recursive
  ```

# 编译

- 操作系统: Windows  10
- SDK：Luat_CSDK_Air724U 。

## 脚本说明

- build.bat:构建工程文件(condeblocks工程文件在build目录中)并编译。
- buildenv.bat：构建工程文件并打开一个cmd.exe窗口（编译环境，此环境中可使用sdk中的工具）。
- menuconfig.bat:配置Kconfig。

## Code::Blocks工程文件使用说明

运行buildenv.bat后，工程文件在build目录下,后缀名为cbp。若出现失败提示可能没有此文件,此时源代码可能有误。

若直接使用codeblocks打开时,只能编辑源代码，而不能执行编译命令测试源代码（有些命令未使用绝对路径，需要在编译环境中执行）。

若要正常使用编译命令,请按以下步骤操作:

- 安装好Code::Blocks。注意:需要能够正常编译的Code::Blocks，即需要带Mingw工具链,如不确定，可建一个helloworld工程测试，如能成功编译则正常。
- 执行buildenv.bat。执行完成后,会有一个cmd.exe的窗口。
- 将Code::Blocks的快捷方式或者codeblocks.exe本身拖入cmd.exe的窗口,按回车执行。
- 打开Code::Blocks后,可关闭cmd.exe窗口,此时使用Code::Blocks打开build目录下的cbp文件即可正常编译源代码。

## 固件烧录

正常编译完成后,最终的固件在 build/hex 目录下,后缀名为pac。

此时可直接使用合宙官方的Luatools的固件下载烧录，烧录方式有以下两种:

- 免USB BOOT烧录。使用USB连接PC后直接下载。注意:这需要原有固件正常时才能使用,如有死机或者不断重启时可能不可用。
- USB BOOT烧录。使用USB连接PC，拉高USB BOOT引脚后（正常运行时需取消拉高操作）进行一次复位,然后直接下载。通常开发过程中需使用这种方式。

# 软件说明

## 调试输出

采用UART2输出调试信息，通过串口调试查看。波特率:921600 8N1。





