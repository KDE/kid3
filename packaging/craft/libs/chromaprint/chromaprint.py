import info
from Package.CMakePackageBase import *


class subinfo(info.infoclass):
    def setDependencies(self):
        self.runtimeDependencies["virtual/base"] = None
        self.runtimeDependencies["libs/zlib"] = None
        self.descriptions = "Extracts fingerprints from any audio source"

    def setTargets(self):
        for ver in ["1.5.0", "1.5.1"]:
            self.targets[ver] = f"https://github.com/acoustid/chromaprint/releases/download/v{ver}/chromaprint-{ver}.tar.gz"
            self.targetInstSrc[ver] = f"chromaprint-v{ver}" if ver <= "1.5.0" else f"chromaprint-{ver}"
        self.targetDigests["1.5.0"] = (["573a5400e635b3823fc2394cfa7a217fbb46e8e50ecebd4a61991451a8af766a"], CraftHash.HashAlgorithm.SHA256)
        self.targetDigests["1.5.1"] = (["a1aad8fa3b8b18b78d3755b3767faff9abb67242e01b478ec9a64e190f335e1c"], CraftHash.HashAlgorithm.SHA256)
        self.description = "Extracts fingerprints from any audio source"
        self.webpage = "https://acoustid.org/chromaprint"
        self.defaultTarget = "1.5.1"


class Package(CMakePackageBase):
    def __init__(self):
        super().__init__()
        self.subinfo.options.dynamic.buildStatic = False
