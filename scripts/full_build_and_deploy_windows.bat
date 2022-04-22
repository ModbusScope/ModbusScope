echo on

REM Usage (path is optional):
REM Use cache: scripts\full_build_and_deploy_windows.bat 'true' [path]
REM No cache: scripts\full_build_and_deploy_windows.bat '' [path]

call scripts\setup_windows.bat %1 %2
call scripts\build_windows.bat
call scripts\deploy_windows.bat
