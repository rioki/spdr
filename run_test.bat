
set CONFIG=%1

xcopy /Y %SIGC_DIR%\bin\*.dll %CONFIG%
xcopy /Y %C9Y_DIR%\bin\*.dll %CONFIG%
xcopy /Y %MUSLI_DIR%\bin\*.dll %CONFIG%

%CONFIG%\spdr_test.exe
