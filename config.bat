@echo off

@rem 工程目录
set PROJECT_PATH=%~dp0


@rem  版本号
set CSDK_VER=1.0.0

@rem  工程名称
set CSDK_PRO=Air724UDemo
set BUILD_TARGET=%PROJECT_PATH%
set BUILD_RELEASE_TYPE=debug

set FOTA_FLAG=%1
set FOTA_TYPE=%2

@rem 设置AM_MODEL,选择不同的核心功能(见%CSDK_INSTALL_PATH%\core目录） 
set AM_MODEL=iot_SDK_720U_BT_TTS_VOLTE


@rem 运行csdk的启动脚本
call %PROJECT_PATH%\csdk\tools\core_launch.bat  %BUILD_TARGET%

echo BUILD_TARGET:%BUILD_TAGET%
