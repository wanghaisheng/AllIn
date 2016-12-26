setlocal

set PROJECT_NAME=%1

for /f "tokens=1,2,* " %%i in ('REG QUERY "HKLM\SOFTWARE\TortoiseSVN" ^| find /i "Directory"') do set "TSVN_PATH=%%k" 

for /f "delims=;" %%i in ('cd') do set WORK_DIR=%%i

SVNVersionGen -HEADER "%TSVN_PATH%\bin" "%WORK_DIR%" "SVNVersion.h"

"%TSVN_PATH%\bin\SubWCRev.exe" "%WORK_DIR%" "%PROJECT_NAME%.template.rc" "%PROJECT_NAME%.rc" -f

endlocal