# -*- mode: makefile; coding: utf-8 -*-
# Copyright © 2003 Colin Walters <walters@debian.org>
# Description: Defines various random rules, including a list-missing rule
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

_cdbs_scripts_path ?= /usr/lib/cdbs
_cdbs_rules_path ?= /usr/share/cdbs/1/rules
_cdbs_class_path ?= /usr/share/cdbs/1/class

ifndef _cdbs_rules_utils
_cdbs_rules_utils = 1

include $(_cdbs_rules_path)/buildcore.mk$(_cdbs_makefile_suffix)

DEB_PHONY_RULES += list-missing

list-missing:
	@if test -d debian/tmp; then \
	  (cd debian/tmp && find . -type f -o -type l | grep -v '/DEBIAN/' | sort) > debian/cdbs-install-list; \
	  (for package in $(DEB_ALL_PACKAGES); do \
	     (cd debian/$$package && find . -type f -o -type l); \
	   done) | sort -u > debian/cdbs-package-list; \
	   if test -e debian/not-installed ;\
	     then grep -v '^#' debian/not-installed >> debian/cdbs-package-list ; \
	     sort -u <debian/cdbs-package-list > debian/cdbs-package-list.tmp ; mv debian/cdbs-package-list.tmp debian/cdbs-package-list ; \
	   fi ;\
	  diff -u debian/cdbs-install-list debian/cdbs-package-list | sed '1,2d' | egrep '^-' || true; \
	else \
	  echo "All files were installed into debian/$(DEB_SOURCE_PACKAGE)."; \
	fi

clean::
	rm -f debian/cdbs-install-list debian/cdbs-package-list

endif
