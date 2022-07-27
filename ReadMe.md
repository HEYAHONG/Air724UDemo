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
- buildenv.bat：构建工程文件并打开一个cmd.exe窗口（编译环境，此环境中可使用sdk中的工具）。
- menuconfig.bat:配置Kconfig。

## Code::Blocks工程文件使用说明

运行buildenv.bat后，工程文件在build目录下,后缀名为cbp。若出现失败提示可能没有此文件,此时源代码可能有误。

若直接使用codeblocks打开时,只能编辑源代码，而不能执行编译命令测试源代码（有些命令未使用绝对路径，需要在编译环境中执行）。若要正常使用编译命令,请安以下步骤操作:

- 安装好Code::Blocks。注意:需要能够正常编译的Code::Blocks，即需要带Mingw工具链,如不确定，可建一个helloworld工程测试，如能成功编译则正常。
- 执行buildenv.bat。执行完成后,会有一个cmd.exe的窗口。
- 将Code::Blocks的快捷方式或者codeblocks.exe本身拖入cmd.exe的窗口,按回车执行。
- 打开Code::Blocks后,可关闭cmd.exe窗口,此时使用Code::Blocks打开build目录下的cbp文件即可正常编译源代码。

# 软件说明

## 调试输出

采用UART2输出调试信息，通过串口调试查看。波特率:921600 8N1。





