# qmake project file

!include(config.pri) {
  error("config.pri not found")
}

CONFIG += release
TEMPLATE = subdirs
contains(CFG_LIBS, -ltag): SUBDIRS += kid3/taglibext
SUBDIRS += kid3 doc/en doc/de po
