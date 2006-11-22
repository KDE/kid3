#
# spec file for package kid3
#
# neededforbuild  aaa_base aaa_dir base bash cpp glibc glibc-devel grep gzip less man sh-utils binutils fileutils gdb id3lib id3lib-devel gpp gcc make perl qt qt-devel qt-devel-doc qt3-devel-tools rpm gettext kdelib kdelibs-devel libxslt libvorbis-devel libogg-devel flac-devel libtunepimp-devel

Name:         kid3
License:      GPL
Group:        Applications/Multimedia
Summary:      Efficient ID3 tag editor
Version:      0.8.1
Release:      1%{?dist}
URL:          http://kid3.sourceforge.net/
Source0:      http://download.sourceforge.net/kid3/%{name}-%{version}.tar.gz
BuildRoot:    %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Prefix:       /opt/kde3
#BuildRequires:  kdelibs-devel
#BuildRequires:  id3lib-devel
#BuildRequires:  flac-devel
#BuildRequires:  libtunepimp-devel
#BuildRequires:  taglib-devel
#BuildRequires:  desktop-file-utils
#BuildRequires:  perl(File::Spec)

%description
With Kid3 you can:

- Edit ID3v1.1 tags
- Edit all ID3v2.3 frames
- Convert between ID3v1.1 and ID3v2.3 tags
- Edit Ogg/Vorbis tags
- Edit FLAC tags
- Edit tags of multiple files, e.g. the artist, album, year and genre
  of all files of an album typically have the same values and can be
  set together.
- Generate tags from filenames
- Generate filenames from tags
- Generate playlist files
- Automatic case conversion and string translation
- Import of album data from freedb.org, MusicBrainz and other data sources
- Export of album data in various formats

Authors: Urs Fleisch


%prep
[ ${RPM_BUILD_ROOT} != "/" ] && rm -rf ${RPM_BUILD_ROOT}
%setup -q
sed -i -e 's/\r//' LICENSE

%build
./configure --disable-debug --prefix=%{prefix}
make

%install
make DESTDIR=$RPM_BUILD_ROOT install
find $RPM_BUILD_ROOT -type f -o -type l | sed "s|^$RPM_BUILD_ROOT||" >master.list
mkdir -p ${RPM_BUILD_ROOT}/%{_defaultdocdir}
strip $RPM_BUILD_ROOT%_bindir/*
find $RPM_BUILD_ROOT%_prefix -type f -o -name "*.so" -exec strip "{}" \;

%clean
[ -d  ${RPM_BUILD_ROOT} -a "${RPM_BUILD_ROOT}" != "/" ] && rm -rf  ${RPM_BUILD_ROOT}

%files -f master.list
%defattr(-,root,root)
%doc AUTHORS COPYING INSTALL LICENSE README ChangeLog
