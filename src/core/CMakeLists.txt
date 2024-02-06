option(WITH_QT_PRIVATE_HEADERS "Use private Qt headers" OFF)

qt_wrap_cpp(core_GEN_MOC_SRCS
  utils/debugutils.h
  config/batchimportsourcesmodel.h
  config/batchimportconfig.h
  config/filterconfig.h
  config/formatconfig.h
  config/generalconfig.h
  config/importconfig.h
  config/exportconfig.h
  config/tagconfig.h
  config/fileconfig.h
  config/rendirconfig.h
  config/numbertracksconfig.h
  config/useractionsconfig.h
  config/guiconfig.h
  config/networkconfig.h
  config/playlistconfig.h
  config/serverimporterconfig.h
  config/findreplaceconfig.h
  config/mainwindowconfig.h
  config/starratingmappingsmodel.h
  tags/frame.h
  tags/framenotice.h
  import/batchimporter.h
  import/httpclient.h
  import/importclient.h
  import/serverimporter.h
  import/servertrackimporter.h
  model/commandstablemodel.h
  model/dirrenamer.h
  model/downloadclient.h
  model/externalprocess.h
  model/filefilter.h
  model/fileproxymodel.h
  model/fileproxymodeliterator.h
  model/bidirfileproxymodeliterator.h
  model/framelist.h
  model/frametablemodel.h
  model/kid3application.h
  model/trackdatamodel.h
  model/tagsearcher.h
  model/timeeventmodel.h
  model/taggedfileselection.h
  model/genremodel.h
  model/frameeditorobject.h
  model/frameobjectmodel.h
  model/playlistmodel.h
  model/proxyitemselectionmodel.h
  model/filesystemmodel.h
  model/fileinfogatherer_p.h
  model/standardtablemodel.h
  model/taggedfilesystemmodel.h
  TARGET kid3-core
)
if(HAVE_QTDBUS)
  qt_wrap_cpp(qtdbus_GEN_MOC_SRCS
    model/scriptinterface.h
    TARGET kid3-core
  )
endif()
if(ANDROID)
  qt_wrap_cpp(android_GEN_MOC_SRCS
    utils/androidutils.h
    TARGET kid3-core
  )
endif()

add_library(kid3-core
  utils/debugutils.cpp
  utils/saferename.cpp
  utils/loadtranslation.cpp
  utils/icoreplatformtools.cpp
  utils/coreplatformtools.cpp
  config/batchimportconfig.cpp
  config/batchimportprofile.cpp
  config/batchimportsourcesmodel.cpp
  config/configstore.cpp
  config/filterconfig.cpp
  config/formatconfig.cpp
  config/generalconfig.cpp
  config/importconfig.cpp
  config/exportconfig.cpp
  config/tagconfig.cpp
  config/fileconfig.cpp
  config/rendirconfig.cpp
  config/numbertracksconfig.cpp
  config/useractionsconfig.cpp
  config/guiconfig.cpp
  config/networkconfig.cpp
  config/playlistconfig.cpp
  config/serverimporterconfig.cpp
  config/isettings.cpp
  config/kid3settings.cpp
  config/findreplaceconfig.cpp
  config/mainwindowconfig.cpp
  config/starratingmappingsmodel.cpp
  tags/attributedata.cpp
  tags/genres.cpp
  tags/formatreplacer.cpp
  tags/frame.cpp
  tags/framenotice.cpp
  tags/pictureframe.cpp
  tags/taggedfile.cpp
  tags/itaggedfilefactory.cpp
  tags/trackdata.cpp
  export/playlistcreator.cpp
  export/textexporter.cpp
  import/batchimporter.cpp
  import/httpclient.cpp
  import/importclient.cpp
  import/importparser.cpp
  import/iserverimporterfactory.cpp
  import/iservertrackimporterfactory.cpp
  import/serverimporter.cpp
  import/servertrackimporter.cpp
  import/textimporter.cpp
  import/trackdatamatcher.cpp
  model/iabortable.cpp
  model/commandformatreplacer.cpp
  model/commandstablemodel.cpp
  model/configtablemodel.cpp
  model/dirproxymodel.cpp
  model/dirrenamer.cpp
  model/downloadclient.cpp
  model/expressionparser.cpp
  model/externalprocess.cpp
  model/filefilter.cpp
  model/fileproxymodel.cpp
  model/fileproxymodeliterator.cpp
  model/bidirfileproxymodeliterator.cpp
  model/framelist.cpp
  model/frametablemodel.cpp
  model/iframeeditor.cpp
  model/kid3application.cpp
  model/modeliterator.cpp
  model/coretaggedfileiconprovider.cpp
  model/texttablemodel.cpp
  model/trackdatamodel.cpp
  model/checkablestringlistmodel.cpp
  model/tagsearcher.cpp
  model/timeeventmodel.cpp
  model/eventtimingcode.cpp
  model/taggedfileselection.cpp
  model/genremodel.cpp
  model/frameeditorobject.cpp
  model/frameobjectmodel.cpp
  model/iusercommandprocessor.cpp
  model/playlistmodel.cpp
  model/proxyitemselectionmodel.cpp
  model/filesystemmodel.cpp
  model/fileinfogatherer.cpp
  model/abstractfiledecorationprovider.cpp
  model/standardtablemodel.cpp
  model/taggedfilesystemmodel.cpp
)
if(HAVE_QTDBUS)
  target_sources(kid3-core PRIVATE model/scriptinterface.cpp)
endif()
if(ANDROID)
  target_sources(kid3-core PRIVATE utils/androidutils.cpp)
endif()
target_sources(kid3-core PRIVATE ${core_GEN_MOC_SRCS})
if(HAVE_QTDBUS)
  target_sources(kid3-core PRIVATE ${qtdbus_GEN_MOC_SRCS})
endif()
if(ANDROID)
  target_sources(kid3-core PRIVATE ${android_GEN_MOC_SRCS})
endif()

target_include_directories(kid3-core PUBLIC utils config tags export import model)

target_compile_definitions(kid3-core
  PRIVATE KID3_BUILD_CORE_LIB
  PUBLIC QT_ASCII_CAST_WARNINGS QT_NO_CAST_TO_ASCII QT_NO_URL_CAST_FROM_STRING QT_STRICT_ITERATORS QT_NO_EXCEPTIONS QT_NO_STL
)

target_link_libraries(kid3-core PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Xml)
if(WITH_QT_PRIVATE_HEADERS)
  target_compile_definitions(kid3-core PRIVATE USE_QT_PRIVATE_HEADERS)
  target_link_libraries(kid3-core PRIVATE Qt${QT_VERSION_MAJOR}::CorePrivate)
endif()
if(WITH_DBUS)
  target_link_libraries(kid3-core PUBLIC Qt${QT_VERSION_MAJOR}::DBus)
endif()
if(PLUGIN_LIBRARIES)
  target_link_libraries(kid3-core PUBLIC ${PLUGIN_LIBRARIES})
endif()
if(APPLE)
  target_link_libraries(kid3-core PUBLIC "-framework CoreServices")
endif()
if(ANDROID AND QT_VERSION_MAJOR LESS 6)
  target_link_libraries(kid3-core PUBLIC Qt${QT_VERSION_MAJOR}::AndroidExtras)
endif()

if(BUILD_SHARED_LIBS)
  install(TARGETS kid3-core
    LIBRARY DESTINATION ${WITH_LIBDIR}
    RUNTIME DESTINATION ${WITH_BINDIR})
endif()

if(BUILD_KDE_APP)
  install(FILES model/org.kde.Kid3.xml
          DESTINATION ${WITH_DBUSDIR})
endif()
