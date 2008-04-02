# until bug 377524 resolved.
include debian/cdbs/cmake.mk
include /usr/share/cdbs/1/rules/debhelper.mk
include /usr/share/cdbs/1/rules/patchsys-quilt.mk
# until bug #423394 resolved.
include debian/cdbs/utils.mk

DEB_COMPRESS_EXCLUDE = .dcl .docbook -license .tag .sty .el
DEB_CMAKE_EXTRA_FLAGS += \
			$(DEB_CMAKE_DEBUG_FLAGS) \
			$(KDE4-ENABLE-FINAL) \
			-DKDE4_BUILD_TESTS=true \
			-DKDE_DISTRIBUTION_TEXT="Kubuntu packages" \
			-DCONFIG_INSTALL_DIR=$(DEB_CMAKE_PREFIX)/etc/kde4 \
			-DDATA_INSTALL_DIR=$(DEB_CMAKE_PREFIX)/share/kde4/apps \
			-DHTML_INSTALL_DIR=$(DEB_CMAKE_PREFIX)/share/doc/kde4/HTML \
			-DKCFG_INSTALL_DIR=$(DEB_CMAKE_PREFIX)/share/kde4/config.kcfg \
			-DLIB_INSTALL_DIR=$(DEB_CMAKE_PREFIX)/lib \
			-DSYSCONF_INSTALL_DIR=$(DEB_CMAKE_PREFIX)/etc

DEB_CMAKE_PREFIX = /usr/lib/kde4
DEB_DH_INSTALL_ARGS = --sourcedir=debian/tmp
DEB_DH_SHLIBDEPS_ARGS = -l/usr/lib/kde4/lib/
DEB_KDE_ENABLE_FINAL ?=
#DEB_MAKE_ENVVARS += XDG_CONFIG_DIRS=/etc/xdg XDG_DATA_DIRS=/usr/share
#DEB_STRIP_EXCLUDE = so

ifeq (,$(findstring noopt,$(DEB_BUILD_OPTIONS)))
    cdbs_treat_me_gently_arches := arm m68k alpha ppc64 armel armeb
    ifeq (,$(filter $(DEB_HOST_ARCH_CPU),$(cdbs_treat_me_gently_arches)))
        KDE4-ENABLE-FINAL = $(if $(DEB_KDE_ENABLE_FINAL),-DKDE4_ENABLE_FINAL=true,)
    else
        KDE4-ENABLE-FINAL =
    endif
endif

ifeq (,$(findstring noopt,$(DEB_BUILD_OPTIONS)))
	#no optimizations, full debug
       DEB_CMAKE_DEBUG_FLAGS = -DCMAKE_BUILD_TYPE=debugfull
else
	#This is around -O2 -g
       DEB_CMAKE_DEBUG_FLAGS = -DCMAKE_BUILD_TYPE=relwithdebinfo
endif

common-build-arch:: debian/stamp-man-pages
debian/stamp-man-pages:
	if ! test -d debian/man/out; then mkdir -p debian/man/out; fi
	for f in $$(find debian/man -name '*.sgml'); do \
		docbook-to-man $$f > debian/man/out/`basename $$f .sgml`.1; \
	done
	for f in $$(find debian/man -name '*.man'); do \
		soelim -I debian/man $$f \
		> debian/man/out/`basename $$f .man`.`head -n1 $$f | awk '{print $$NF}'`; \
	done
	touch debian/stamp-man-pages

clean::
ifndef THIS_SHOULD_GO_TO_UNSTABLE
	#guard against experimental uploads to unstable
	#dpkg-parsechangelog | grep ^Distribution | grep -q experimental
endif
	rm -rf debian/man/out
	-rmdir debian/man
	rm -f debian/stamp-man-pages
	rm -f CMakeCache.txt

binary-install/$(DEB_SOURCE_PACKAGE)-doc-html::
	set -e; \
	for doc in `cd $(DEB_DESTDIR)/usr/share/doc/kde/HTML/en; find . -name index.docbook`; do \
		pkg=$${doc%/index.docbook}; pkg=$${pkg#./}; \
		echo Building $$pkg HTML docs...; \
		mkdir -p $(CURDIR)/debian/$(DEB_SOURCE_PACKAGE)-doc-html/usr/share/doc/kde/HTML/en/$$pkg; \
		cd $(CURDIR)/debian/$(DEB_SOURCE_PACKAGE)-doc-html/usr/share/doc/kde/HTML/en/$$pkg; \
		meinproc4 $(DEB_DESTDIR)/usr/share/doc/kde/HTML/en/$$pkg/index.docbook; \
	done
	for pkg in $(DOC_HTML_PRUNE) ; do \
		rm -rf debian/$(DEB_SOURCE_PACKAGE)-doc-html/usr/share/doc/kde/HTML/en/$$pkg; \
	done

