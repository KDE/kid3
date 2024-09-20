%global gstversion 1.0

Name:           kid3
Version:        3.9.6
Release:        1%{?dist}
Summary:        Efficient ID3 tag editor

License:        GPLv2+
URL:            https://kid3.kde.org/
Source0:        https://downloads.sourceforge.net/kid3/%{name}-%{version}.tar.gz
BuildRequires:  kf5-kio-devel
BuildRequires:  kf5-kdoctools-devel
BuildRequires:  kf5-rpm-macros
BuildRequires:  extra-cmake-modules
BuildRequires:  qt5-qtmultimedia-devel
BuildRequires:  cmake
BuildRequires:  id3lib-devel
BuildRequires:  taglib-devel >= 1.4
BuildRequires:  flac-devel
BuildRequires:  libvorbis-devel
BuildRequires:  libchromaprint-devel
BuildRequires:  pkgconfig(gstreamer-%{gstversion})
BuildRequires:  readline-devel
BuildRequires:  gettext
BuildRequires:  libappstream-glib
BuildRequires:  gcc-c++
Requires:       %{name}-common = %{version}-%{release}

%description
If you want to easily tag multiple MP3, Ogg/Vorbis, FLAC, MPC,
MP4/AAC, MP2, Speex, TrueAudio, WavPack, WMA, WAV, and AIFF files
(e.g. full albums) without typing the same information again and again
and have control over both ID3v1 and ID3v2 tags, then Kid3 is the
program you are looking for.

%package        common
Summary:        Efficient command line ID3 tag editor
Recommends:     xdg-utils

%description    common
If you want to easily tag multiple MP3, Ogg/Vorbis, FLAC, MPC,
MP4/AAC, MP2, Speex, TrueAudio, WavPack, WMA, WAV, and AIFF files
(e.g. full albums) without typing the same information again and again
and have control over both ID3v1 and ID3v2 tags, then Kid3 is the
program you are looking for.  The %{name}-common package provides Kid3
command line tool and files shared between all Kid3 variants.


%package        qt
Summary:        Efficient Qt ID3 tag editor
Requires:       %{name}-common = %{version}-%{release}

%description    qt
If you want to easily tag multiple MP3, Ogg/Vorbis, FLAC, MPC,
MP4/AAC, MP2, Speex, TrueAudio, WavPack, WMA, WAV, and AIFF files
(e.g. full albums) without typing the same information again and again
and have control over both ID3v1 and ID3v2 tags, then Kid3 is the
program you are looking for.  The %{name}-qt package provides Kid3
built without KDE dependencies.


%prep
%autosetup -p1


%build
# lib64 stuff: //bugzilla.redhat.com/show_bug.cgi?id=1425064
%cmake_kf5 \
%if "%{?_lib}" == "lib64"
    %{?_cmake_lib_suffix64} \
%endif
    -DWITH_GSTREAMER_VERSION=%{gstversion} \
    -DWITH_NO_MANCOMPRESS=ON \
    .
%make_build


%install

%make_install

install -dm 755 $RPM_BUILD_ROOT%{_pkgdocdir}
install -pm 644 AUTHORS ChangeLog README $RPM_BUILD_ROOT%{_pkgdocdir}

%find_lang %{name} --with-man
# --with-kde doesn't work with kf5 yet
# https://github.com/rpm-software-management/rpm/pull/112
mv %{name}.lang %{name}-kde.lang
%find_lang %{name}-qt --with-man
%find_lang %{name}-cli --with-man
%find_lang %{name} --with-qt
for l in en $(ls doc/docs); do \
  echo "%%lang($l) %%{_kf5_datadir}/doc/HTML/$l/kid3/" >> %{name}-kde.lang; \
  echo "%%lang($l) %%{_docdir}/kid3-qt/kid3_$l.html" >> %{name}-qt.lang; \
done
cat %{name}.lang >> %{name}-cli.lang
cat <<EOF >> %{name}-cli.lang
%%dir %%{_datadir}/kid3/
%%dir %%{_datadir}/kid3/translations/
EOF


%check
appstream-util validate-relax --nonet \
    $RPM_BUILD_ROOT%{_datadir}/metainfo/*.appdata.xml

%files -f %{name}-kde.lang
%{_bindir}/kid3
%{_datadir}/metainfo/org.kde.kid3.appdata.xml
%{_datadir}/icons/hicolor/*x*/apps/kid3.png
%{_datadir}/icons/hicolor/scalable/apps/kid3.svgz
%{_datadir}/applications/org.kde.kid3.desktop
%{_datadir}/kxmlgui5/%{name}

%files common -f %{name}-cli.lang
%{_bindir}/kid3-cli
%{_libdir}/kid3/
%{_datadir}/dbus-1/interfaces/*.xml
%{_datadir}/kid3/qml/
%{_mandir}/man1/kid3.1*
%{_mandir}/man1/kid3-cli.1*
%license COPYING LICENSE
%{_pkgdocdir}/

%files qt -f %{name}-qt.lang
%{_bindir}/kid3-qt
%{_datadir}/metainfo/org.kde.kid3-qt.appdata.xml
%{_datadir}/applications/org.kde.kid3-qt.desktop
%{_datadir}/icons/hicolor/*x*/apps/kid3-qt.png
%{_datadir}/icons/hicolor/scalable/apps/kid3-qt.svg
%dir %{_docdir}/kid3-qt/
%{_mandir}/man1/kid3-qt.1*
