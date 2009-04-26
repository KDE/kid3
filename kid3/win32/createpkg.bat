mkdir kid3win\imageformats
copy %QTDIR%\plugins\imageformats\qjpeg4.dll kid3win\imageformats
for %%L in (Core Gui Network Xml) do copy %QTDIR%\bin\Qt%%L4.dll kid3win
copy %MSYSDIR%\mingw\bin\mingwm10.dll kid3win
for /f %%N in ('%msysdir%\bin\perl -ne "print \"kid3-winbin-$1.zip\" if /Version:\s+([\d\.]+)/" ../kid3.lsm') do set ZIPNAME=%%N
%ProgramFiles%\7-Zip\7z.exe a %ZIPNAME% kid3win
