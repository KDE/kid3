#
# spec file for package kid3 (Version 0.1)
#
# neededforbuild  aaa_base aaa_dir base bash cpp glibc glibc-devel grep gzip less man sh-utils binutils fileutils gdb id3lib id3lib-devel gpp gcc make perl qt qt-devel qt-devel-doc qt3-devel-tools rpm gettext kdelib kdelibs-devel libxslt

Name:         kid3
%define suse_version   730
%if %suse_version > 730
%define qt_path    /usr/lib/qt3
%define kde_path   /opt/kde3
%else
%define qt_path    /usr/lib/qt2
%define kde_path   /opt/kde2
%endif
Copyright:    GPL
Group:        Applications/Multimedia
Summary:      ID3 tagger
Version:      0.1
Release:      1
Source0:      kid3-%{version}.tgz
BuildRoot:    /var/tmp/%{name}-buildroot

%description
Kid3 - Efficient ID3 Tagger

With Kid3 you can:

- Edit ID3v1.1 tags
- Edit all ID3v2.3 frames
- Convert between ID3v1.1 and ID3v2.3 tags
- Edit tags of multiple files, e.g. the artist, album, year and genre
  of all files of an album typically have the same values and can be
  set together.
- Generate tags from filenames
- Generate filenames from tags
- Generate playlist files

Authors:
--------
    Urs Fleisch


%prep
%setup -n kid3-0.1

%build
  
make KDEDIR=%{kde_path} QTDIR=%{qt_path}

%install
make KDEDIR=%{kde_path} QTDIR=%{qt_path} DESTDIR=$RPM_BUILD_ROOT install

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}

%files
%doc AUTHORS COPYING INSTALL LICENSE README ChangeLog
%dir %{kde_path}/share/apps/kid3
%dir %{kde_path}/share/doc/HTML/en/kid3/
%dir %{kde_path}/share/doc/HTML/de/kid3/
%{kde_path}/bin/kid3
%{kde_path}/share/apps/kid3/kid3ui.rc
%{kde_path}/share/icons/hicolor/16x16/apps/kid3.png
%{kde_path}/share/icons/hicolor/32x32/apps/kid3.png
%{kde_path}/share/icons/hicolor/48x48/apps/kid3.png
%{kde_path}/share/applnk/Multimedia/kid3.desktop
%{kde_path}/share/doc/HTML/en/kid3/index.cache.bz2
%{kde_path}/share/doc/HTML/en/kid3/index.docbook
%{kde_path}/share/doc/HTML/en/kid3/common
%{kde_path}/share/doc/HTML/de/kid3/index.cache.bz2
%{kde_path}/share/doc/HTML/de/kid3/index.docbook
%{kde_path}/share/doc/HTML/de/kid3/common
%{kde_path}/share/locale/de/LC_MESSAGES/kid3.mo
