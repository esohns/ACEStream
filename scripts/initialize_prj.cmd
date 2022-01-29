@rem #//%%%FILE%%%//////////////////////////////////////////////////////////////
@rem #// File Name: initialize_prj.cmd
@rem #//
@rem #// arguments:
@rem #// History:
@rem #//   Date   |Name | Description of modification
@rem #// ---------|-----|-------------------------------------------------------
@rem #// 12/07/16 | soh | Creation.
@rem #//%%%FILE%%%//////////////////////////////////////////////////////////////

@echo off
set RC=0
setlocal enabledelayedexpansion
pushd . >NUL 2>&1
goto Begin

:Print_Usage
@rem echo usage: %~n0
echo usage: %~n0
goto Clean_Up

:Begin
@rem step1: initialize ACE
@rem set SCRIPT_FILE="%~dp0\initialize_ACE.cmd"
@rem if NOT exist "%SCRIPT_FILE%" (
@rem  echo invalid script file ^(was: "%SCRIPT_FILE%"^)^, exiting
@rem  goto Failed
@rem )
@rem "%SCRIPT_FILE%" windows
@rem if %ERRORLEVEL% NEQ 0 (
@rem  echo failed to run script "%SCRIPT_FILE%"^, exiting
@rem  set RC=%ERRORLEVEL%
@rem  goto Failed
@rem )
@rem echo "%SCRIPT_FILE%"...DONE

@rem step2: check out gtkglarea branch
set PROJECTS_ROOT_DIR_DEFAULT=D:\projects
set PROJECTS_ROOT_DIR="%PRJ_ROOT%"
if NOT exist "%PROJECTS_ROOT_DIR%" (
 set PROJECTS_ROOT_DIR="%PROJECTS_ROOT_DIR_DEFAULT%"
)
if NOT exist "%PROJECTS_ROOT_DIR%" (
 echo invalid projects directory ^(was: "%PROJECTS_ROOT_DIR%"^)^, exiting
 goto Failed
)
set SCRIPT_FILE="%PROJECTS_ROOT_DIR%\tools\development\win32\prj\checkout_branch.cmd"
if NOT exist "%SCRIPT_FILE%" (
 echo invalid script file ^(was: "%SCRIPT_FILE%"^)^, exiting
 goto Failed
)
"%SCRIPT_FILE%" gtkglarea master
if %ERRORLEVEL% NEQ 0 (
 echo failed to run script "%SCRIPT_FILE%"^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)
echo "%SCRIPT_FILE%"...DONE

goto Clean_Up

:Failed
echo "%SCRIPT_FILE%"...FAILED

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

