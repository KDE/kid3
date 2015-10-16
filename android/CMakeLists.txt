include(qt-android-cmake/AddQtAndroidApk.cmake)
set(QT_ANDROID_APP_PACKAGE_NAME "net.sourceforge.kid3")
set(QT_ANDROID_APP_NAME "Kid3")
add_qt_android_apk(apk kid3-qml
  NAME Kid3
  PACKAGE_NAME "net.sourceforge.kid3"
  DEPENDS kid3-core kid3-gui qmlcommand musicbrainzimport discogsimport freedbimport amazonimport taglibmetadata kid3qml
)
