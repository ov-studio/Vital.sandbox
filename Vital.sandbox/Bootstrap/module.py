from Bootstrap.utils import *
from Bootstrap.vcpkg import *

def Build_Module(self):
    self.Build_Conan()
    self.Build_VCPKG()
BaseEnvironment.Build_Module = Build_Module

def Stage_Module(self, build):
    self.Stage_VCPKG(build)
BaseEnvironment.Stage_Module = Stage_Module
