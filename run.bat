:: run.bat
@echo off

:: 1. PUSHD：保存用户当前的目录，并自动切换到本批处理文件所在的目录。
:: %~dp0 代表批处理文件所在的路径。
pushd "%~dp0"

:: 2. 执行你的隐藏命令
:: PowerShell 脚本会运行同目录下的 EXE 并隐藏窗口
PowerShell -WindowStyle Hidden -File ".\run.ps1"

:: 3. POPD：将目录恢复到 PUSHD 之前用户的原始目录。
popd
