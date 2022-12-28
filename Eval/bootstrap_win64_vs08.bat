@echo off

setlocal()

set _msvs=Visual Studio 2008
set _os=x64
set _cmake_string=Visual Studio 9 2008 Win64
set _project_name=Eval
set _cmake_switch=-D %_project_name%_STANDALONE=ON

title Creating %_msvs% %_os% project for %_project_name%

:main
cls

if not exist bin (
	mkdir bin
)
if not exist lib (
	mkdir lib
)

if not exist build/%COMPUTERNAME%%_os%_WAS_HERE (
	if exist build (
		echo Deleting ./build, because it was created on another System.
		rmdir /S /Q build
	)
	mkdir build
	echo Created by %0 > build/%COMPUTERNAME%%_os%_WAS_HERE
) else (
	echo Keeping ./build since it was created on this machine.
)
cd build
if exist ../../%_project_name% (
	goto :cmake 
) else (
	goto :report_error
)

:cmake
echo Creating %_msvs% %_os% project in 'build'
@echo on
cmake %_cmake_switch% -G "%_cmake_string%" ..
@echo off
if %ERRORLEVEL% neq 0 (
	goto :report_error
) else (
	goto :open_project
)

:report_error
cd..
echo.
echo Something went wrong, check output and retry.
echo.
choice /M "Retry?"
if %ERRORLEVEL% EQU 1 (
	goto :main
) else (
	goto :eof
)




:open_project
start %_project_name%.sln
cd ..
goto :eof

:eof
endlocal()
exit
