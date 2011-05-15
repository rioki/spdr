
set CONFIG=%1

xcopy /Y spdr\%CONFIG%\spdr.dll %SPDR_DIR%\bin 
xcopy /Y spdr\%CONFIG%\spdr.dll %SPDR_DIR%\lib
xcopy /Y spdr\*.h %SPDR_DIR%\include\spdr
