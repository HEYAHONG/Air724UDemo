@echo off

@rem 调用配置脚本
call "%~dp0\config.bat"  %1 %2


@rem 创建工程目录
MD "%PROJECT_PATH%\build" 1>nul 2>nul
cd   "%PROJECT_PATH%\build"


@rem 生成工程（可通过codeblcoks编辑）并编译
cmake %CMAKEDEF% "%PROJECT_PATH%\csdk"  -G "CodeBlocks - Ninja" & ninja 


@rem 回到工程目录
cd "%PROJECT_PATH%"