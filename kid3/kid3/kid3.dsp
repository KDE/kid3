# Microsoft Developer Studio Project File - Name="kid3" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=kid3 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "kid3.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "kid3.mak" CFG="kid3 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "kid3 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "kid3 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "kid3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O1 /I "$(QTDIR)\include" /I "$(INCDIR)" /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D "NO_DEBUG" /D ID3LIB_LINKOPTION=3 /D "FLAC__NO_DLL" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 $(LIBDIR)\Release\id3lib.lib $(LIBDIR)\Release\vorbis_static.lib $(LIBDIR)\Release\vorbisfile_static.lib $(LIBDIR)\Release\ogg_static.lib $(LIBDIR)\Release\libFLAC_static.lib $(LIBDIR)\Release\libFLAC++_static.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib $(QTDIR)\lib\qt-mtnc321.lib $(QTDIR)\lib\qtmain.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcmt"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "$(INCDIR)" /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D ID3LIB_LINKOPTION=3 /D "FLAC__NO_DLL" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(LIBDIR)\Debug\id3lib.lib $(LIBDIR)\Debug\vorbis_static_d.lib $(LIBDIR)\Debug\vorbisfile_static_d.lib $(LIBDIR)\Debug\ogg_static_d.lib $(LIBDIR)\Debug\libFLAC_static.lib $(LIBDIR)\Debug\libFLAC++_static.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib $(QTDIR)\lib\qt-mtnc321.lib $(QTDIR)\lib\qtmain.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmtd" /nodefaultlib:"msvcrtd" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "kid3 - Win32 Release"
# Name "kid3 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\commandstable.cpp
# End Source File
# Begin Source File

SOURCE=.\configdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\dirlist.cpp
# End Source File
# Begin Source File

SOURCE=.\exportdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\filelist.cpp
# End Source File
# Begin Source File

SOURCE=.\flacfile.cpp
# End Source File
# Begin Source File

SOURCE=.\flacframelist.cpp
# End Source File
# Begin Source File

SOURCE=.\formatbox.cpp
# End Source File
# Begin Source File

SOURCE=.\formatconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\framelist.cpp
# End Source File
# Begin Source File

SOURCE=.\freedbclient.cpp
# End Source File
# Begin Source File

SOURCE=.\freedbconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\freedbdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\generalconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\genres.cpp
# End Source File
# Begin Source File

SOURCE=.\id3form.cpp
# End Source File
# Begin Source File

SOURCE=.\importconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\importdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\importparser.cpp
# End Source File
# Begin Source File

SOURCE=.\importselector.cpp
# End Source File
# Begin Source File

SOURCE=.\kid3.cpp
# End Source File
# Begin Source File

SOURCE=.\kid3win.rc
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\miscconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_commandstable.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_exportdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_filelist.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_formatbox.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_freedbclient.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_freedbdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_id3form.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_importdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_importselector.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_kid3.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_mp3framelist.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_musicbrainzclient.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_musicbrainzdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_rendirdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\mp3file.cpp
# End Source File
# Begin Source File

SOURCE=.\mp3framelist.cpp
# End Source File
# Begin Source File

SOURCE=.\musicbrainzclient.cpp
# End Source File
# Begin Source File

SOURCE=.\musicbrainzconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\musicbrainzdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\numbertracksdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\oggfile.cpp
# End Source File
# Begin Source File

SOURCE=.\oggframelist.cpp
# End Source File
# Begin Source File

SOURCE=.\rendirdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\standardtags.cpp
# End Source File
# Begin Source File

SOURCE=.\taggedfile.cpp
# End Source File
# Begin Source File

SOURCE=.\vcedit.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\commandstable.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\commandstable.h

".\moc_commandstable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_commandstable.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\commandstable.h

".\moc_commandstable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_commandstable.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\configdialog.h
# End Source File
# Begin Source File

SOURCE=.\dirlist.h
# End Source File
# Begin Source File

SOURCE=.\exportdialog.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\exportdialog.h

".\moc_exportdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_exportdialog.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\exportdialog.h

".\moc_exportdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_exportdialog.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\filelist.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\filelist.h

".\moc_filelist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_filelist.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\filelist.h

".\moc_filelist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_filelist.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\flacfile.h
# End Source File
# Begin Source File

SOURCE=.\flacframelist.h
# End Source File
# Begin Source File

SOURCE=.\formatbox.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\formatbox.h

".\moc_formatbox.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_formatbox.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\formatbox.h

".\moc_formatbox.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_formatbox.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\formatconfig.h
# End Source File
# Begin Source File

SOURCE=.\framelist.h
# End Source File
# Begin Source File

SOURCE=.\freedbclient.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\freedbclient.h

".\moc_freedbclient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_freedbclient.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\freedbclient.h

".\moc_freedbclient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_freedbclient.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\freedbconfig.h
# End Source File
# Begin Source File

SOURCE=.\freedbdialog.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\freedbdialog.h

".\moc_freedbdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_freedbdialog.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\freedbdialog.h

".\moc_freedbdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_freedbdialog.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\generalconfig.h
# End Source File
# Begin Source File

SOURCE=.\genres.h
# End Source File
# Begin Source File

SOURCE=.\id3form.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\id3form.h

".\moc_id3form.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_id3form.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\id3form.h

".\moc_id3form.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_id3form.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\importconfig.h
# End Source File
# Begin Source File

SOURCE=.\importdialog.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\importdialog.h

".\moc_importdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_importdialog.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\importdialog.h

".\moc_importdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_importdialog.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\importparser.h
# End Source File
# Begin Source File

SOURCE=.\importselector.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\importselector.h

".\moc_importselector.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_importselector.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\importselector.h

".\moc_importselector.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_importselector.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\importtrackdata.h
# End Source File
# Begin Source File

SOURCE=.\kid3.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\kid3.h

".\moc_kid3.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_kid3.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\kid3.h

".\moc_kid3.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_kid3.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\miscconfig.h
# End Source File
# Begin Source File

SOURCE=.\mp3file.h
# End Source File
# Begin Source File

SOURCE=.\mp3framelist.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\mp3framelist.h

".\moc_mp3framelist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_mp3framelist.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\mp3framelist.h

".\moc_mp3framelist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_mp3framelist.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\musicbrainzclient.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\musicbrainzclient.h

".\moc_musicbrainzclient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_musicbrainzclient.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\musicbrainzclient.h

".\moc_musicbrainzclient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_musicbrainzclient.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\musicbrainzconfig.h
# End Source File
# Begin Source File

SOURCE=.\musicbrainzdialog.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\musicbrainzdialog.h

".\moc_musicbrainzdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_musicbrainzdialog.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\musicbrainzdialog.h

".\moc_musicbrainzdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_musicbrainzdialog.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\numbertracksdialog.h
# End Source File
# Begin Source File

SOURCE=.\oggfile.h
# End Source File
# Begin Source File

SOURCE=.\oggframelist.h
# End Source File
# Begin Source File

SOURCE=.\rendirdialog.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\rendirdialog.h

".\moc_rendirdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_rendirdialog.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputPath)
InputPath=.\rendirdialog.h

".\moc_rendirdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_rendirdialog.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\standardtags.h
# End Source File
# Begin Source File

SOURCE=.\taggedfile.h
# End Source File
# Begin Source File

SOURCE=.\vcedit.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\kid3.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\config.mk

!IF  "$(CFG)" == "kid3 - Win32 Release"

# Begin Custom Build - Creating config.h from $(InputPath)
InputPath=.\config.mk

"..\config.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%PERLDIR%\bin\perl.exe  -ne "print if (s/^([^#][^=\s]*)[\s=]+(.+)$/#define \1 \2/)" $(InputPath) >..\config.h

# End Custom Build

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

# Begin Custom Build - Creating config.h from $(InputPath)
InputPath=.\config.mk

"..\config.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%PERLDIR%\bin\perl.exe  -ne "print if (s/^([^#][^=\s]*)[\s=]+(.+)$/#define \1 \2/)" $(InputPath) >..\config.h

# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
