@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@rem #// File Name: start_soffice.bat
@rem #//
@rem #// History:
@rem #//   Date   |Name | Description of modification
@rem #// ---------|-----|-------------------------------------------------------------
@rem #// 20/02/06 | soh | Creation.
@rem #//%%%FILE%%%////////////////////////////////////////////////////////////////////
@echo off
set RC=0
setlocal enabledelayedexpansion
pushd . >NUL 2>&1

set SOfficeEXE="%ProgramFiles(x86)%\LibreOffice 5\program\soffice.exe"
if exist %SOfficeEXE% goto Next
echo invalid file ^(was: "%SOfficeEXE%"^)^, exiting
goto Failed

:Next
%SOfficeEXE% --accept="socket,host=localhost,port=2083;urp;StarOffice.ServiceManager" --nofirststartwizard --nologo --headless --norestore --invisible >>%TEMP%\libreoffice.log 2>&1
if %ERRORLEVEL% NEQ 0 (
 echo failed to start office server %SOfficeEXE%^, exiting
 set RC=%ERRORLEVEL%
 goto Failed
)

goto Clean_Up

:Failed

:Clean_Up
popd
::endlocal & set RC=%ERRORLEVEL%
endlocal & set RC=%RC%
goto Error_Level

:Exit_Code
::echo %ERRORLEVEL% %1 *WORKAROUND*
exit /b %1

:Error_Level
call :Exit_Code %RC%
