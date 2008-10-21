include debian/cdbs/versions.mk
debian/control: debian/control.tmp
	mv debian/control.tmp debian/control

debian/control.tmp: update-versions

update-versions: debian/control.in
	sed "s/CDBS_MIN_VER/$(CDBS_MIN_VER)/;\
	     s/QUILT_MIN_VER/$(QUILT_MIN_VER)/;\
	     s/CMAKE_MIN_VER/$(CMAKE_MIN_VER)/;\
	     s/KDELIBS_VERSION/$(KDELIBS_VERSION)/;\
	     s/KDELIBS_UPSTREAM_VERSION/$(KDELIBS_UPSTREAM_VERSION)/;\
	     s/KDELIBS_SOURCE_VERSION/$(KDELIBS_SOURCE_VERSION)/;\
	     s/KDEPIMLIBS_VERSION/$(KDEPIMLIBS_VERSION)/;\
	     s/KDEPIMLIBS_SOURCE_VERSION/$(KDEPIMLIBS_SOURCE_VERSION)/;\
	     s/KDEPIMLIBS_UPSTREAM_VERSION/$(KDEPIMLIBS_UPSTREAM_VERSION)/;\
	     " debian/control.in > debian/control.tmp

.PHONY: update-versions
