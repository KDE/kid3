#
# spec file for package kid3
#

Name:         kid3
License:      GPL
Group:        Applications/Multimedia
Summary:      Efficient ID3 tag editor
Version:      0.10
Release:      1%{?dist}
URL:          http://kid3.sourceforge.net/
Source0:      http://downloads.sourceforge.net/kid3/%{name}-%{version}.tar.gz
BuildRoot:    %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
# If these two prefixes overlap, generation of master.list and master-qt.list
# will not work and you have to list the %files explicitly.
%define _kid3_kde_prefix /opt/kde3
%define _kid3_qt_prefix /usr
Prefix:       %{_kid3_kde_prefix}
BuildRequires:  kdelibs-devel
BuildRequires:  id3lib-devel
BuildRequires:  taglib-devel >= 1.4
BuildRequires:  flac-devel
BuildRequires:  libmp4v2-devel
BuildRequires:  libtunepimp-devel
BuildRequires:  perl(File::Spec)

%description
With Kid3 you can:

- Edit ID3v1.1 tags
- Edit all ID3v2.3 and ID3v2.4 frames
- Convert between ID3v1.1, ID3v2.3 and ID3v2.4 tags
- Edit tags in MP3, Ogg/Vorbis, FLAC, MPC, MP4/AAC, MP2, Speex,
  TrueAudio and WavPack files.
- Edit tags of multiple files, e.g. the artist, album, year and genre
  of all files of an album typically have the same values and can be
  set together.
- Generate tags from filenames
- Generate filenames from tags
- Generate playlist files
- Automatic case conversion and string translation
- Import and export album data
- Import from gnudb.org, TrackType.org, MusicBrainz, Discogs

This package uses KDE libraries, if you do not use KDE you should use kid3-qt.

Authors: Urs Fleisch


%prep
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}
%setup -q
sed -i -e 's|/lib /usr/lib\b|/%{_lib} %{_libdir}|g' configure # lib64 rpaths

%build
./configure --disable-debug --prefix=%{_kid3_kde_prefix}
make

cd kid3-qt; \
./configure --prefix=%{_kid3_qt_prefix}; \
cd ..
make -C kid3-qt

%install
make DESTDIR=$RPM_BUILD_ROOT install
find $RPM_BUILD_ROOT%{_kid3_kde_prefix} -type f -o -type l | sed "s|^$RPM_BUILD_ROOT||" >master.list
mkdir -p ${RPM_BUILD_ROOT}/%{_defaultdocdir}
find $RPM_BUILD_ROOT%{_kid3_kde_prefix} -type f -o -name "*.so" -exec strip "{}" \;

# qmake generates wrong relative paths if the build and prefix are below /usr.
# This fixes the case for prefix /usr
find kid3-qt -name Makefile -exec sed -i 's|$(INSTALL_ROOT)\(../\)\{6,7\}|$(INSTALL_ROOT)/usr/|g' {} \;
make -C kid3-qt install INSTALL_ROOT=$RPM_BUILD_ROOT
find $RPM_BUILD_ROOT%{_kid3_qt_prefix} -type f -o -type l | sed "s|^$RPM_BUILD_ROOT||" >master-qt.list
find $RPM_BUILD_ROOT%{_kid3_qt_prefix} -type f -o -name "*.so" -exec strip "{}" \;

%clean
[ -d  ${RPM_BUILD_ROOT} -a "${RPM_BUILD_ROOT}" != "/" ] && rm -rf  ${RPM_BUILD_ROOT}

%files -f master.list
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING LICENSE README


%package qt
Group:        Applications/Multimedia
Summary:      Efficient ID3 tag editor
Prefix:       %{_kid3_qt_prefix}

%description qt
With Kid3 you can:

- Edit ID3v1.1 tags
- Edit all ID3v2.3 and ID3v2.4 frames
- Convert between ID3v1.1, ID3v2.3 and ID3v2.4 tags
- Edit tags in MP3, Ogg/Vorbis, FLAC, MPC, MP4/AAC, MP2, Speex,
  TrueAudio and WavPack files.
- Edit tags of multiple files, e.g. the artist, album, year and genre
  of all files of an album typically have the same values and can be
  set together.
- Generate tags from filenames
- Generate filenames from tags
- Generate playlist files
- Automatic case conversion and string translation
- Import and export album data
- Import from gnudb.org, TrackType.org, MusicBrainz, Discogs

This package does not use KDE libraries, if you use KDE you should use kid3.

Authors: Urs Fleisch

%files qt -f master-qt.list
%defattr(-,root,root,-)
%doc AUTHORS ChangeLog COPYING LICENSE README
