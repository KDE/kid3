#
# spec file for package kid3 (Version 0.3)
#
# neededforbuild  aaa_base aaa_dir base bash cpp glibc glibc-devel grep gzip less man sh-utils binutils fileutils gdb id3lib id3lib-devel gpp gcc make perl qt qt-devel qt-devel-doc qt3-devel-tools rpm gettext kdelib kdelibs-devel libxslt

Name:         kid3
Copyright:    GPL
Group:        Applications/Multimedia
Summary:      ID3 tagger
Version:      0.3
Release:      1
Source0:      kid3-%{version}.tar.gz
BuildRoot:    /var/tmp/%{name}-buildroot
Prefix:       /opt/kde3
Requires:     id3lib
BuildRequires: id3lib-devel

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
- Automatic case conversion and string translation
- Import of album data

Authors:
--------
    Urs Fleisch


%prep
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}
%setup -n %{name}-%{version}

%build
./configure --disable-debug --prefix=%{prefix}
make

%install
make DESTDIR=$RPM_BUILD_ROOT install
find $RPM_BUILD_ROOT -type f -o -type l | sed "s|^$RPM_BUILD_ROOT||" >master.list
mkdir -p ${RPM_BUILD_ROOT}/%{_defaultdocdir}

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}

%files -f master.list
%doc AUTHORS COPYING INSTALL LICENSE README ChangeLog
