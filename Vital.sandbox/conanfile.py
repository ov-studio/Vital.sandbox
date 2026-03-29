from conan import ConanFile
from conan.tools.scons import SConsDeps
from vital import *

class BuildConan(ConanFile):
    settings = "os", "arch", "build_type", "compiler"
    generators = "SConsDeps"
    
    def requirements(self):
        self.requires("openssl/3.5.1")
        self.requires("cpp-httplib/0.39.0")
        self.requires("libmysqlclient/8.1.0")
        self.requires("soci/4.1.2")
        self.requires("zlib/1.3.1")
        self.requires("libzip/1.11.4")
        self.requires("rapidjson/1.1.0")
        self.requires("yaml-cpp/0.8.0")
        self.requires("fmt/12.1.0")

    def configure(self):
        os_info = Fetch_OS()
        self.options["openssl"].shared = False
        self.options["cpp-httplib"].shared = False
        self.options["libmysqlclient"].shared = False
        self.options["soci"].shared = False
        self.options["soci"].with_mysql = True
        self.options["soci"].with_sqlite3 = False
        self.options["soci"].with_postgresql = False
        self.options["soci"].with_odbc = False
        self.options["soci"].with_boost = False
        self.options["zlib"].shared = False
        self.options["libzip"].shared = False
        self.options["rapidjson"].shared = False
        self.options["yaml-cpp"].shared = False
        self.options["fmt"].shared = False

