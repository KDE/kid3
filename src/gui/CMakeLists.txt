qt5_wrap_cpp(gui_GEN_MOC_SRCS
  widgets/abstractlistedit.h
  widgets/configtable.h
  widgets/formatbox.h
  widgets/filenameformatbox.h
  widgets/tagformatbox.h
  widgets/formatlistedit.h
  widgets/frametable.h
  widgets/imageviewer.h
  widgets/playtoolbar.h
  widgets/stringlistedit.h
  widgets/timeeventeditor.h
  widgets/timestampdelegate.h
  widgets/chaptereditor.h
  widgets/tableofcontentseditor.h
  widgets/subframeseditor.h
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
  TARGET kid3-gui
)

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
  widgets/playtoolbar.cpp
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
  ${gui_GEN_MOC_SRCS}
)

target_include_directories(kid3-gui PRIVATE widgets PUBLIC dialogs forms)

target_compile_definitions(kid3-gui PRIVATE KID3_BUILD_GUI_LIB)

set_target_properties(kid3-gui PROPERTIES VERSION ${KID3_VERSION})
target_link_libraries(kid3-gui kid3-core Qt5::Widgets)

if(BUILD_SHARED_LIBS)
  install(TARGETS kid3-gui
    LIBRARY DESTINATION ${WITH_LIBDIR}
    RUNTIME DESTINATION ${WITH_BINDIR})
endif()
