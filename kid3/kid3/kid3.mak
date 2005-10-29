# Microsoft Developer Studio Generated NMAKE File, Based on kid3.dsp
!IF "$(CFG)" == ""
CFG=kid3 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to kid3 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "kid3 - Win32 Release" && "$(CFG)" != "kid3 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "kid3 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\kid3.exe"


CLEAN :
	-@erase "$(INTDIR)\commandstable.obj"
	-@erase "$(INTDIR)\configdialog.obj"
	-@erase "$(INTDIR)\dirlist.obj"
	-@erase "$(INTDIR)\filelist.obj"
	-@erase "$(INTDIR)\flacfile.obj"
	-@erase "$(INTDIR)\flacframelist.obj"
	-@erase "$(INTDIR)\formatbox.obj"
	-@erase "$(INTDIR)\formatconfig.obj"
	-@erase "$(INTDIR)\framelist.obj"
	-@erase "$(INTDIR)\freedbclient.obj"
	-@erase "$(INTDIR)\freedbconfig.obj"
	-@erase "$(INTDIR)\freedbdialog.obj"
	-@erase "$(INTDIR)\generalconfig.obj"
	-@erase "$(INTDIR)\genres.obj"
	-@erase "$(INTDIR)\id3form.obj"
	-@erase "$(INTDIR)\importconfig.obj"
	-@erase "$(INTDIR)\importdialog.obj"
	-@erase "$(INTDIR)\importparser.obj"
	-@erase "$(INTDIR)\importselector.obj"
	-@erase "$(INTDIR)\kid3.obj"
	-@erase "$(INTDIR)\kid3win.res"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\miscconfig.obj"
	-@erase "$(INTDIR)\moc_commandstable.obj"
	-@erase "$(INTDIR)\moc_filelist.obj"
	-@erase "$(INTDIR)\moc_formatbox.obj"
	-@erase "$(INTDIR)\moc_freedbclient.obj"
	-@erase "$(INTDIR)\moc_freedbdialog.obj"
	-@erase "$(INTDIR)\moc_id3form.obj"
	-@erase "$(INTDIR)\moc_importselector.obj"
	-@erase "$(INTDIR)\moc_kid3.obj"
	-@erase "$(INTDIR)\moc_mp3framelist.obj"
	-@erase "$(INTDIR)\moc_musicbrainzclient.obj"
	-@erase "$(INTDIR)\moc_musicbrainzdialog.obj"
	-@erase "$(INTDIR)\moc_rendirdialog.obj"
	-@erase "$(INTDIR)\mp3file.obj"
	-@erase "$(INTDIR)\mp3framelist.obj"
	-@erase "$(INTDIR)\musicbrainzclient.obj"
	-@erase "$(INTDIR)\musicbrainzconfig.obj"
	-@erase "$(INTDIR)\musicbrainzdialog.obj"
	-@erase "$(INTDIR)\oggfile.obj"
	-@erase "$(INTDIR)\oggframelist.obj"
	-@erase "$(INTDIR)\rendirdialog.obj"
	-@erase "$(INTDIR)\standardtags.obj"
	-@erase "$(INTDIR)\taggedfile.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vcedit.obj"
	-@erase "$(OUTDIR)\kid3.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /O1 /I "$(QTDIR)\include" /I "$(INCDIR)" /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D "NO_DEBUG" /D ID3LIB_LINKOPTION=3 /D "FLAC__NO_DLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\kid3win.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\kid3.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(LIBDIR)\Release\id3lib.lib $(LIBDIR)\Release\vorbis_static.lib $(LIBDIR)\Release\vorbisfile_static.lib $(LIBDIR)\Release\ogg_static.lib $(LIBDIR)\Release\libFLAC_static.lib $(LIBDIR)\Release\libFLAC++_static.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib $(QTDIR)\lib\qt-mtnc321.lib $(QTDIR)\lib\qtmain.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\kid3.pdb" /machine:I386 /nodefaultlib:"libcmt" /out:"$(OUTDIR)\kid3.exe" 
LINK32_OBJS= \
	"$(INTDIR)\commandstable.obj" \
	"$(INTDIR)\configdialog.obj" \
	"$(INTDIR)\dirlist.obj" \
	"$(INTDIR)\filelist.obj" \
	"$(INTDIR)\flacfile.obj" \
	"$(INTDIR)\flacframelist.obj" \
	"$(INTDIR)\formatbox.obj" \
	"$(INTDIR)\formatconfig.obj" \
	"$(INTDIR)\framelist.obj" \
	"$(INTDIR)\freedbclient.obj" \
	"$(INTDIR)\freedbconfig.obj" \
	"$(INTDIR)\freedbdialog.obj" \
	"$(INTDIR)\generalconfig.obj" \
	"$(INTDIR)\genres.obj" \
	"$(INTDIR)\id3form.obj" \
	"$(INTDIR)\importconfig.obj" \
	"$(INTDIR)\importdialog.obj" \
	"$(INTDIR)\importparser.obj" \
	"$(INTDIR)\importselector.obj" \
	"$(INTDIR)\kid3.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\miscconfig.obj" \
	"$(INTDIR)\moc_commandstable.obj" \
	"$(INTDIR)\moc_filelist.obj" \
	"$(INTDIR)\moc_formatbox.obj" \
	"$(INTDIR)\moc_freedbclient.obj" \
	"$(INTDIR)\moc_freedbdialog.obj" \
	"$(INTDIR)\moc_id3form.obj" \
	"$(INTDIR)\moc_importselector.obj" \
	"$(INTDIR)\moc_kid3.obj" \
	"$(INTDIR)\moc_mp3framelist.obj" \
	"$(INTDIR)\moc_musicbrainzclient.obj" \
	"$(INTDIR)\moc_musicbrainzdialog.obj" \
	"$(INTDIR)\moc_rendirdialog.obj" \
	"$(INTDIR)\mp3file.obj" \
	"$(INTDIR)\mp3framelist.obj" \
	"$(INTDIR)\musicbrainzclient.obj" \
	"$(INTDIR)\musicbrainzconfig.obj" \
	"$(INTDIR)\musicbrainzdialog.obj" \
	"$(INTDIR)\oggfile.obj" \
	"$(INTDIR)\oggframelist.obj" \
	"$(INTDIR)\rendirdialog.obj" \
	"$(INTDIR)\standardtags.obj" \
	"$(INTDIR)\taggedfile.obj" \
	"$(INTDIR)\vcedit.obj" \
	"$(INTDIR)\kid3win.res"

"$(OUTDIR)\kid3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\kid3.exe" "$(OUTDIR)\kid3.bsc"


CLEAN :
	-@erase "$(INTDIR)\commandstable.obj"
	-@erase "$(INTDIR)\commandstable.sbr"
	-@erase "$(INTDIR)\configdialog.obj"
	-@erase "$(INTDIR)\configdialog.sbr"
	-@erase "$(INTDIR)\dirlist.obj"
	-@erase "$(INTDIR)\dirlist.sbr"
	-@erase "$(INTDIR)\filelist.obj"
	-@erase "$(INTDIR)\filelist.sbr"
	-@erase "$(INTDIR)\flacfile.obj"
	-@erase "$(INTDIR)\flacfile.sbr"
	-@erase "$(INTDIR)\flacframelist.obj"
	-@erase "$(INTDIR)\flacframelist.sbr"
	-@erase "$(INTDIR)\formatbox.obj"
	-@erase "$(INTDIR)\formatbox.sbr"
	-@erase "$(INTDIR)\formatconfig.obj"
	-@erase "$(INTDIR)\formatconfig.sbr"
	-@erase "$(INTDIR)\framelist.obj"
	-@erase "$(INTDIR)\framelist.sbr"
	-@erase "$(INTDIR)\freedbclient.obj"
	-@erase "$(INTDIR)\freedbclient.sbr"
	-@erase "$(INTDIR)\freedbconfig.obj"
	-@erase "$(INTDIR)\freedbconfig.sbr"
	-@erase "$(INTDIR)\freedbdialog.obj"
	-@erase "$(INTDIR)\freedbdialog.sbr"
	-@erase "$(INTDIR)\generalconfig.obj"
	-@erase "$(INTDIR)\generalconfig.sbr"
	-@erase "$(INTDIR)\genres.obj"
	-@erase "$(INTDIR)\genres.sbr"
	-@erase "$(INTDIR)\id3form.obj"
	-@erase "$(INTDIR)\id3form.sbr"
	-@erase "$(INTDIR)\importconfig.obj"
	-@erase "$(INTDIR)\importconfig.sbr"
	-@erase "$(INTDIR)\importdialog.obj"
	-@erase "$(INTDIR)\importdialog.sbr"
	-@erase "$(INTDIR)\importparser.obj"
	-@erase "$(INTDIR)\importparser.sbr"
	-@erase "$(INTDIR)\importselector.obj"
	-@erase "$(INTDIR)\importselector.sbr"
	-@erase "$(INTDIR)\kid3.obj"
	-@erase "$(INTDIR)\kid3.sbr"
	-@erase "$(INTDIR)\kid3win.res"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\miscconfig.obj"
	-@erase "$(INTDIR)\miscconfig.sbr"
	-@erase "$(INTDIR)\moc_commandstable.obj"
	-@erase "$(INTDIR)\moc_commandstable.sbr"
	-@erase "$(INTDIR)\moc_filelist.obj"
	-@erase "$(INTDIR)\moc_filelist.sbr"
	-@erase "$(INTDIR)\moc_formatbox.obj"
	-@erase "$(INTDIR)\moc_formatbox.sbr"
	-@erase "$(INTDIR)\moc_freedbclient.obj"
	-@erase "$(INTDIR)\moc_freedbclient.sbr"
	-@erase "$(INTDIR)\moc_freedbdialog.obj"
	-@erase "$(INTDIR)\moc_freedbdialog.sbr"
	-@erase "$(INTDIR)\moc_id3form.obj"
	-@erase "$(INTDIR)\moc_id3form.sbr"
	-@erase "$(INTDIR)\moc_importselector.obj"
	-@erase "$(INTDIR)\moc_importselector.sbr"
	-@erase "$(INTDIR)\moc_kid3.obj"
	-@erase "$(INTDIR)\moc_kid3.sbr"
	-@erase "$(INTDIR)\moc_mp3framelist.obj"
	-@erase "$(INTDIR)\moc_mp3framelist.sbr"
	-@erase "$(INTDIR)\moc_musicbrainzclient.obj"
	-@erase "$(INTDIR)\moc_musicbrainzclient.sbr"
	-@erase "$(INTDIR)\moc_musicbrainzdialog.obj"
	-@erase "$(INTDIR)\moc_musicbrainzdialog.sbr"
	-@erase "$(INTDIR)\moc_rendirdialog.obj"
	-@erase "$(INTDIR)\moc_rendirdialog.sbr"
	-@erase "$(INTDIR)\mp3file.obj"
	-@erase "$(INTDIR)\mp3file.sbr"
	-@erase "$(INTDIR)\mp3framelist.obj"
	-@erase "$(INTDIR)\mp3framelist.sbr"
	-@erase "$(INTDIR)\musicbrainzclient.obj"
	-@erase "$(INTDIR)\musicbrainzclient.sbr"
	-@erase "$(INTDIR)\musicbrainzconfig.obj"
	-@erase "$(INTDIR)\musicbrainzconfig.sbr"
	-@erase "$(INTDIR)\musicbrainzdialog.obj"
	-@erase "$(INTDIR)\musicbrainzdialog.sbr"
	-@erase "$(INTDIR)\oggfile.obj"
	-@erase "$(INTDIR)\oggfile.sbr"
	-@erase "$(INTDIR)\oggframelist.obj"
	-@erase "$(INTDIR)\oggframelist.sbr"
	-@erase "$(INTDIR)\rendirdialog.obj"
	-@erase "$(INTDIR)\rendirdialog.sbr"
	-@erase "$(INTDIR)\standardtags.obj"
	-@erase "$(INTDIR)\standardtags.sbr"
	-@erase "$(INTDIR)\taggedfile.obj"
	-@erase "$(INTDIR)\taggedfile.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vcedit.obj"
	-@erase "$(INTDIR)\vcedit.sbr"
	-@erase "$(OUTDIR)\kid3.bsc"
	-@erase "$(OUTDIR)\kid3.exe"
	-@erase "$(OUTDIR)\kid3.ilk"
	-@erase "$(OUTDIR)\kid3.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Gm /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "$(INCDIR)" /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /D "UNICODE" /D "QT_THREAD_SUPPORT" /D ID3LIB_LINKOPTION=3 /D "FLAC__NO_DLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\kid3win.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\kid3.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commandstable.sbr" \
	"$(INTDIR)\configdialog.sbr" \
	"$(INTDIR)\dirlist.sbr" \
	"$(INTDIR)\filelist.sbr" \
	"$(INTDIR)\flacfile.sbr" \
	"$(INTDIR)\flacframelist.sbr" \
	"$(INTDIR)\formatbox.sbr" \
	"$(INTDIR)\formatconfig.sbr" \
	"$(INTDIR)\framelist.sbr" \
	"$(INTDIR)\freedbclient.sbr" \
	"$(INTDIR)\freedbconfig.sbr" \
	"$(INTDIR)\freedbdialog.sbr" \
	"$(INTDIR)\generalconfig.sbr" \
	"$(INTDIR)\genres.sbr" \
	"$(INTDIR)\id3form.sbr" \
	"$(INTDIR)\importconfig.sbr" \
	"$(INTDIR)\importdialog.sbr" \
	"$(INTDIR)\importparser.sbr" \
	"$(INTDIR)\importselector.sbr" \
	"$(INTDIR)\kid3.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\miscconfig.sbr" \
	"$(INTDIR)\moc_commandstable.sbr" \
	"$(INTDIR)\moc_filelist.sbr" \
	"$(INTDIR)\moc_formatbox.sbr" \
	"$(INTDIR)\moc_freedbclient.sbr" \
	"$(INTDIR)\moc_freedbdialog.sbr" \
	"$(INTDIR)\moc_id3form.sbr" \
	"$(INTDIR)\moc_importselector.sbr" \
	"$(INTDIR)\moc_kid3.sbr" \
	"$(INTDIR)\moc_mp3framelist.sbr" \
	"$(INTDIR)\moc_musicbrainzclient.sbr" \
	"$(INTDIR)\moc_musicbrainzdialog.sbr" \
	"$(INTDIR)\moc_rendirdialog.sbr" \
	"$(INTDIR)\mp3file.sbr" \
	"$(INTDIR)\mp3framelist.sbr" \
	"$(INTDIR)\musicbrainzclient.sbr" \
	"$(INTDIR)\musicbrainzconfig.sbr" \
	"$(INTDIR)\musicbrainzdialog.sbr" \
	"$(INTDIR)\oggfile.sbr" \
	"$(INTDIR)\oggframelist.sbr" \
	"$(INTDIR)\rendirdialog.sbr" \
	"$(INTDIR)\standardtags.sbr" \
	"$(INTDIR)\taggedfile.sbr" \
	"$(INTDIR)\vcedit.sbr"

"$(OUTDIR)\kid3.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(LIBDIR)\Debug\id3lib.lib $(LIBDIR)\Debug\vorbis_static_d.lib $(LIBDIR)\Debug\vorbisfile_static_d.lib $(LIBDIR)\Debug\ogg_static_d.lib $(LIBDIR)\Debug\libFLAC_static.lib $(LIBDIR)\Debug\libFLAC++_static.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib $(QTDIR)\lib\qt-mtnc321.lib $(QTDIR)\lib\qtmain.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\kid3.pdb" /debug /machine:I386 /nodefaultlib:"libcmtd" /nodefaultlib:"msvcrtd" /out:"$(OUTDIR)\kid3.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\commandstable.obj" \
	"$(INTDIR)\configdialog.obj" \
	"$(INTDIR)\dirlist.obj" \
	"$(INTDIR)\filelist.obj" \
	"$(INTDIR)\flacfile.obj" \
	"$(INTDIR)\flacframelist.obj" \
	"$(INTDIR)\formatbox.obj" \
	"$(INTDIR)\formatconfig.obj" \
	"$(INTDIR)\framelist.obj" \
	"$(INTDIR)\freedbclient.obj" \
	"$(INTDIR)\freedbconfig.obj" \
	"$(INTDIR)\freedbdialog.obj" \
	"$(INTDIR)\generalconfig.obj" \
	"$(INTDIR)\genres.obj" \
	"$(INTDIR)\id3form.obj" \
	"$(INTDIR)\importconfig.obj" \
	"$(INTDIR)\importdialog.obj" \
	"$(INTDIR)\importparser.obj" \
	"$(INTDIR)\importselector.obj" \
	"$(INTDIR)\kid3.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\miscconfig.obj" \
	"$(INTDIR)\moc_commandstable.obj" \
	"$(INTDIR)\moc_filelist.obj" \
	"$(INTDIR)\moc_formatbox.obj" \
	"$(INTDIR)\moc_freedbclient.obj" \
	"$(INTDIR)\moc_freedbdialog.obj" \
	"$(INTDIR)\moc_id3form.obj" \
	"$(INTDIR)\moc_importselector.obj" \
	"$(INTDIR)\moc_kid3.obj" \
	"$(INTDIR)\moc_mp3framelist.obj" \
	"$(INTDIR)\moc_musicbrainzclient.obj" \
	"$(INTDIR)\moc_musicbrainzdialog.obj" \
	"$(INTDIR)\moc_rendirdialog.obj" \
	"$(INTDIR)\mp3file.obj" \
	"$(INTDIR)\mp3framelist.obj" \
	"$(INTDIR)\musicbrainzclient.obj" \
	"$(INTDIR)\musicbrainzconfig.obj" \
	"$(INTDIR)\musicbrainzdialog.obj" \
	"$(INTDIR)\oggfile.obj" \
	"$(INTDIR)\oggframelist.obj" \
	"$(INTDIR)\rendirdialog.obj" \
	"$(INTDIR)\standardtags.obj" \
	"$(INTDIR)\taggedfile.obj" \
	"$(INTDIR)\vcedit.obj" \
	"$(INTDIR)\kid3win.res"

"$(OUTDIR)\kid3.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("kid3.dep")
!INCLUDE "kid3.dep"
!ELSE 
!MESSAGE Warning: cannot find "kid3.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "kid3 - Win32 Release" || "$(CFG)" == "kid3 - Win32 Debug"
SOURCE=.\commandstable.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\commandstable.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\commandstable.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\commandstable.obj"	"$(INTDIR)\commandstable.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\commandstable.h"


!ENDIF 

SOURCE=.\configdialog.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\configdialog.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\commandstable.h" ".\formatbox.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\configdialog.obj"	"$(INTDIR)\configdialog.sbr" : $(SOURCE) "$(INTDIR)" ".\formatbox.h" ".\commandstable.h" "..\config.h"


!ENDIF 

SOURCE=.\dirlist.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\dirlist.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\dirlist.obj"	"$(INTDIR)\dirlist.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\filelist.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\filelist.obj" : $(SOURCE) "$(INTDIR)" ".\filelist.h" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\filelist.obj"	"$(INTDIR)\filelist.sbr" : $(SOURCE) "$(INTDIR)" ".\filelist.h" "..\config.h"


!ENDIF 

SOURCE=.\flacfile.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\flacfile.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\flacfile.obj"	"$(INTDIR)\flacfile.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\flacframelist.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\flacframelist.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\flacframelist.obj"	"$(INTDIR)\flacframelist.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\formatbox.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\formatbox.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\formatbox.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\formatbox.obj"	"$(INTDIR)\formatbox.sbr" : $(SOURCE) "$(INTDIR)" ".\formatbox.h" "..\config.h"


!ENDIF 

SOURCE=.\formatconfig.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\formatconfig.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\formatconfig.obj"	"$(INTDIR)\formatconfig.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\framelist.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\framelist.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\framelist.obj"	"$(INTDIR)\framelist.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\freedbclient.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\freedbclient.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\freedbclient.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\freedbclient.obj"	"$(INTDIR)\freedbclient.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\freedbclient.h"


!ENDIF 

SOURCE=.\freedbconfig.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\freedbconfig.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\freedbconfig.obj"	"$(INTDIR)\freedbconfig.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\freedbdialog.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\freedbdialog.obj" : $(SOURCE) "$(INTDIR)" ".\freedbclient.h" "..\config.h" ".\freedbdialog.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\freedbdialog.obj"	"$(INTDIR)\freedbdialog.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\freedbdialog.h" ".\freedbclient.h"


!ENDIF 

SOURCE=.\generalconfig.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\generalconfig.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\generalconfig.obj"	"$(INTDIR)\generalconfig.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\genres.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\genres.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\genres.obj"	"$(INTDIR)\genres.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\id3form.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\id3form.obj" : $(SOURCE) "$(INTDIR)" ".\filelist.h" ".\kid3.h" "..\config.h" ".\id3form.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\id3form.obj"	"$(INTDIR)\id3form.sbr" : $(SOURCE) "$(INTDIR)" ".\kid3.h" ".\id3form.h" ".\filelist.h" "..\config.h"


!ENDIF 

SOURCE=.\importconfig.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\importconfig.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\importconfig.obj"	"$(INTDIR)\importconfig.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\importdialog.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\importdialog.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\importselector.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\importdialog.obj"	"$(INTDIR)\importdialog.sbr" : $(SOURCE) "$(INTDIR)" ".\importselector.h" "..\config.h"


!ENDIF 

SOURCE=.\importparser.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\importparser.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\importparser.obj"	"$(INTDIR)\importparser.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\importselector.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\importselector.obj" : $(SOURCE) "$(INTDIR)" ".\importselector.h" ".\freedbdialog.h" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\importselector.obj"	"$(INTDIR)\importselector.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\importselector.h" ".\freedbdialog.h"


!ENDIF 

SOURCE=.\kid3.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\kid3.obj" : $(SOURCE) "$(INTDIR)" ".\id3form.h" "..\config.h" ".\mp3framelist.h" ".\filelist.h" ".\rendirdialog.h" ".\kid3.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\kid3.obj"	"$(INTDIR)\kid3.sbr" : $(SOURCE) "$(INTDIR)" ".\rendirdialog.h" "..\config.h" ".\mp3framelist.h" ".\filelist.h" ".\id3form.h" ".\kid3.h"


!ENDIF 

SOURCE=.\kid3win.rc

"$(INTDIR)\kid3win.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\main.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" ".\kid3.h" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)" ".\kid3.h" "..\config.h"


!ENDIF 

SOURCE=.\miscconfig.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\miscconfig.obj" : $(SOURCE) "$(INTDIR)" ".\filelist.h" "..\config.h" ".\rendirdialog.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\miscconfig.obj"	"$(INTDIR)\miscconfig.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\rendirdialog.h" ".\filelist.h"


!ENDIF 

SOURCE=.\moc_commandstable.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_commandstable.obj" : $(SOURCE) "$(INTDIR)" ".\commandstable.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_commandstable.obj"	"$(INTDIR)\moc_commandstable.sbr" : $(SOURCE) "$(INTDIR)" ".\commandstable.h"


!ENDIF 

SOURCE=.\moc_filelist.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_filelist.obj" : $(SOURCE) "$(INTDIR)" ".\filelist.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_filelist.obj"	"$(INTDIR)\moc_filelist.sbr" : $(SOURCE) "$(INTDIR)" ".\filelist.h"


!ENDIF 

SOURCE=.\moc_formatbox.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_formatbox.obj" : $(SOURCE) "$(INTDIR)" ".\formatbox.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_formatbox.obj"	"$(INTDIR)\moc_formatbox.sbr" : $(SOURCE) "$(INTDIR)" ".\formatbox.h"


!ENDIF 

SOURCE=.\moc_freedbclient.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_freedbclient.obj" : $(SOURCE) "$(INTDIR)" ".\freedbclient.h" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_freedbclient.obj"	"$(INTDIR)\moc_freedbclient.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\freedbclient.h"


!ENDIF 

SOURCE=.\moc_freedbdialog.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_freedbdialog.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\freedbdialog.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_freedbdialog.obj"	"$(INTDIR)\moc_freedbdialog.sbr" : $(SOURCE) "$(INTDIR)" ".\freedbdialog.h" "..\config.h"


!ENDIF 

SOURCE=.\moc_id3form.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_id3form.obj" : $(SOURCE) "$(INTDIR)" ".\id3form.h" ".\filelist.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_id3form.obj"	"$(INTDIR)\moc_id3form.sbr" : $(SOURCE) "$(INTDIR)" ".\id3form.h" ".\filelist.h"


!ENDIF 

SOURCE=.\moc_importselector.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_importselector.obj" : $(SOURCE) "$(INTDIR)" ".\importselector.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_importselector.obj"	"$(INTDIR)\moc_importselector.sbr" : $(SOURCE) "$(INTDIR)" ".\importselector.h"


!ENDIF 

SOURCE=.\moc_kid3.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_kid3.obj" : $(SOURCE) "$(INTDIR)" ".\kid3.h" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_kid3.obj"	"$(INTDIR)\moc_kid3.sbr" : $(SOURCE) "$(INTDIR)" ".\kid3.h" "..\config.h"


!ENDIF 

SOURCE=.\moc_mp3framelist.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_mp3framelist.obj" : $(SOURCE) "$(INTDIR)" ".\mp3framelist.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_mp3framelist.obj"	"$(INTDIR)\moc_mp3framelist.sbr" : $(SOURCE) "$(INTDIR)" ".\mp3framelist.h"


!ENDIF 

SOURCE=.\moc_musicbrainzclient.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_musicbrainzclient.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\musicbrainzclient.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_musicbrainzclient.obj"	"$(INTDIR)\moc_musicbrainzclient.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\musicbrainzclient.h"


!ENDIF 

SOURCE=.\moc_musicbrainzdialog.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_musicbrainzdialog.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\musicbrainzdialog.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_musicbrainzdialog.obj"	"$(INTDIR)\moc_musicbrainzdialog.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\musicbrainzdialog.h"


!ENDIF 

SOURCE=.\moc_rendirdialog.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\moc_rendirdialog.obj" : $(SOURCE) "$(INTDIR)" ".\rendirdialog.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\moc_rendirdialog.obj"	"$(INTDIR)\moc_rendirdialog.sbr" : $(SOURCE) "$(INTDIR)" ".\rendirdialog.h"


!ENDIF 

SOURCE=.\mp3file.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\mp3file.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\mp3framelist.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\mp3file.obj"	"$(INTDIR)\mp3file.sbr" : $(SOURCE) "$(INTDIR)" ".\mp3framelist.h" "..\config.h"


!ENDIF 

SOURCE=.\mp3framelist.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\mp3framelist.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\mp3framelist.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\mp3framelist.obj"	"$(INTDIR)\mp3framelist.sbr" : $(SOURCE) "$(INTDIR)" ".\mp3framelist.h" "..\config.h"


!ENDIF 

SOURCE=.\musicbrainzclient.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\musicbrainzclient.obj" : $(SOURCE) "$(INTDIR)" ".\musicbrainzclient.h" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\musicbrainzclient.obj"	"$(INTDIR)\musicbrainzclient.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\musicbrainzclient.h"


!ENDIF 

SOURCE=.\musicbrainzconfig.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\musicbrainzconfig.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\musicbrainzconfig.obj"	"$(INTDIR)\musicbrainzconfig.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\musicbrainzdialog.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\musicbrainzdialog.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\musicbrainzdialog.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\musicbrainzdialog.obj"	"$(INTDIR)\musicbrainzdialog.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\musicbrainzdialog.h"


!ENDIF 

SOURCE=.\oggfile.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\oggfile.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\oggfile.obj"	"$(INTDIR)\oggfile.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\oggframelist.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\oggframelist.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\oggframelist.obj"	"$(INTDIR)\oggframelist.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\rendirdialog.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\rendirdialog.obj" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\rendirdialog.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\rendirdialog.obj"	"$(INTDIR)\rendirdialog.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h" ".\rendirdialog.h"


!ENDIF 

SOURCE=.\standardtags.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\standardtags.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\standardtags.obj"	"$(INTDIR)\standardtags.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\taggedfile.cpp

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\taggedfile.obj" : $(SOURCE) "$(INTDIR)" ".\filelist.h" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\taggedfile.obj"	"$(INTDIR)\taggedfile.sbr" : $(SOURCE) "$(INTDIR)" ".\filelist.h" "..\config.h"


!ENDIF 

SOURCE=.\vcedit.c

!IF  "$(CFG)" == "kid3 - Win32 Release"


"$(INTDIR)\vcedit.obj" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"


"$(INTDIR)\vcedit.obj"	"$(INTDIR)\vcedit.sbr" : $(SOURCE) "$(INTDIR)" "..\config.h"


!ENDIF 

SOURCE=.\commandstable.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\commandstable.h

".\moc_commandstable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_commandstable.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\commandstable.h

".\moc_commandstable.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_commandstable.cpp
<< 
	

!ENDIF 

SOURCE=.\filelist.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\filelist.h

".\moc_filelist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_filelist.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\filelist.h

".\moc_filelist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_filelist.cpp
<< 
	

!ENDIF 

SOURCE=.\formatbox.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\formatbox.h

".\moc_formatbox.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_formatbox.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\formatbox.h

".\moc_formatbox.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_formatbox.cpp
<< 
	

!ENDIF 

SOURCE=.\freedbclient.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\freedbclient.h

".\moc_freedbclient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_freedbclient.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\freedbclient.h

".\moc_freedbclient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_freedbclient.cpp
<< 
	

!ENDIF 

SOURCE=.\freedbdialog.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\freedbdialog.h

".\moc_freedbdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_freedbdialog.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\freedbdialog.h

".\moc_freedbdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_freedbdialog.cpp
<< 
	

!ENDIF 

SOURCE=.\id3form.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\id3form.h

".\moc_id3form.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_id3form.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\id3form.h

".\moc_id3form.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_id3form.cpp
<< 
	

!ENDIF 

SOURCE=.\importselector.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\importselector.h

".\moc_importselector.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_importselector.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\importselector.h

".\moc_importselector.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_importselector.cpp
<< 
	

!ENDIF 

SOURCE=.\kid3.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\kid3.h

".\moc_kid3.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_kid3.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\kid3.h

".\moc_kid3.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_kid3.cpp
<< 
	

!ENDIF 

SOURCE=.\mp3framelist.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\mp3framelist.h

".\moc_mp3framelist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_mp3framelist.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\mp3framelist.h

".\moc_mp3framelist.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_mp3framelist.cpp
<< 
	

!ENDIF 

SOURCE=.\musicbrainzclient.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\musicbrainzclient.h

".\moc_musicbrainzclient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_musicbrainzclient.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\musicbrainzclient.h

".\moc_musicbrainzclient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_musicbrainzclient.cpp
<< 
	

!ENDIF 

SOURCE=.\musicbrainzdialog.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\musicbrainzdialog.h

".\moc_musicbrainzdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_musicbrainzdialog.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\musicbrainzdialog.h

".\moc_musicbrainzdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_musicbrainzdialog.cpp
<< 
	

!ENDIF 

SOURCE=.\rendirdialog.h

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\rendirdialog.h

".\moc_rendirdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_rendirdialog.cpp
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\rendirdialog.h

".\moc_rendirdialog.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%QTDIR%\bin\moc.exe $(InputPath) -o .\moc_rendirdialog.cpp
<< 
	

!ENDIF 

SOURCE=.\config.mk

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\config.mk

"..\config.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%PERLDIR%\bin\perl.exe  -ne "print if (s/^([^#][^=\s]*)[\s=]+(.+)$/#define \1 \2/)" $(InputPath) >..\config.h
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\config.mk

"..\config.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	%PERLDIR%\bin\perl.exe  -ne "print if (s/^([^#][^=\s]*)[\s=]+(.+)$/#define \1 \2/)" $(InputPath) >..\config.h
<< 
	

!ENDIF 

SOURCE=.\kid3.h.qtonly

!IF  "$(CFG)" == "kid3 - Win32 Release"

InputPath=.\kid3.h.qtonly

".\kid3.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	copy kid3.h.qtonly kid3.h
<< 
	

!ELSEIF  "$(CFG)" == "kid3 - Win32 Debug"

InputPath=.\kid3.h.qtonly

".\kid3.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	copy kid3.h.qtonly kid3.h
<< 
	

!ENDIF 


!ENDIF 

