from conan import ConanFile
from conan.tools.scons import SConsDeps
from vital import *

class BuildConan(ConanFile):
    settings = "os", "arch", "build_type", "compiler"
    generators = "SConsDeps"
    
    def requirements(self):
        self.requires("libmysqlclient/8.1.0")
        self.requires("soci/4.1.2")

    def configure(self):
        self.conf.define("tools.cmake.cmaketoolchain:generator", "Ninja")
        self.options["libmysqlclient"].shared = False
        self.options["soci"].shared = False
        self.options["soci"].with_mysql = True
        self.options["soci"].with_sqlite3 = False
        self.options["soci"].with_postgresql = False
        self.options["soci"].with_odbc = False
        self.options["soci"].with_boost = False