@echo off

@rem 工程目录
set PROJECT_PATH=%~dp0

@rem CSDK安装目录(默认需提前下载至%userprofile%\OpenLuat),目录必须可写
set CSDK_INSTALL_PATH=%userprofile%\OpenLuat\Luat_CSDK_Air724U

@rem  版本号
set CSDK_VER=1.0.0

@rem  工程名称(不要与CSDK中demo的名称相同)
set CSDK_PRO=Air724UDemo
set BUILD_TARGET=%CSDK_PRO%
set BUILD_RELEASE_TYPE=debug

set FOTA_FLAG=%1
set FOTA_TYPE=%2

@rem 设置AM_MODEL,选择不同的核心功能(见%CSDK_INSTALL_PATH%\core目录） 
set AM_MODEL=iot_SDK_720U

@rem 创建工程链接至CSDK
rmdir "%CSDK_INSTALL_PATH%\demo\%CSDK_PRO%" 1>nul 2>nul
mklink /d /j "%CSDK_INSTALL_PATH%\demo\%CSDK_PRO%"  "%PROJECT_PATH%" 1>nul 2>nul

@rem 添加CSDK中的工具
mklink /d /j   "%PROJECT_PATH%\csdk" "%CSDK_INSTALL_PATH%"  1>nul 2>nul
set PATH=%PROJECT_PATH%\csdk\prebuilts\win32\bin;%PATH%
set PATH=%PROJECT_PATH%\csdk\prebuilts\win32\cmake\bin;%PATH%
set PATH=%PROJECT_PATH%\csdk\prebuilts\win32\python3;%PATH%
set PATH=%PROJECT_PATH%\csdk\prebuilts\win32\gcc-arm-none-eabi\bin;%PATH%
set PATH=%PROJECT_PATH%\csdk\tools;%PATH%
set PATH=%PROJECT_PATH%\csdk\tools\win32;%PATH%


@rem 创建工程目录
MD %PROJECT_PATH%\build 1>nul 2>nul
cd   %PROJECT_PATH%\build


@rem 生成工程（可通过codeblcoks编辑）并编译
cmake "%PROJECT_PATH%\csdk"  -G "CodeBlocks - Ninja" & ninja 


@rem 回到工程目录
cd %PROJECT_PATH%
