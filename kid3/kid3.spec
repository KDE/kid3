#
# spec file for package kid3
#

Name:         kid3
License:      GPL
Group:        Applications/Multimedia
Summary:      Efficient ID3 tag editor
Version:      1.5
Release:      1%{?dist}
URL:          http://kid3.sourceforge.net/
Source0:      http://downloads.sourceforge.net/kid3/%{name}-%{version}.tar.gz
BuildRoot:    %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  kdelibs4-devel
# OpenSUSE: BuildRequires:  libkde4-devel
BuildRequires:  cmake
BuildRequires:  id3lib-devel
# Mandriva: BuildRequires:  libid3lib3.8-devel
BuildRequires:  taglib-devel >= 1.4
BuildRequires:  flac-devel
# Mandriva: BuildRequires:  libflac++-devel
BuildRequires:  libtunepimp-devel
BuildRequires:  libvorbis-devel
BuildRequires:  libmp4v2-devel
BuildRequires:  gettext
Requires:       xdg-utils

%description
With Kid3 you can:

- Edit ID3v1.1 tags
- Edit all ID3v2.3 and ID3v2.4 frames
- Convert between ID3v1.1, ID3v2.3 and ID3v2.4 tags
- Edit tags in MP3, Ogg/Vorbis, FLAC, MPC, MP4/AAC, MP2, Speex,
  TrueAudio, WavPack, WMA, WAV and AIFF files.
- Edit tags of multiple files, e.g. the artist, album, year and genre
  of all files of an album typically have the same values and can be
  set together.
- Generate tags from filenames
- Generate filenames from tags
- Generate playlist files
- Automatic case conversion and string translation
- Import and export album data
- Import from gnudb.org, TrackType.org, MusicBrainz, Discogs, Amazon

This package uses KDE libraries, if you do not use KDE you should use kid3-qt.

Authors: Urs Fleisch


%prep
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}
%setup -q

%build
mkdir kde-build
cd kde-build; \
cmake -DCMAKE_SKIP_RPATH=ON -DCMAKE_INSTALL_PREFIX=/usr -DLIB_SUFFIX= -DCMAKE_BUILD_TYPE=release ..; \
make %{?_smp_mflags}; \
cd ..

mkdir qt-build
cd qt-build; \
../kid3-qt/configure --prefix=/usr; \
make %{?_smp_mflags}; \
cd ..

%install
# qmake generates wrong relative paths if the build and prefix are below /usr.
# This fixes the case for prefix /usr
find qt-build -name Makefile -exec sed -i 's|$(INSTALL_ROOT)\(../\)\{6,7\}|$(INSTALL_ROOT)/usr/|g' {} \;
mkdir -p ${RPM_BUILD_ROOT}/%{_defaultdocdir}
make -C kde-build install DESTDIR=${RPM_BUILD_ROOT}
make -C qt-build install INSTALL_ROOT=${RPM_BUILD_ROOT}
install -Dpm 644 deb/kid3.1 $RPM_BUILD_ROOT%{_mandir}/man1/kid3.1

test -d $RPM_BUILD_ROOT/usr/bin && strip $RPM_BUILD_ROOT/usr/bin/*
find $RPM_BUILD_ROOT -type f -o -name "*.so" -exec strip "{}" \;

%clean
[ -d  ${RPM_BUILD_ROOT} -a "${RPM_BUILD_ROOT}" != "/" ] && rm -rf  ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING LICENSE README
%{_bindir}/kid3
%{_datadir}/applications/kde4/*kid3.desktop
%{_datadir}/icons/hicolor/*x*/apps/kid3.png
%{_datadir}/icons/hicolor/scalable/apps/kid3.svgz
%{_datadir}/dbus-1/interfaces/*.xml
%{_datadir}/kde4/apps/kid3/
%{_datadir}/doc/kde/HTML/en/kid3/
%{_datadir}/doc/kde/HTML/de/kid3/
%{_datadir}/locale/*/LC_MESSAGES/kid3.mo
%{_mandir}/man1/kid3.1*


%package qt
Group:        Applications/Multimedia
Summary:      Efficient ID3 tag editor

%description qt
With Kid3 you can:

- Edit ID3v1.1 tags
- Edit all ID3v2.3 and ID3v2.4 frames
- Convert between ID3v1.1, ID3v2.3 and ID3v2.4 tags
- Edit tags in MP3, Ogg/Vorbis, FLAC, MPC, MP4/AAC, MP2, Speex,
  TrueAudio, WavPack, WMA, WAV and AIFF files.
- Edit tags of multiple files, e.g. the artist, album, year and genre
  of all files of an album typically have the same values and can be
  set together.
- Generate tags from filenames
- Generate filenames from tags
- Generate playlist files
- Automatic case conversion and string translation
- Import and export album data
- Import from gnudb.org, TrackType.org, MusicBrainz, Discogs, Amazon

This package does not use KDE libraries, if you use KDE you should use kid3.

Authors: Urs Fleisch

%files qt
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING LICENSE README
%{_bindir}/kid3-qt
%{_datadir}/applications/*kid3-qt.desktop
%{_datadir}/icons/hicolor/*x*/apps/kid3-qt.png
%{_datadir}/icons/hicolor/scalable/apps/kid3-qt.svgz
%{_datadir}/doc/kid3-qt/
%{_datadir}/kid3-qt/translations/
