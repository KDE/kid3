if(QT_VERSION_MAJOR LESS 6)
  set(KID3_ANDROID_MIN_SDK_VERSION 16)
  set(KID3_ANDROID_TARGET_SDK_VERSION 22)
  set(KID3_ANDROID_NAME "org.qtproject.qt5.android.bindings.QtApplication")
  set(FIX_KID3ACTIVITY "import fileinput; [print(line, end='') for line in fileinput.input()]")
  set(ANDROIDMANIFEST_XML_IN "AndroidManifest.xml.in")
else()
  set(KID3_ANDROID_MIN_SDK_VERSION 23)
  # To avoid scoped storage with API 29+, we need access to real files.
  # https://developer.android.com/about/versions/10/behavior-changes-10
  set(KID3_ANDROID_TARGET_SDK_VERSION 28)
  set(KID3_ANDROID_NAME "org.qtproject.qt.android.bindings.QtApplication")
  set(FIX_KID3ACTIVITY "import fileinput; \
    [print(line.replace('org.qtproject.qt5', 'org.qtproject.qt'), end='') \
    for line in fileinput.input()]")
  set(ANDROIDMANIFEST_XML_IN "AndroidManifest-qt6.xml.in")
endif()

set(QT_ANDROID_APP_PACKAGE_NAME "net.sourceforge.kid3")
set(QT_ANDROID_APP_NAME "Kid3")
set(_androidPackageDir ${CMAKE_CURRENT_BINARY_DIR}/package)
configure_file(${ANDROIDMANIFEST_XML_IN}
               ${_androidPackageDir}/AndroidManifest.xml @ONLY)
add_custom_command(OUTPUT ${_androidPackageDir}/src/net/sourceforge/kid3/Kid3Activity.java
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_androidPackageDir}/src/net/sourceforge/kid3
  COMMAND ${PYTHON_EXECUTABLE} -c "${FIX_KID3ACTIVITY}"
          ${CMAKE_CURRENT_SOURCE_DIR}/Kid3Activity.java
          > ${_androidPackageDir}/src/net/sourceforge/kid3/Kid3Activity.java VERBATIM
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Kid3Activity.java
)
set(_widths 32 48 64 128)
set(_densities ldpi mdpi hdpi xhdpi)
set(_icons)
list(LENGTH _widths _len)
math(EXPR _maxnr "${_len} - 1")
foreach(_nr RANGE ${_maxnr})
  list(GET _widths ${_nr} _width)
  list(GET _densities ${_nr} _dens)
  set(_icondir ${_androidPackageDir}/res/drawable-${_dens})
  set(_icon ${_icondir}/icon.png)
  list(APPEND _icons ${_icon})
  set(_png ${CMAKE_CURRENT_SOURCE_DIR}/../src/app/${_width}-apps-kid3.png)
  add_custom_command(OUTPUT ${_icon}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_icondir}
    COMMAND ${CMAKE_COMMAND} -E copy ${_png} ${_icon}
    DEPENDS ${_png}
  )
endforeach()

add_custom_target(android-package
  DEPENDS ${_icons}
          ${_androidPackageDir}/src/net/sourceforge/kid3/Kid3Activity.java)
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND ANDROID_KEYSTORE_PATH AND ANDROID_KEYSTORE_ALIAS)
  set(_keystorePathAlias ${ANDROID_KEYSTORE_PATH} ${ANDROID_KEYSTORE_ALIAS})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(_keystorePathAlias "UNSIGNED_RELEASE")
else()
  set(_keystorePathAlias)
endif()


if(QT_VERSION_MAJOR LESS 6)
  set(_deploymentDepends
      lib/libQt${QT_VERSION_MAJOR}Core.so lib/libQt${QT_VERSION_MAJOR}Xml.so lib/libQt${QT_VERSION_MAJOR}Network.so lib/libQt${QT_VERSION_MAJOR}Gui.so
      plugins/bearer/libqandroidbearer.so plugins/platforms/android/libqtforandroid.so plugins/iconengines/libqsvgicon.so
      plugins/imageformats/libqjpeg.so
      plugins/imageformats/libqsvg.so
      lib/libQt${QT_VERSION_MAJOR}Multimedia.so lib/libQt${QT_VERSION_MAJOR}Qml.so
      lib/libQt${QT_VERSION_MAJOR}Quick.so lib/libQt${QT_VERSION_MAJOR}Widgets.so lib/libQt${QT_VERSION_MAJOR}Svg.so
      lib/libQt${QT_VERSION_MAJOR}QuickControls2.so lib/libQt${QT_VERSION_MAJOR}QuickTemplates2.so lib/libQt${QT_VERSION_MAJOR}AndroidExtras.so
      qml/Qt/labs/folderlistmodel/libqmlfolderlistmodelplugin.so qml/Qt/labs/folderlistmodel/plugins.qmltypes qml/Qt/labs/folderlistmodel/qmldir
      qml/QtQuick/Layouts/libqquicklayoutsplugin.so qml/QtQuick/Layouts/plugins.qmltypes qml/QtQuick/Layouts/qmldir
      qml/QtQuick/Window.2/libwindowplugin.so qml/QtQuick/Window.2/plugins.qmltypes qml/QtQuick/Window.2/qmldir
      qml/QtQuick/Templates.2/libqtquicktemplates2plugin.so qml/QtQuick/Templates.2/plugins.qmltypes qml/QtQuick/Templates.2/qmldir
      qml/QtQuick.2/libqtquick2plugin.so qml/QtQuick.2/plugins.qmltypes qml/QtQuick.2/qmldir
      qml/QtQuick/Controls.2/libqtquickcontrols2plugin.so qml/QtQuick/Controls.2/plugins.qmltypes qml/QtQuick/Controls.2/qmldir
      qml/QtQuick/Controls.2/AbstractButton.qml qml/QtQuick/Controls.2/ApplicationWindow.qml qml/QtQuick/Controls.2/BusyIndicator.qml qml/QtQuick/Controls.2/Button.qml qml/QtQuick/Controls.2/ButtonGroup.qml qml/QtQuick/Controls.2/CheckBox.qml qml/QtQuick/Controls.2/CheckDelegate.qml qml/QtQuick/Controls.2/ComboBox.qml qml/QtQuick/Controls.2/Container.qml qml/QtQuick/Controls.2/Control.qml qml/QtQuick/Controls.2/DelayButton.qml qml/QtQuick/Controls.2/Dial.qml qml/QtQuick/Controls.2/Dialog.qml qml/QtQuick/Controls.2/DialogButtonBox.qml qml/QtQuick/Controls.2/Drawer.qml qml/QtQuick/Controls.2/Frame.qml qml/QtQuick/Controls.2/GroupBox.qml qml/QtQuick/Controls.2/ItemDelegate.qml qml/QtQuick/Controls.2/Label.qml qml/QtQuick/Controls.2/Menu.qml qml/QtQuick/Controls.2/MenuItem.qml qml/QtQuick/Controls.2/MenuSeparator.qml qml/QtQuick/Controls.2/Page.qml qml/QtQuick/Controls.2/PageIndicator.qml qml/QtQuick/Controls.2/Pane.qml qml/QtQuick/Controls.2/Popup.qml qml/QtQuick/Controls.2/ProgressBar.qml qml/QtQuick/Controls.2/RadioButton.qml qml/QtQuick/Controls.2/RadioDelegate.qml qml/QtQuick/Controls.2/RangeSlider.qml qml/QtQuick/Controls.2/RoundButton.qml qml/QtQuick/Controls.2/ScrollBar.qml qml/QtQuick/Controls.2/ScrollIndicator.qml qml/QtQuick/Controls.2/ScrollView.qml qml/QtQuick/Controls.2/Slider.qml qml/QtQuick/Controls.2/SpinBox.qml qml/QtQuick/Controls.2/StackView.qml qml/QtQuick/Controls.2/SwipeDelegate.qml qml/QtQuick/Controls.2/SwipeView.qml qml/QtQuick/Controls.2/Switch.qml qml/QtQuick/Controls.2/SwitchDelegate.qml qml/QtQuick/Controls.2/TabBar.qml qml/QtQuick/Controls.2/TabButton.qml qml/QtQuick/Controls.2/TextArea.qml qml/QtQuick/Controls.2/TextField.qml qml/QtQuick/Controls.2/ToolBar.qml qml/QtQuick/Controls.2/ToolButton.qml qml/QtQuick/Controls.2/ToolSeparator.qml qml/QtQuick/Controls.2/ToolTip.qml qml/QtQuick/Controls.2/Tumbler.qml
      qml/QtQuick/Controls.2/Material/libqtquickcontrols2materialstyleplugin.so qml/QtQuick/Controls.2/Material/plugins.qmltypes qml/QtQuick/Controls.2/Material/qmldir
      qml/QtQuick/Controls.2/Material/ApplicationWindow.qml qml/QtQuick/Controls.2/Material/BoxShadow.qml qml/QtQuick/Controls.2/Material/BusyIndicator.qml qml/QtQuick/Controls.2/Material/Button.qml qml/QtQuick/Controls.2/Material/CheckBox.qml qml/QtQuick/Controls.2/Material/CheckDelegate.qml qml/QtQuick/Controls.2/Material/CheckIndicator.qml qml/QtQuick/Controls.2/Material/ComboBox.qml qml/QtQuick/Controls.2/Material/CursorDelegate.qml qml/QtQuick/Controls.2/Material/DelayButton.qml qml/QtQuick/Controls.2/Material/Dial.qml qml/QtQuick/Controls.2/Material/Dialog.qml qml/QtQuick/Controls.2/Material/DialogButtonBox.qml qml/QtQuick/Controls.2/Material/Drawer.qml qml/QtQuick/Controls.2/Material/ElevationEffect.qml qml/QtQuick/Controls.2/Material/Frame.qml qml/QtQuick/Controls.2/Material/GroupBox.qml qml/QtQuick/Controls.2/Material/ItemDelegate.qml qml/QtQuick/Controls.2/Material/Label.qml qml/QtQuick/Controls.2/Material/Menu.qml qml/QtQuick/Controls.2/Material/MenuItem.qml qml/QtQuick/Controls.2/Material/MenuSeparator.qml qml/QtQuick/Controls.2/Material/Page.qml qml/QtQuick/Controls.2/Material/PageIndicator.qml qml/QtQuick/Controls.2/Material/Pane.qml qml/QtQuick/Controls.2/Material/Popup.qml qml/QtQuick/Controls.2/Material/ProgressBar.qml qml/QtQuick/Controls.2/Material/RadioButton.qml qml/QtQuick/Controls.2/Material/RadioDelegate.qml qml/QtQuick/Controls.2/Material/RadioIndicator.qml qml/QtQuick/Controls.2/Material/RangeSlider.qml qml/QtQuick/Controls.2/Material/RectangularGlow.qml qml/QtQuick/Controls.2/Material/RoundButton.qml qml/QtQuick/Controls.2/Material/ScrollBar.qml qml/QtQuick/Controls.2/Material/ScrollIndicator.qml qml/QtQuick/Controls.2/Material/Slider.qml qml/QtQuick/Controls.2/Material/SliderHandle.qml qml/QtQuick/Controls.2/Material/SpinBox.qml qml/QtQuick/Controls.2/Material/StackView.qml qml/QtQuick/Controls.2/Material/SwipeDelegate.qml qml/QtQuick/Controls.2/Material/SwipeView.qml qml/QtQuick/Controls.2/Material/Switch.qml qml/QtQuick/Controls.2/Material/SwitchDelegate.qml qml/QtQuick/Controls.2/Material/SwitchIndicator.qml qml/QtQuick/Controls.2/Material/TabBar.qml qml/QtQuick/Controls.2/Material/TabButton.qml qml/QtQuick/Controls.2/Material/TextArea.qml qml/QtQuick/Controls.2/Material/TextField.qml qml/QtQuick/Controls.2/Material/ToolBar.qml qml/QtQuick/Controls.2/Material/ToolButton.qml qml/QtQuick/Controls.2/Material/ToolSeparator.qml qml/QtQuick/Controls.2/Material/ToolTip.qml qml/QtQuick/Controls.2/Material/Tumbler.qml
  )
  if(EXISTS "${QT_ANDROID_QT_ROOT}/jar/QtAndroid-bundled.jar")
    set(_deploymentDepends jar/QtAndroid-bundled.jar jar/QtAndroidBearer-bundled.jar jar/QtMultimedia-bundled.jar ${_deploymentDepends})
  else()
    set(_deploymentDepends jar/QtAndroid.jar jar/QtAndroidBearer.jar jar/QtAndroidExtras.jar jar/QtMultimedia.jar ${_deploymentDepends})
  endif()
  foreach(_qmlFile CheckIndicator.qml RadioIndicator.qml SwitchIndicator.qml)
    if(EXISTS "${QT_ANDROID_QT_ROOT}/qml/QtQuick/Controls.2/${_qmlFile}")
      set(_deploymentDepends ${_deploymentDepends} qml/QtQuick/Controls.2/${_qmlFile})
    endif()
  endforeach()

  include(qt-android-cmake/AddQtAndroidApk.cmake)
  set(QT_ANDROID_BUILD_TOOLS_REVISION "23.0.1")
  add_qt_android_apk(apk kid3-qml
    NAME Kid3
    PACKAGE_NAME "net.sourceforge.kid3"
    DEPENDS kid3-core kid3-gui
            musicbrainzimport discogsimport freedbimport amazonimport
            taglibmetadata kid3qml Qt${QT_VERSION_MAJOR}::Svg
            ${ANDROID_EXTRA_LIBS_DIR}/libssl_1_1.so ${ANDROID_EXTRA_LIBS_DIR}/libcrypto_1_1.so
    DEPLOYMENT_DEPENDS ${_deploymentDepends}
    PACKAGE_SOURCES ${_androidPackageDir}
    KEYSTORE ${_keystorePathAlias}
  )

  # Unfortunately, androiddeployqt performs the library stripping before copying
  # the android-extra-libs (after DEPENDS in the command above). The external
  # libraries have to be stripped by the linker before the package is built.
  set_property(TARGET musicbrainzimport APPEND PROPERTY LINK_FLAGS_RELEASE -s)
  set_property(TARGET discogsimport APPEND PROPERTY LINK_FLAGS_RELEASE -s)
  set_property(TARGET freedbimport APPEND PROPERTY LINK_FLAGS_RELEASE -s)
  set_property(TARGET amazonimport APPEND PROPERTY LINK_FLAGS_RELEASE -s)
  set_property(TARGET taglibmetadata APPEND PROPERTY LINK_FLAGS_RELEASE -s)
  set_property(TARGET kid3qml APPEND PROPERTY LINK_FLAGS_RELEASE -s)
  set_property(TARGET kid3-core APPEND PROPERTY LINK_FLAGS_RELEASE -s)
  set_property(TARGET kid3-gui APPEND PROPERTY LINK_FLAGS_RELEASE -s)

  add_dependencies(apk android-package musicbrainzimport discogsimport
                   freedbimport amazonimport taglibmetadata kid3qml)
else(QT_VERSION_MAJOR LESS 6)
  set(_deploymentDepends
      jar/Qt${QT_VERSION_MAJOR}Android.jar jar/Qt${QT_VERSION_MAJOR}AndroidNetwork.jar jar/Qt${QT_VERSION_MAJOR}AndroidNetworkInformationBackend.jar jar/Qt${QT_VERSION_MAJOR}AndroidMultimedia.jar
      lib/libQt${QT_VERSION_MAJOR}Core_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}Xml_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}Network_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}Gui_armeabi-v7a.so
      lib/libQt${QT_VERSION_MAJOR}OpenGL_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}QmlModels_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}QuickControls2Impl_armeabi-v7a.so
      lib/libQt${QT_VERSION_MAJOR}LabsFolderListModel_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}QuickLayouts_armeabi-v7a.so
      plugins/networkinformation/libplugins_networkinformation_qandroidnetworkinformation_armeabi-v7a.so plugins/platforms/libplugins_platforms_qtforandroid_armeabi-v7a.so plugins/iconengines/libplugins_iconengines_qsvgicon_armeabi-v7a.so
      plugins/imageformats/libplugins_imageformats_qjpeg_armeabi-v7a.so
      plugins/imageformats/libplugins_imageformats_qsvg_armeabi-v7a.so
      lib/libQt${QT_VERSION_MAJOR}Multimedia_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}Qml_armeabi-v7a.so
      lib/libQt${QT_VERSION_MAJOR}Quick_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}Widgets_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}Svg_armeabi-v7a.so
      lib/libQt${QT_VERSION_MAJOR}QuickControls2_armeabi-v7a.so lib/libQt${QT_VERSION_MAJOR}QuickTemplates2_armeabi-v7a.so
      lib/libQt${QT_VERSION_MAJOR}QmlWorkerScript_armeabi-v7a.so
      qml/Qt/labs/folderlistmodel/libqml_Qt_labs_folderlistmodel_qmlfolderlistmodelplugin_armeabi-v7a.so qml/Qt/labs/folderlistmodel/plugins.qmltypes qml/Qt/labs/folderlistmodel/qmldir
      qml/QtQml/WorkerScript/libqml_QtQml_WorkerScript_workerscriptplugin_armeabi-v7a.so qml/QtQml/WorkerScript/plugins.qmltypes qml/QtQml/WorkerScript/qmldir
      qml/QtQuick/Layouts/libqml_QtQuick_Layouts_qquicklayoutsplugin_armeabi-v7a.so qml/QtQuick/Layouts/plugins.qmltypes qml/QtQuick/Layouts/qmldir
      qml/QtQuick/Window/libqml_QtQuick_Window_quickwindowplugin_armeabi-v7a.so qml/QtQuick/Window/qmldir qml/QtQuick/Window/quickwindow.qmltypes
      qml/QtQuick/Templates/libqml_QtQuick_Templates_qtquicktemplates2plugin_armeabi-v7a.so qml/QtQuick/Templates/plugins.qmltypes qml/QtQuick/Templates/qmldir
      qml/QtQuick/libqml_QtQuick_qtquick2plugin_armeabi-v7a.so qml/QtQuick/plugins.qmltypes qml/QtQuick/qmldir
      qml/QtQuick/Controls/libqml_QtQuick_Controls_qtquickcontrols2plugin_armeabi-v7a.so qml/QtQuick/Controls/plugins.qmltypes qml/QtQuick/Controls/qmldir
      qml/QtQuick/Controls/Basic/libqml_QtQuick_Controls_Basic_qtquickcontrols2basicstyleplugin_armeabi-v7a.so qml/QtQuick/Controls/Basic/plugins.qmltypes qml/QtQuick/Controls/Basic/qmldir
      qml/QtQuick/Controls/Basic/AbstractButton.qml qml/QtQuick/Controls/Basic/ApplicationWindow.qml qml/QtQuick/Controls/Basic/BusyIndicator.qml qml/QtQuick/Controls/Basic/Button.qml qml/QtQuick/Controls/Basic/ButtonGroup.qml qml/QtQuick/Controls/Basic/CheckBox.qml qml/QtQuick/Controls/Basic/CheckDelegate.qml qml/QtQuick/Controls/Basic/ComboBox.qml qml/QtQuick/Controls/Basic/Container.qml qml/QtQuick/Controls/Basic/Control.qml qml/QtQuick/Controls/Basic/DelayButton.qml qml/QtQuick/Controls/Basic/Dial.qml qml/QtQuick/Controls/Basic/Dialog.qml qml/QtQuick/Controls/Basic/DialogButtonBox.qml qml/QtQuick/Controls/Basic/Drawer.qml qml/QtQuick/Controls/Basic/Frame.qml qml/QtQuick/Controls/Basic/GroupBox.qml qml/QtQuick/Controls/Basic/ItemDelegate.qml qml/QtQuick/Controls/Basic/Label.qml qml/QtQuick/Controls/Basic/Menu.qml qml/QtQuick/Controls/Basic/MenuItem.qml qml/QtQuick/Controls/Basic/MenuSeparator.qml qml/QtQuick/Controls/Basic/Page.qml qml/QtQuick/Controls/Basic/PageIndicator.qml qml/QtQuick/Controls/Basic/Pane.qml qml/QtQuick/Controls/Basic/Popup.qml qml/QtQuick/Controls/Basic/ProgressBar.qml qml/QtQuick/Controls/Basic/RadioButton.qml qml/QtQuick/Controls/Basic/RadioDelegate.qml qml/QtQuick/Controls/Basic/RangeSlider.qml qml/QtQuick/Controls/Basic/RoundButton.qml qml/QtQuick/Controls/Basic/ScrollBar.qml qml/QtQuick/Controls/Basic/ScrollIndicator.qml qml/QtQuick/Controls/Basic/ScrollView.qml qml/QtQuick/Controls/Basic/Slider.qml qml/QtQuick/Controls/Basic/SpinBox.qml qml/QtQuick/Controls/Basic/StackView.qml qml/QtQuick/Controls/Basic/SwipeDelegate.qml qml/QtQuick/Controls/Basic/SwipeView.qml qml/QtQuick/Controls/Basic/Switch.qml qml/QtQuick/Controls/Basic/SwitchDelegate.qml qml/QtQuick/Controls/Basic/TabBar.qml qml/QtQuick/Controls/Basic/TabButton.qml qml/QtQuick/Controls/Basic/TextArea.qml qml/QtQuick/Controls/Basic/TextField.qml qml/QtQuick/Controls/Basic/ToolBar.qml qml/QtQuick/Controls/Basic/ToolButton.qml qml/QtQuick/Controls/Basic/ToolSeparator.qml qml/QtQuick/Controls/Basic/ToolTip.qml qml/QtQuick/Controls/Basic/Tumbler.qml
      qml/QtQuick/Controls/Material/libqml_QtQuick_Controls_Material_qtquickcontrols2materialstyleplugin_armeabi-v7a.so qml/QtQuick/Controls/Material/plugins.qmltypes qml/QtQuick/Controls/Material/qmldir
      qml/QtQuick/Controls/Material/ApplicationWindow.qml qml/QtQuick/Controls/Material/impl/BoxShadow.qml qml/QtQuick/Controls/Material/BusyIndicator.qml qml/QtQuick/Controls/Material/Button.qml qml/QtQuick/Controls/Material/CheckBox.qml qml/QtQuick/Controls/Material/CheckDelegate.qml qml/QtQuick/Controls/Material/impl/CheckIndicator.qml qml/QtQuick/Controls/Material/ComboBox.qml qml/QtQuick/Controls/Material/impl/CursorDelegate.qml qml/QtQuick/Controls/Material/DelayButton.qml qml/QtQuick/Controls/Material/Dial.qml qml/QtQuick/Controls/Material/Dialog.qml qml/QtQuick/Controls/Material/DialogButtonBox.qml qml/QtQuick/Controls/Material/Drawer.qml qml/QtQuick/Controls/Material/impl/ElevationEffect.qml qml/QtQuick/Controls/Material/Frame.qml qml/QtQuick/Controls/Material/GroupBox.qml qml/QtQuick/Controls/Material/ItemDelegate.qml qml/QtQuick/Controls/Material/Label.qml qml/QtQuick/Controls/Material/Menu.qml qml/QtQuick/Controls/Material/MenuItem.qml qml/QtQuick/Controls/Material/MenuSeparator.qml qml/QtQuick/Controls/Material/Page.qml qml/QtQuick/Controls/Material/PageIndicator.qml qml/QtQuick/Controls/Material/Pane.qml qml/QtQuick/Controls/Material/Popup.qml qml/QtQuick/Controls/Material/ProgressBar.qml qml/QtQuick/Controls/Material/RadioButton.qml qml/QtQuick/Controls/Material/RadioDelegate.qml qml/QtQuick/Controls/Material/impl/RadioIndicator.qml qml/QtQuick/Controls/Material/RangeSlider.qml qml/QtQuick/Controls/Material/impl/RectangularGlow.qml qml/QtQuick/Controls/Material/RoundButton.qml qml/QtQuick/Controls/Material/ScrollBar.qml qml/QtQuick/Controls/Material/ScrollIndicator.qml qml/QtQuick/Controls/Material/Slider.qml qml/QtQuick/Controls/Material/impl/SliderHandle.qml qml/QtQuick/Controls/Material/SpinBox.qml qml/QtQuick/Controls/Material/StackView.qml qml/QtQuick/Controls/Material/SwipeDelegate.qml qml/QtQuick/Controls/Material/SwipeView.qml qml/QtQuick/Controls/Material/Switch.qml qml/QtQuick/Controls/Material/SwitchDelegate.qml qml/QtQuick/Controls/Material/impl/SwitchIndicator.qml qml/QtQuick/Controls/Material/TabBar.qml qml/QtQuick/Controls/Material/TabButton.qml qml/QtQuick/Controls/Material/TextArea.qml qml/QtQuick/Controls/Material/TextField.qml qml/QtQuick/Controls/Material/ToolBar.qml qml/QtQuick/Controls/Material/ToolButton.qml qml/QtQuick/Controls/Material/ToolSeparator.qml qml/QtQuick/Controls/Material/ToolTip.qml qml/QtQuick/Controls/Material/Tumbler.qml
      qml/QtQuick/Controls/Material/impl/libqml_QtQuick_Controls_Material_impl_qtquickcontrols2materialstyleimplplugin_armeabi-v7a.so qml/QtQuick/Controls/Material/impl/plugins.qmltypes qml/QtQuick/Controls/Material/impl/qmldir
  )

  qt_add_executable(kid3apk MANUAL_FINALIZATION)
  target_link_libraries(kid3apk PRIVATE kid3-qml Qt${QT_VERSION_MAJOR}::Svg)

  # qt_add_qml_module is needed to have the QML plugins included
  set(_qmlFiles ${QMLAPP_QML_SOURCES})
  list(TRANSFORM _qmlFiles PREPEND "../src/qml/")
  qt_add_qml_module(kid3apk
      URI net.sourceforge.kid3
      VERSION 1.1
      QML_FILES ${_qmlFiles}
      NO_PLUGIN_OPTIONAL
      NO_GENERATE_PLUGIN_SOURCE
      NO_GENERATE_QMLTYPES
      NO_GENERATE_QMLDIR
      NO_LINT
      NO_CACHEGEN
      NO_RESOURCE_TARGET_PATH
  )

  add_dependencies(kid3apk android-package musicbrainzimport discogsimport
                   freedbimport amazonimport taglibmetadata kid3qml)

  cmake_policy(SET CMP0026 OLD) # allow use of the LOCATION target property
  set(EXTRA_LIBS)
  foreach(LIB kid3apk kid3-qml kid3-core kid3-gui musicbrainzimport discogsimport
      freedbimport amazonimport taglibmetadata kid3qml
      ${ANDROID_EXTRA_LIBS_DIR}/libssl_1_1.so
      ${ANDROID_EXTRA_LIBS_DIR}/libcrypto_1_1.so)
    if(TARGET ${LIB})
      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        get_property(LIB_PATH TARGET ${LIB} PROPERTY DEBUG_LOCATION)
      else()
        get_property(LIB_PATH TARGET ${LIB} PROPERTY LOCATION)
      endif()
      set(LIB ${LIB_PATH})
    endif()
    if(EXTRA_LIBS)
      set(EXTRA_LIBS "${EXTRA_LIBS},${LIB}")
    else()
      set(EXTRA_LIBS "${LIB}")
    endif()
  endforeach()

  set_target_properties(kid3apk PROPERTIES
    QT_ANDROID_DEPLOYMENT_DEPENDENCIES "${_deploymentDepends}"
    QT_ANDROID_EXTRA_LIBS "${EXTRA_LIBS}"
    QT_ANDROID_MIN_SDK_VERSION "${KID3_ANDROID_MIN_SDK_VERSION}"
    QT_ANDROID_PACKAGE_SOURCE_DIR "${_androidPackageDir}"
    QT_ANDROID_TARGET_SDK_VERSION "${KID3_ANDROID_TARGET_SDK_VERSION}"
    QT_ANDROID_VERSION_NAME "${QT_ANDROID_APP_VERSION}"
    QT_ANDROID_VERSION_CODE "${QT_ANDROID_APP_VERSION_CODE}"
  )

  # To sign the package when building the apk target, the enviroment variables
  # QT_ANDROID_KEYSTORE_PATH and QT_ANDROID_KEYSTORE_ALIAS must be set.
  # If QT_ANDROID_KEYSTORE_STORE_PASS is not set, the user has to enter the
  # password.
  if(CMAKE_BUILD_TYPE STREQUAL "Release" AND ANDROID_KEYSTORE_PATH AND ANDROID_KEYSTORE_ALIAS)
    set(QT_ANDROID_SIGN_APK ON)
  endif()

  qt_finalize_target(kid3apk)
endif(QT_VERSION_MAJOR LESS 6)
