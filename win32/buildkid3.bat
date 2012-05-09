set QTDIR=C:\Qt\4.8.1
set MSYSDIR=C:\msys\1.0
set MINGWDIR=C:\msys\1.0\mingw
set PERLDIR=%HOME%\prg\Perl
set DUMPBINDIR=%HOME%\prg\dumpbin
set CMAKEDIR=%HOME%\prg\cmake-2.8.6-win32-x86
set XSLTPROCDIR=%HOME%\prg\xsltproc
set DOCBOOKDIR=%HOME%\prg\docbook-xsl-1.72.0
@rem Make sure the following environment variables are set:
@rem QTDIR, MINGWDIR, PERLDIR, DUMPBINDIR, CMAKEDIR, XSLTPROCDIR, DOCBOOKDIR
set PATH=%QTDIR%\bin;%MINGWDIR%\bin;C:\WINNT\System32;C:\Windows\System32;%PERLDIR%\bin;%DUMPBINDIR%;%CMAKEDIR%\bin
set INCLUDE=%MSYSDIR%\local\include
set LIB=%MSYSDIR%\local\lib
cmake -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= ..
mingw32-make
cpack
