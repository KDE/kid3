#
# spec file for package kid3
#

Name:         kid3
License:      GPL
Group:        Applications/Multimedia
Summary:      Efficient ID3 tag editor
Version:      3.3.2
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
BuildRequires:  libchromaprint-devel
BuildRequires:  libvorbis-devel
BuildRequires:  readline-devel
BuildRequires:  gettext
Requires:       xdg-utils

%description
With Kid3 you can:

- Edit ID3v1.1 tags
- Edit all ID3v2.3 and ID3v2.4 frames
- Convert between ID3v1.1, ID3v2.3 and ID3v2.4 tags
- Edit tags in MP3, Ogg/Vorbis, Opus, DSF, FLAC, MPC, APE, MP4/AAC,
  MP2, Speex, TrueAudio, WavPack, WMA, WAV, AIFF files and tracker
  modules (MOD, S3M, IT, XM).
- Edit tags of multiple files, e.g. the artist, album, year and genre
  of all files of an album typically have the same values and can be
  set together.
- Generate tags from filenames
- Generate tags from the contents of tag fields
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
mkdir kid3-build
cd kid3-build; \
cmake -DCMAKE_INSTALL_PREFIX=/usr -DLIB_SUFFIX= -DCMAKE_BUILD_TYPE=Release ..; \
make %{?_smp_mflags}; \
cd ..

%install
mkdir -p ${RPM_BUILD_ROOT}/%{_defaultdocdir}
make -C kid3-build install DESTDIR=${RPM_BUILD_ROOT}

test -d $RPM_BUILD_ROOT/usr/bin && strip $RPM_BUILD_ROOT/usr/bin/*
find $RPM_BUILD_ROOT -type f -o -name "*.so" -exec strip "{}" \;

%clean
[ -d  ${RPM_BUILD_ROOT} -a "${RPM_BUILD_ROOT}" != "/" ] && rm -rf  ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING LICENSE README
%{_bindir}/kid3
%{_datadir}/applications/kde4/*kid3.desktop
%{_datadir}/appdata/kid3.appdata.xml
%{_datadir}/icons/hicolor/*x*/apps/kid3.png
%{_datadir}/icons/hicolor/scalable/apps/kid3.svgz
%{_datadir}/kde4/apps/kid3/
%{_datadir}/doc/kde/HTML/en/kid3/
%{_datadir}/doc/kde/HTML/de/kid3/


%package qt
Group:        Applications/Multimedia
Summary:      Efficient ID3 tag editor

%description qt
With Kid3 you can:

- Edit ID3v1.1 tags
- Edit all ID3v2.3 and ID3v2.4 frames
- Convert between ID3v1.1, ID3v2.3 and ID3v2.4 tags
- Edit tags in MP3, Ogg/Vorbis, FLAC, MPC, MP4/AAC, MP2, Speex,
  TrueAudio, WavPack, WMA, WAV, AIFF files and tracker modules (MOD,
  S3M, IT, XM).
- Edit tags of multiple files, e.g. the artist, album, year and genre
  of all files of an album typically have the same values and can be
  set together.
- Generate tags from filenames
- Generate tags from the contents of tag fields
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
%{_datadir}/appdata/kid3-qt.appdata.xml
%{_datadir}/icons/hicolor/*x*/apps/kid3-qt.png
%{_datadir}/icons/hicolor/scalable/apps/kid3-qt.svg
%{_datadir}/doc/kid3-qt/
%{_mandir}/man1/kid3-qt.1.gz
%{_mandir}/de/man1/kid3-qt.1.gz

%package core
Group:        Applications/Multimedia
Summary:      Audio tag editor core libraries and data

%description core
This package contains common libraries and data used by both kid3 and kid3-qt.

Authors: Urs Fleisch

%files core
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING LICENSE README
%{_libdir}/kid3/*
%{_datadir}/dbus-1/interfaces/*.xml
%{_datadir}/kid3/translations/
%{_datadir}/kid3/qml/script/
%{_mandir}/man1/kid3.1.gz
%{_mandir}/de/man1/kid3.1.gz

%package cli
Group:        Applications/Multimedia
Summary:      Efficient ID3 tag editor

%description cli
With Kid3 you can:

- Edit ID3v1.1 tags
- Edit all ID3v2.3 and ID3v2.4 frames
- Convert between ID3v1.1, ID3v2.3 and ID3v2.4 tags
- Edit tags in MP3, Ogg/Vorbis, FLAC, MPC, MP4/AAC, MP2, Speex,
  TrueAudio, WavPack, WMA, WAV, AIFF files and tracker modules (MOD,
  S3M, IT, XM).
- Edit tags of multiple files, e.g. the artist, album, year and genre
  of all files of an album typically have the same values and can be
  set together.
- Generate tags from filenames
- Generate tags from the contents of tag fields
- Generate filenames from tags
- Generate playlist files
- Automatic case conversion and string translation
- Import and export album data
- Import from gnudb.org, TrackType.org, MusicBrainz, Discogs, Amazon

This package contains a command line interface for Kid3, for a GUI you can
use kid3-qt or kid3.

Authors: Urs Fleisch

%files cli
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING LICENSE README
%{_bindir}/kid3-cli
%{_mandir}/man1/kid3-cli.1.gz
%{_mandir}/de/man1/kid3-cli.1.gz
