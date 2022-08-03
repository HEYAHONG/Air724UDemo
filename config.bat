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

@rem 设置AM_MODEL,选择不同的核心功能(见csdk\core目录） 
set AM_MODEL=iot_SDK_720U_BT_TTS_VOLTE


@rem 运行csdk的启动脚本
call %PROJECT_PATH%\csdk\tools\core_launch.bat  %BUILD_TARGET%

@rem 进入工程目录
cd %PROJECT_PATH%

@rem 处理Kconfig

python3 -m genconfig --config-out .config

@rem 设定Kconfig中的变量
FOR /F "usebackq delims=#" %%i IN (`FINDSTR /V /C:# %PROJECT_PATH%\.config`) do set %%i

if not  %CONFIG_CSDK_VER% == "" set CSDK_VER=%CONFIG_CSDK_VER:"=%
if not  %CONFIG_CSDK_PRO% == "" set CSDK_PRO=%CONFIG_CSDK_PRO:"=%
if not  %CONFIG_FOTA_FLAG% == "" set FOTA_FLAG=%CONFIG_FOTA_FLAG:"=%
if not  %CONFIG_FOTA_TYPE% == "" set CSDK_PRO=%CONFIG_FOTA_TYPE:"=%

if "%CONFIG_RELEASE_TYPE_DEBUG%" == "y" set  BUILD_RELEASE_TYPE=debug
if "%CONFIG_RELEASE_TYPE_RELEASE%" == "y" set  BUILD_RELEASE_TYPE=release

if "%CONFIG_CORE_BASE%" == "y" set  AM_MODEL=iot_SDK_720U
if "%CONFIG_CORE_BT_TTS%" == "y" set  AM_MODEL=iot_SDK_720U_BT_TTS
if "%CONFIG_CORE_BT_TTS_VOLTE%" == "y" set  AM_MODEL=iot_SDK_720U_BT_TTS_VOLTE


@rem 处理CMAKE定义
SET CMAKEDEF=
FOR /F "usebackq delims=#" %%i IN (`FINDSTR /V /C:# %PROJECT_PATH%\.config`) do call  :CMakeAddDef  -D%%i

goto :eof

:CMakeAddDef
set STR=%1=%2 %3 %4 %5 %6 %7 %8 %9
echo CMake Config:  %STR%
set CMAKEDEF= %STR%   %CMAKEDEF%
goto :eof