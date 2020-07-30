import info
from Package.CMakePackageBase import *


class subinfo(info.infoclass):
    def setDependencies(self):
        self.runtimeDependencies['virtual/base'] = None
        self.runtimeDependencies['libs/zlib'] = None
        self.descriptions = 'Extracts fingerprints from any audio source'

    def setTargets(self):
        for ver in ['1.5.0']:
            self.targets[ver] = f'https://github.com/acoustid/chromaprint/releases/download/v{ver}/chromaprint-{ver}.tar.gz'
            self.targetInstSrc[ver] = f'chromaprint-v{ver}'
        self.targetDigests['1.5.0'] = (['573a5400e635b3823fc2394cfa7a217fbb46e8e50ecebd4a61991451a8af766a'], CraftHash.HashAlgorithm.SHA256)
        self.description = 'Extracts fingerprints from any audio source'
        self.webpage = 'https://acoustid.org/chromaprint'
        self.defaultTarget = '1.5.0'


class Package(CMakePackageBase):
    def __init__(self):
        CMakePackageBase.__init__(self)
        self.subinfo.options.configure.args = '-DBUILD_SHARED_LIBS=ON'
