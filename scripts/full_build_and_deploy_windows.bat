echo on

call scripts\setup_windows.bat %1
call scripts\build_windows.bat
call scripts\deploy_windows.bat