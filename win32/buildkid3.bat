set QTDIR=C:\Qt\4.8.6
set MSYSDIR=C:\msys\1.0
set MINGWDIR=C:\msys\1.0\mingw
set PYTHONDIR=C:\Python27
set DUMPBINDIR=%HOME%\prg\dumpbin
set CMAKEDIR=%HOME%\prg\cmake-2.8.10.2-win32-x86
set XSLTPROCDIR=%HOME%\prg\xsltproc
set DOCBOOKDIR=%HOME%\prg\docbook-xsl-1.72.0
@rem Make sure the following environment variables are set:
@rem QTDIR, MINGWDIR, PYTHONDIR, DUMPBINDIR, CMAKEDIR, XSLTPROCDIR, DOCBOOKDIR
set PATH=%QTDIR%\bin;%MINGWDIR%\bin;C:\WINNT\System32;C:\Windows\System32;%PYTHONDIR%;%DUMPBINDIR%;%CMAKEDIR%\bin
set INCLUDE=%MSYSDIR%\local\include
set LIB=%MSYSDIR%\local\lib
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON -DWITH_MP4V2=ON ..
mingw32-make
cpack
