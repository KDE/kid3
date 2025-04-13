# This will build a Kid3 package using MSVC and vcpkg and install into a
# directory msvc_pkg at the same level as the source folder.
# You may have to adapt the paths used for WITH_DOCBOOKDIR and XSLTPROC.
$kid3WinDir = $PSScriptRoot.Replace('\', '/')
$kid3Parent = (Get-Item -Path $kid3WinDir).Parent.Parent.FullName.Replace('\', '/')
$kid3SrcDir = "$kid3Parent/kid3"
$kid3DstDir = "$kid3Parent/msvc_build"
$kid3PkgDir = "$kid3Parent/msvc_pkg"

$env:PATH += ";$env:VCPKG_ROOT"
# If the vcpkg installation is already done, you can speed up the configuration
# with -DVCPKG_MANIFEST_INSTALL=OFF
cmake -B $kid3DstDir -S $kid3SrcDir -DCMAKE_BUILD_TYPE=Release `
  -DVCPKG_MANIFEST_DIR="$kid3WinDir" `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DWITH_ID3LIB=OFF  `
  -DWITH_DOCBOOKDIR="C:/msys64/usr/share/xml/docbook/xsl-stylesheets-1.79.2"  `
  -DXSLTPROC="C:/msys64/usr/bin/xsltproc.exe" `
  -G "Visual Studio 17 2022"
cmake --build $kid3DstDir --config Release
cmake --install $kid3DstDir --config Release --prefix $kid3PkgDir --strip
$kid3Dlls = @(
    "avcodec-61.dll",
    "avformat-61.dll",
    "avutil-59.dll",
    "brotlicommon.dll",
    "brotlidec.dll",
    "bz2.dll",
    "chromaprint.dll",
    "double-conversion.dll",
    "FLAC.dll",
    "FLAC++.dll",
    "freetype.dll",
    "harfbuzz.dll",
    "icudt74.dll",
    "icuin74.dll",
    "icuuc74.dll",
    "jpeg62.dll",
    "libcrypto-3-x64.dll",
    "libcrypto-3-x64.dll",
    "libpng16.dll",
    "libsharpyuv.dll",
    "libssl-3-x64.dll",
    "libwebp.dll",
    "libwebpdemux.dll",
    "libwebpmux.dll",
    "ogg.dll",
    "pcre2-16.dll",
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Multimedia.dll",
    "Qt6Network.dll",
    "Qt6Qml.dll",
    "Qt6QmlMeta.dll",
    "Qt6Widgets.dll",
    "swresample-5.dll",
    "tag.dll",
    "vorbis.dll",
    "vorbisfile.dll",
    "zlib1.dll",
    "zstd.dll"
)
foreach ($lib in $kid3Dlls) {
  Copy-Item "$kid3DstDir/vcpkg_installed/x64-windows/bin/${lib}" $kid3PkgDir
}
