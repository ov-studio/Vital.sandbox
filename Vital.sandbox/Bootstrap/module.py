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
    self.Stage_VCPKG(build)
    self.Stage_Discord(build)
BaseEnvironment.Stage_Module = Stage_Module
