qt_wrap_cpp(gui_GEN_MOC_SRCS
  widgets/abstractlistedit.h
  widgets/configtable.h
  widgets/formatbox.h
  widgets/filenameformatbox.h
  widgets/tagformatbox.h
  widgets/formatlistedit.h
  widgets/frametable.h
  widgets/imageviewer.h
  widgets/stringlistedit.h
  widgets/timeeventeditor.h
  widgets/timestampdelegate.h
  widgets/chaptereditor.h
  widgets/tableofcontentseditor.h
  widgets/subframeseditor.h
  widgets/picturelabel.h
  widgets/progresswidget.h
  widgets/frameitemdelegate.h
  widgets/tablemodeledit.h
  dialogs/batchimportdialog.h
  dialogs/batchimportsourcedialog.h
  dialogs/browsecoverartdialog.h
  dialogs/configdialogpages.h
  dialogs/downloaddialog.h
  dialogs/editframefieldsdialog.h
  dialogs/exportdialog.h
  dialogs/filterdialog.h
  dialogs/importdialog.h
  dialogs/servertrackimportdialog.h
  dialogs/numbertracksdialog.h
  dialogs/playlistdialog.h
  dialogs/rendirdialog.h
  dialogs/serverimportdialog.h
  dialogs/tagimportdialog.h
  dialogs/textimportdialog.h
  dialogs/findreplacedialog.h
  dialogs/playlisteditdialog.h
  forms/filelist.h
  forms/configurabletreeview.h
  forms/kid3form.h
  forms/basemainwindow.h
  forms/playlistview.h
  forms/sectionactions.h
  TARGET kid3-gui
)
if(HAVE_QTMULTIMEDIA)
  qt_wrap_cpp(qtmultimedia_GEN_MOC_SRCS
    widgets/playtoolbar.h
    forms/audioplayer.h
    forms/mprisinterface.h
    TARGET kid3-gui
  )
endif()

add_library(kid3-gui
  widgets/abstractlistedit.cpp
  widgets/comboboxdelegate.cpp
  widgets/configtable.cpp
  widgets/formatbox.cpp
  widgets/filenameformatbox.cpp
  widgets/tagformatbox.cpp
  widgets/formatlistedit.cpp
  widgets/frametable.cpp
  widgets/imageviewer.cpp
  widgets/picturelabel.cpp
  widgets/stringlistedit.cpp
  widgets/timeeventeditor.cpp
  widgets/enumdelegate.cpp
  widgets/eventcodedelegate.cpp
  widgets/timestampdelegate.cpp
  widgets/chaptereditor.cpp
  widgets/tableofcontentseditor.cpp
  widgets/subframeseditor.cpp
  widgets/progresswidget.cpp
  widgets/frameitemdelegate.cpp
  widgets/tablemodeledit.cpp
  widgets/tracknumbervalidator.cpp
  dialogs/batchimportdialog.cpp
  dialogs/batchimportsourcedialog.cpp
  dialogs/browsecoverartdialog.cpp
  dialogs/configdialogpages.cpp
  dialogs/contexthelp.cpp
  dialogs/downloaddialog.cpp
  dialogs/editframefieldsdialog.cpp
  dialogs/exportdialog.cpp
  dialogs/filterdialog.cpp
  dialogs/importdialog.cpp
  dialogs/servertrackimportdialog.cpp
  dialogs/numbertracksdialog.cpp
  dialogs/playlistdialog.cpp
  dialogs/rendirdialog.cpp
  dialogs/serverimportdialog.cpp
  dialogs/tagimportdialog.cpp
  dialogs/textimportdialog.cpp
  dialogs/findreplacedialog.cpp
  dialogs/playlisteditdialog.cpp
  dialogs/stringlisteditdialog.cpp
  forms/filelist.cpp
  forms/configurabletreeview.cpp
  forms/kid3form.cpp
  forms/basemainwindow.cpp
  forms/iplatformtools.cpp
  forms/playlistview.cpp
  forms/pixmapprovider.cpp
  forms/taggedfileiconprovider.cpp
  forms/guiplatformtools.cpp
  forms/sectionactions.cpp
)
if(HAVE_QTMULTIMEDIA)
  target_sources(kid3-gui PRIVATE
    widgets/playtoolbar.cpp
    forms/audioplayer.cpp
    forms/mprisinterface.cpp
  )
endif()
target_sources(kid3-gui PRIVATE ${gui_GEN_MOC_SRCS})
if(HAVE_QTMULTIMEDIA)
  target_sources(kid3-gui PRIVATE ${qtmultimedia_GEN_MOC_SRCS})
endif()

target_include_directories(kid3-gui PRIVATE widgets PUBLIC dialogs forms)

target_compile_definitions(kid3-gui PRIVATE KID3_BUILD_GUI_LIB)

target_link_libraries(kid3-gui kid3-core Qt${QT_VERSION_MAJOR}::Widgets)
if(HAVE_QTMULTIMEDIA)
  target_link_libraries(kid3-gui Qt${QT_VERSION_MAJOR}::Multimedia)
endif()

if(BUILD_SHARED_LIBS)
  install(TARGETS kid3-gui
    LIBRARY DESTINATION ${WITH_LIBDIR}
    RUNTIME DESTINATION ${WITH_BINDIR})
endif()
