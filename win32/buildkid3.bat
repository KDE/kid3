@rem Make sure the following environment variables are set:
@rem QTDIR, MSYSDIR, PERLDIR, DUMPBINDIR, CMAKEDIR, XSLTPROCDIR, DOCBOOKDIR
set PATH=%QTDIR%\bin;%MSYSDIR%\mingw\bin;C:\WINNT\System32;%PERLDIR%\bin;%DUMPBINDIR%
set INCLUDE=%MSYSDIR%\local\include
set LIB=%MSYSDIR%\local\lib
%CMAKEDIR%\bin\cmake.exe -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= -D WITH_TUNEPIMP=OFF ..
mingw32-make
%CMAKEDIR%\bin\cpack.exe
