
set CONFIG=%1

xcopy /Y %SIGC_DIR%\bin\*.dll %CONFIG%
xcopy /Y %C9Y_DIR%\%CONFIG%\*.dll %CONFIG%
xcopy /Y %MUSLI_DIR%\%CONFIG%\*.dll %CONFIG%
xcopy /Y %SANITY_DIR%\%CONFIG%\*.dll %CONFIG%

%CONFIG%\spdr_test.exe
