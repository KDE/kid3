@rem Make sure the following environment variables are set:
@rem QTDIR, MSYSDIR, PERLDIR, DUMPBINDIR, CMAKEDIR, XSLTPROCDIR, DOCBOOKDIR
set PATH=%QTDIR%\bin;%QTDIR%\..\bin;%QTDIR%\..\mingw\bin;%MSYSDIR%\mingw\bin;C:\WINNT\System32;C:\Windows\System32;%PERLDIR%\bin;%DUMPBINDIR%;%CMAKEDIR%\bin
set INCLUDE=%MSYSDIR%\local\include
set LIB=%MSYSDIR%\local\lib
cmake -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= -D WITH_TUNEPIMP=OFF -D WITH_MP4V2=ON ..\kid3
mingw32-make
cpack
