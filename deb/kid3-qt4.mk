#!/usr/bin/make -f

# These are used for cross-compiling and for saving the configure script
# from having to guess our platform (since we know it already)
DEB_HOST_GNU_TYPE   ?= $(shell dpkg-architecture -qDEB_HOST_GNU_TYPE)
DEB_BUILD_GNU_TYPE  ?= $(shell dpkg-architecture -qDEB_BUILD_GNU_TYPE)

ifneq (,$(filter noopt,$(DEB_BUILD_OPTIONS)))
DEB_CMAKE_EXTRA_FLAGS += -DCMAKE_BUILD_TYPE=Debug
else
DEB_CMAKE_EXTRA_FLAGS += -DCMAKE_BUILD_TYPE=Release
endif

DEB_CMAKE_PREFIX ?= /usr

DEB_CMAKE_EXTRA_FLAGS += \
			-DWITH_KDE=OFF \
			-DWITH_DATAROOTDIR=share \
			-DWITH_DOCDIR=share/doc/kid3-qt \
			-DWITH_TRANSLATIONSDIR=share/kid3-qt/translations \
			-DWITH_BINDIR=bin

CMAKE = cmake
DEB_CMAKE_INSTALL_PREFIX = $(DEB_CMAKE_PREFIX)
DEB_CMAKE_NORMAL_ARGS = -DCMAKE_INSTALL_PREFIX="$(DEB_CMAKE_INSTALL_PREFIX)" -DCMAKE_C_COMPILER:FILEPATH="$(CC)" -DCMAKE_CXX_COMPILER:FILEPATH="$(CXX)" -DCMAKE_C_FLAGS="$(CFLAGS)" -DCMAKE_CXX_FLAGS="$(CXXFLAGS)" -DCMAKE_SKIP_RPATH=ON -DCMAKE_VERBOSE_MAKEFILE=ON

build: kid3-qt4.build-stamp

kid3-qt4.build-stamp:
	mkdir kid3-qt4; \
	cd kid3-qt4; \
	$(CMAKE) .. $(DEB_CMAKE_NORMAL_ARGS) $(DEB_CMAKE_EXTRA_FLAGS); \
	cd ..; \
	$(MAKE) -C kid3-qt4

	touch kid3-qt4.build-stamp

clean: 
	[ ! -f kid3-qt4/Makefile ] || $(MAKE) -C kid3-qt4 clean
	-rm -rf kid3-qt4.build-stamp kid3-qt4

install: build
	$(MAKE) -C kid3-qt4 install DESTDIR=$(CURDIR)/debian/kid3-qt

.PHONY: build clean install
