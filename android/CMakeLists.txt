include(qt-android-cmake/AddQtAndroidApk.cmake)
set(QT_ANDROID_APP_PACKAGE_NAME "net.sourceforge.kid3")
set(QT_ANDROID_APP_NAME "Kid3")
set(_androidPackageDir ${CMAKE_CURRENT_BINARY_DIR}/package)
configure_file(AndroidManifest.xml.in
               ${_androidPackageDir}/AndroidManifest.xml @ONLY)
add_custom_command(OUTPUT ${_androidPackageDir}/res/drawable/icon.png
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_androidPackageDir}/res
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_androidPackageDir}/res/drawable
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../src/app/hi48-apps-kid3.png
          ${_androidPackageDir}/res/drawable/icon.png
)
add_custom_target(android-package
  DEPENDS ${_androidPackageDir}/res/drawable/icon.png)
add_qt_android_apk(apk kid3-qml
  NAME Kid3
  PACKAGE_NAME "net.sourceforge.kid3"
  DEPENDS kid3-core kid3-gui
          musicbrainzimport discogsimport freedbimport amazonimport
          taglibmetadata kid3qml Qt5::Svg
  PACKAGE_SOURCES ${_androidPackageDir}
)
add_dependencies(apk android-package musicbrainzimport discogsimport
                 freedbimport amazonimport taglibmetadata kid3qml)
