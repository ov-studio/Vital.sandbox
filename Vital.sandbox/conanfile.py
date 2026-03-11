from conan import ConanFile
from conan.tools.scons import SConsDeps
from vital import *

class BuildConan(ConanFile):
    settings = "os", "arch", "build_type", "compiler"
    generators = "SConsDeps"
    
    def requirements(self):
        self.requires("openssl/3.5.1")
        self.requires("libcurl/8.12.1")
        self.requires("zlib/1.3.1")
        self.requires("soci/4.1.2")
        self.requires("libmysqlclient/8.1.0")

    def configure(self):
        os_info = Fetch_OS()
        self.options["openssl"].shared = False
        self.options["libcurl"].shared = False
        self.options["zlib"].shared = False
        self.options["libmysqlclient"].shared = False
        self.options["soci"].shared = False
        self.options["soci"].with_mysql = True
        self.options["soci"].with_sqlite3 = False
        self.options["soci"].with_postgresql = False
        self.options["soci"].with_odbc = False
        self.options["soci"].with_boost = False
        
        if os_info["type"] == "Windows":
            self.settings.compiler.runtime = "static"