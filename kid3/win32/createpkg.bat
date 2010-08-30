mkdir kid3win\imageformats
mkdir kid3win\phonon_backend
copy %QTDIR%\plugins\imageformats\qjpeg4.dll kid3win\imageformats
copy %QTDIR%\plugins\phonon_backend\phonon_ds94.dll kid3win\phonon_backend
for %%L in (Core Gui Network Xml) do copy %QTDIR%\bin\Qt%%L4.dll kid3win
copy %QTDIR%\bin\phonon4.dll kid3win
copy %QTDIR%\bin\mingwm10.dll kid3win
copy %QTDIR%\bin\libgcc_s_dw2-1.dll kid3win
for /f %%N in ('%MSYSDIR%\bin\perl -ne "print \"kid3-winbin-$1.zip\" if /Version:\s+([\d\.]+)/" ../kid3.lsm') do set ZIPNAME=%%N
%ProgramFiles%\7-Zip\7z.exe a %ZIPNAME% kid3win
