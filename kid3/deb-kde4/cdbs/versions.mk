
CDBS_MIN_VER:=0.4.52
QUILT_MIN_VER:=0.40
CMAKE_MIN_VER:=2.4.8


KDELIBS_VERSION:=$(shell dpkg -l kdelibs5 | grep kdelibs5 | awk '{print $$3}')
KDELIBS_SOURCE_VERSION:=$(shell echo $(KDELIBS_VERSION) | sed 's/+b.*//')
KDELIBS_UPSTREAM_VERSION:=$(shell echo $(KDELIBS_VERSION) | sed 's/-.*//')

KDEPIMLIBS_VERSION:=$(shell dpkg -l kdepimlibs5 | grep kdepimlibs5 | awk '{print $$3}')
KDEPIMLIBS_SOURCE_VERSION:=$(shell echo $(KDEPIMLIBS_VERSION) | sed 's/+b.*//')
KDEPIMLIBS_UPSTREAM_VERSION:=$(shell echo $(KDEPIMLIBS_VERSION) | sed 's/-.*//')

