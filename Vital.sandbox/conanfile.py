from conan import ConanFile

class MyBin(ConanFile):
    settings = "os", "arch","build_type", "compiler"
    generators = "SConsDeps"

    def requirements(self):
        self.requires("openssl/3.5.1")

    def configure(self):
        self.options["openssl"].shared = False
        self.settings.compiler.runtime = "static"