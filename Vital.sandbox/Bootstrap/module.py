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
    cwd = os.path.abspath(os.getcwd())
    vital_bin = os.path.normpath(os.path.join(cwd, "..", f"Vital.{self.Args["platform_type"].lower()}", self.name.lower(), self["platform"]))
    build_dir = os.path.dirname(str(build[0].abspath))
    vilta_stage = self.Alias("stage", self.Install(vital_bin, self.Glob(os.path.join(build_dir, "*"))))
    self.Stage_VCPKG(build, build_dir)
    self.Stage_Discord(build, build_dir)
    self.Depends(vilta_stage, build)
    self.Default(vilta_stage)
BaseEnvironment.Stage_Module = Stage_Module
