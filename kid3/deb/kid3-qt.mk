#!/usr/bin/make -f

QMAKE = qmake-qt3
DEB_CONFIGURE_PREFIX = /usr

build: kid3-qt.build-stamp

kid3-qt/configure: kid3-qt/configure.in
	cd kid3-qt && autoconf

kid3-qt.build-stamp: kid3-qt/configure
	cd kid3-qt; \
	./configure --host=$(DEB_HOST_GNU_TYPE) --build=$(DEB_BUILD_GNU_TYPE) \
	            --prefix=$(DEB_CONFIGURE_PREFIX) --with-qmake=$(QMAKE); \
	cd ..
	$(MAKE) -C kid3-qt

	touch kid3-qt.build-stamp

clean: 
	[ ! -f kid3-qt/Makefile ] || $(MAKE) -C kid3-qt distclean
	-rm -rf kid3-qt.build-stamp kid3-qt/kid3 kid3-qt/doc kid3-qt/po

install: build
	$(MAKE) -C kid3-qt install INSTALL_ROOT=$(CURDIR)/debian/kid3-qt

.PHONY: build clean install
