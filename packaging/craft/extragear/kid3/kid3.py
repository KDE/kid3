import info
import utils
from CraftCore import CraftCore
from CraftStandardDirs import CraftStandardDirs
from Package.CMakePackageBase import CMakePackageBase


class subinfo(info.infoclass):
    def setTargets(self):
        self.svnTargets["master"] = "https://invent.kde.org/multimedia/kid3.git"

        for ver in ["3.9.3", "3.9.4"]:
            self.targets[ver] = f"https://download.kde.org/stable/kid3/{ver}/kid3-{ver}.tar.xz"
            self.targetInstSrc[ver] = "kid3-" + ver
            self.targetDigestUrls[ver] = f"https://download.kde.org/stable/kid3/{ver}/kid3-{ver}.tar.xz.sha256"

        self.description = "Edit audio file metadata"
        self.webpage = "https://kid3.kde.org"
        self.displayName = "Kid3"

        self.defaultTarget = "3.9.4"

    def setDependencies(self):
        self.buildDependencies["kde/frameworks/extra-cmake-modules"] = None
        self.buildDependencies["data/docbook-xsl"] = None
        self.runtimeDependencies["virtual/base"] = None
        self.runtimeDependencies["libs/qt/qtbase"] = None
        self.runtimeDependencies["libs/qt/qtmultimedia"] = None
        self.runtimeDependencies["libs/taglib"] = None
        self.runtimeDependencies["libs/chromaprint"] = None
        self.runtimeDependencies["libs/ffmpeg"] = None


class Package(CMakePackageBase):
    def __init__(self):
        super().__init__()
        self.subinfo.options.configure.args += [
            "-DWITH_ID3LIB=OFF",
            "-DWITH_VORBIS=OFF",
            "-DWITH_FLAC=OFF",
            "-DWITH_MP4V2=OFF",
            "-DWITH_QML=ON",
            '-DWITH_APPS="Qt;CLI"',
        ]
        if self.buildTarget == "master":
            self.subinfo.options.configure.args += ["-DDOWNLOAD_POS=REMOVE"]

        docbookdir = CraftStandardDirs.craftRoot() / "bin/data/xml/docbook/xsl-stylesheets"
        if CraftCore.compiler.isMacOS:
            docbookdir = CraftStandardDirs.craftRoot() / "share/xml/docbook/xsl-stylesheets"
        self.subinfo.options.configure.args += [f"-DWITH_DOCBOOKDIR={docbookdir}"]
        self.subinfo.options.package.movePluginsToBin = False

    def createPackage(self):
        self.blacklist_file.append(self.blueprintDir() / "blacklist.txt")
        self.defines["executable"] = "kid3.exe"
        self.defines["icon"] = self.sourceDir() / "src/app/qt/kid3.ico"
        return super().createPackage()

    def install(self):
        result = super().install()
        if CraftCore.compiler.isLinux:
            utils.copyFile(
                self.installDir() / "share/applications/org.kde.kid3-qt.desktop",
                self.installDir() / "share/applications/kid3.desktop",
            )
        return result
