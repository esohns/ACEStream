@rem #//%%%FILE%%%//////////////////////////////////////////////////////////////
@rem #// File Name: make_oo_headers.bat
@rem #//
@rem #// arguments:
@rem #// History:
@rem #//   Date   |Name | Description of modification
@rem #// ---------|-----|-------------------------------------------------------
@rem #// 11/07/16 | soh | Creation.
@rem #//%%%FILE%%%//////////////////////////////////////////////////////////////

@echo off
set RC=0
setlocal enabledelayedexpansion
pushd . >NUL 2>&1

@rem elevate to admin
net session>nul 2>&1
if %errorlevel%==0 goto main
echo CreateObject("Shell.Application").ShellExecute "%~f0", "", "", "runas">"%temp%/elevate.vbs"
"%temp%/elevate.vbs"
del "%temp%/elevate.vbs"
@rem exit

:main
goto Begin

:Print_Usage
@rem echo usage: %~n0 ^[Debug ^| Release^]
echo usage: %~n0
goto Clean_Up

:Begin
@rem if "%1."=="." (
@rem  echo invalid argument^, exiting
@rem  goto Print_Usage
@rem )
@rem if NOT "%1"=="Debug" if NOT "%1"=="Release" (
@rem  echo invalid argument ^(was: "%1"^)^, exiting
@rem  goto Print_Usage
@rem )
if NOT exist "%OO_SDK_HOME%" (
 echo invalid directory OO_SDK_HOME ^(was: "%OO_SDK_HOME%"^)^, exiting
 goto Failed
)
if NOT exist "%UNO_PATH%" (
 echo invalid directory UNO_PATH ^(was: "%UNO_PATH%"^)^, continuing
 set UNO_PATH=%OO_SDK_HOME%\..\program
@rem goto Failed
)

@rem step1: copy sal3.dll, salhelper3msc.dll, unoiddlo.dll, uwinapi.dll and
@rem        reglo.dll storelo.dll to the path of cppumaker.exe (dependencies)
for %%A in (sal3.dll unoidllo.dll salhelper3MSC.dll reglo.dll storelo.dll) do (
@rem for %%A in (sal3.dll unoidllo.dll uwinapi.dll salhelper3MSC.dll reglo.dll storelo.dll) do (
@rem for %%A in (sal3.dll unoidllo.dll uwinapi.dll salhelper3MSC.dll reglo.dll storelo.dll) do (
 if NOT exist "%UNO_PATH%\%%A" (
  echo invalid .dll file ^(was: %%A^)^, exiting
  goto Failed
 )
 copy /Y "%UNO_PATH%\%%A" "%OO_SDK_HOME%\bin" >NUL
@rem if %ERRORLEVEL% NEQ 0 (
@rem  echo failed to copy dll "%UNO_PATH%\%%A" to "%OO_SDK_HOME%\bin"^, exiting
@rem  set RC=%ERRORLEVEL%
@rem  goto Failed
@rem )
 echo copied %%A...DONE
)

@rem step2: generate c++ headers
set CPPUMAKEREXE=%OO_SDK_HOME%\bin\cppumaker.exe
if NOT exist "%CPPUMAKEREXE%" (
 echo invalid file ^(was: "%CPPUMAKEREXE%"^)^, exiting
 goto Failed
)
set TYPES_RDB=%UNO_PATH%\types.rdb
if NOT exist "%TYPES_RDB%" (
 echo invalid .rdb file ^(was: "%TYPES_RDB%"^)^, exiting
 goto Failed
)
set SERVICES_RDB=%UNO_PATH%\services.rdb
if NOT exist "%SERVICES_RDB%" (
 echo invalid .rdb file ^(was: "%SERVICES_RDB%"^)^, exiting
 goto Failed
)
set OOAPI_RDB=%UNO_PATH%\types\offapi.rdb
if NOT exist "%OOAPI_RDB%" (
 echo invalid .rdb file ^(was: "%OOAPI_RDB%"^)^, exiting
 goto Failed
)
set OOVBAAPI_RDB=%UNO_PATH%\types\oovbaapi.rdb
if NOT exist "%OOVBAAPI_RDB%" (
 echo invalid .rdb file ^(was: "%OOVBAAPI_RDB%"^)^, exiting
 goto Failed
)

echo generating headers...
@rem %CPPUMAKEREXE% -BUCR -C -Gc -O "%OO_SDK_HOME%\include" "%TYPES_RDB%" "%OOAPI_RDB%"
"%CPPUMAKEREXE%" -C -Gc -O "%OO_SDK_HOME%\include" "%TYPES_RDB%" "%OOAPI_RDB%" "%OOVBAAPI_RDB%"
if %ERRORLEVEL% NEQ 0 (
 echo failed to generate headers^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)
echo generating headers...DONE
goto Clean_Up

:Failed
echo generating headers...FAILED

:Clean_Up
popd
::endlocal & set RC=%ERRORLEVEL%
endlocal & set RC=%RC%
goto Error_Level

:Exit_Code
:: echo %ERRORLEVEL% %1 *WORKAROUND*
exit /b %1

:Error_Level
call :Exit_Code %RC%

