# 简介

本工程是一个使用[Luat_CSDK_Air724U](https://github.com/openLuat/Luat_CSDK_Air724U.git)进行开发的Demo程序。
由于官方此时(20210804)的支持尚不完善，此Demo中的某些分支并不是使用官方源代码,而是进行了某些修改,地址为[https://github.com/HEYAHONG/Luat_CSDK_Air724U.git](https://github.com/HEYAHONG/Luat_CSDK_Air724U.git)。
若使用的demo的分支名称在官方源代码中不存在，请使用修改后的SDK代码。

# 编译环境

- 操作系统: Windows  (sdk虽有部分linux系统支持代码，但支持不全，此时无法使用（20210804))
- SDK：Luat_CSDK_Air724U (内置在windows下的编译工具链,一般无需安装其它工具)，安装在%userprofile%\OpenLuat\Luat_CSDK_Air724U目录下（不在此目录下需要修改编译脚本）。

## Windows下SDK安装

- 安装[git for windows](https://gitforwindows.org/)/其它git工具
- 打开cmd.exe,执行以下命令,进入%userprofile%\OpenLuat目录:

```cmd
mkdir  %userprofile%\OpenLuat 1>nul 2>nul
cd /d %userprofile%\OpenLuat

```

- 使用git在刚才的目录下clone,使用-b指定分支。下载完成就表示安装完成。

```cmd
git clone -b HYH https://github.com/HEYAHONG/Luat_CSDK_Air724U.git
```

- 下载完成后,在demo目录中可使用build.bat构建demo程序,构建完成后,在demo工程目录/build/hex中就是最终程序。





