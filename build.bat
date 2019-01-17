@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
rmdir /S /Q build
del %~dp0\xmrig-amd-%1-win64.zip
mkdir build &&^
cd build &&^
git clone https://github.com/MoneroOcean/xmrig-amd.git &&^
git clone https://github.com/xmrig/xmrig-deps.git &&^
mkdir xmrig-amd\build &&^
cd xmrig-amd\build &&^
git checkout %1 &&^
cmake .. -G "Visual Studio 15 2017 Win64" -DXMRIG_DEPS=%~dp0\build\xmrig-deps\msvc2017\x64 &&^
msbuild /p:Configuration=Release xmrig-amd.sln &&^
cd Release &&^
copy ..\..\src\config.json . &&^
7z.exe a -tzip -mx %~dp0\xmrig-amd-%1-win64.zip xmrig-amd.exe config.json &&^
cd %~dp0 &&^
rmdir /S /Q build
