include /usr/share/cdbs/1/class/cmake.mk
include /usr/share/cdbs/1/rules/debhelper.mk
include /usr/share/cdbs/1/rules/patchsys-quilt.mk
include /usr/share/cdbs/1/rules/utils.mk

DEB_CONFIG_INSTALL_DIR ?= /usr/share/kde4/config

DEB_COMPRESS_EXCLUDE = .dcl .docbook -license .tag .sty .el
DEB_CMAKE_EXTRA_FLAGS += \
			-DCMAKE_BUILD_TYPE=Debian \
			$(KDE4-ENABLE-FINAL) \
			-DKDE4_BUILD_TESTS=false \
			-DKDE_DISTRIBUTION_TEXT="Kubuntu packages" \
			-DCMAKE_SKIP_RPATH=true \
			-DKDE4_USE_ALWAYS_FULL_RPATH=false \
			-DCONFIG_INSTALL_DIR=$(DEB_CONFIG_INSTALL_DIR) \
			-DDATA_INSTALL_DIR=/usr/share/kde4/apps \
			-DHTML_INSTALL_DIR=/usr/share/doc/kde4/HTML \
			-DKCFG_INSTALL_DIR=/usr/share/kde4/config.kcfg \
			-DLIB_INSTALL_DIR=/usr/lib \
			-DSYSCONF_INSTALL_DIR=/etc

# Set the one below to something else than 'yes' to disable linking 
# with --as-needed (on by default)
DEB_KDE_LINK_WITH_AS_NEEDED ?= yes
ifneq (,$(findstring yes, $(DEB_KDE_LINK_WITH_AS_NEEDED)))
	ifeq (,$(findstring no-as-needed, $(DEB_BUILD_OPTIONS)))
		DEB_KDE_LINK_WITH_AS_NEEDED := yes
		DEB_CMAKE_EXTRA_FLAGS += \
					-DCMAKE_SHARED_LINKER_FLAGS="-Wl,--no-undefined -Wl,--as-needed" \
					-DCMAKE_MODULE_LINKER_FLAGS="-Wl,--no-undefined -Wl,--as-needed" \
					-DCMAKE_EXE_LINKER_FLAGS="-Wl,--no-undefined -Wl,--as-needed"
	else
		DEB_KDE_LINK_WITH_AS_NEEDED := no
	endif
else
	DEB_KDE_LINK_WITH_AS_NEEDED := no
endif

#DEB_CMAKE_PREFIX = /usr/lib/kde4
DEB_DH_INSTALL_SOURCEDIR = debian/tmp
#DEB_DH_SHLIBDEPS_ARGS = -l/usr/lib/kde4/lib/
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
	#dpkg-parsechangelog | grep ^Distribution | grep -q 'experimental\|UNRELEASED'
endif
	rm -rf debian/man/out
	-rmdir debian/man
	rm -f debian/stamp-man-pages
	rm -f CMakeCache.txt


$(patsubst %,binary-install/%,$(DEB_PACKAGES)) :: binary-install/%:
	if test -x /usr/bin/dh_desktop; then dh_desktop -p$(cdbs_curpkg) $(DEB_DH_DESKTOP_ARGS); fi
	if test -e debian/$(cdbs_curpkg).lintian; then \
		install -p -D -m644 debian/$(cdbs_curpkg).lintian \
			debian/$(cdbs_curpkg)/usr/share/lintian/overrides/$(cdbs_curpkg); \
	fi
	if test -e debian/$(cdbs_curpkg).presubj; then \
		install -p -D -m644 debian/$(cdbs_curpkg).presubj \
			debian/$(cdbs_curpkg)/usr/share/bug/$(cdbs_curpkg)/presubj; \
	fi

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


# Process "sameVersionDep:" substvars
DH_SAMEVERSIONDEPS=debian/cdbs/dh_sameversiondeps
common-binary-predeb-arch common-binary-predeb-indep::
	@if [ ! -x "$(DH_SAMEVERSIONDEPS)" ]; then chmod a+x "$(DH_SAMEVERSIONDEPS)"; fi
	$(DH_SAMEVERSIONDEPS)
