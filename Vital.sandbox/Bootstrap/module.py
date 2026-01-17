from Bootstrap.utils import *
from Bootstrap.vendor import *
from Bootstrap.conan import *
from Bootstrap.vcpkg import *

def Build_Module(self):
    self.Build_Vendor()
    self.Build_Conan()
    self.Build_VCPKG()
    self.Build_Discord()
BaseEnvironment.Build_Module = Build_Module

def Stage_Module(self, build):
    build_dir = os.path.dirname(str(build[0].abspath))
    self.Stage_VCPKG(build, build_dir)
    self.Stage_Discord(build, build_dir)
BaseEnvironment.Stage_Module = Stage_Module
