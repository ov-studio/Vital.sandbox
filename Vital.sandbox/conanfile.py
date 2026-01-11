from conan import ConanFile
from conan.tools.scons import SConsDeps

class BuildConan(ConanFile):
    settings = "os", "arch","build_type", "compiler"
    generators = "SConsDeps"

    def requirements(self):
        self.requires("openssl/3.5.1")
        self.requires("libcurl/8.12.1")

    def configure(self):
        self.options["openssl"].shared = False
        self.options["libcurl"].shared = False
        self.settings.compiler.cppstd = "17"
        self.settings.compiler.runtime = "static"