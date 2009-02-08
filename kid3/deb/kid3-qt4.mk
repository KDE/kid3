#!/usr/bin/make -f

QMAKE = qmake-qt4
DEB_CONFIGURE_PREFIX = /usr

build: kid3-qt4.build-stamp

kid3-qt/configure: kid3-qt/configure.in
	cd kid3-qt && autoconf

kid3-qt4.build-stamp: kid3-qt/configure
	mkdir kid3-qt4; \
	cd kid3-qt4; \
	../kid3-qt/configure --host=$(DEB_HOST_GNU_TYPE) --build=$(DEB_BUILD_GNU_TYPE) \
	            --prefix=$(DEB_CONFIGURE_PREFIX) --with-qmake=$(QMAKE); \
	cd ..; \
	$(MAKE) -C kid3-qt4

	touch kid3-qt4.build-stamp

clean: 
	[ ! -f kid3-qt4/Makefile ] || $(MAKE) -C kid3-qt4 distclean
	-rm -rf kid3-qt4.build-stamp kid3-qt4

install: build
	$(MAKE) -C kid3-qt4 install INSTALL_ROOT=$(CURDIR)/debian/kid3-qt

.PHONY: build clean install
