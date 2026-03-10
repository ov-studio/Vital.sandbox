from Bootstrap.utils import *
from Bootstrap.vendor import *
from Bootstrap.conan import *
from Bootstrap.vcpkg import *

class Module:
    def __init__(self, env):
        self.env = env

    def build(self):
        self.env.Vendor.build()
        self.env.Conan.build()
        self.env.VCPKG.build()
        self.env.Discord.build()

    def stage(self, build):
        cwd = os.path.abspath(os.getcwd())
        vital_dir = os.path.normpath(os.path.join(cwd, "..", f"Vital.{self.env.Args['platform_type'].lower()}", self.env.name.lower(), self.env['platform']))
        build_dir = os.path.dirname(str(build[0].abspath))
        self.env.VCPKG.stage(build, build_dir)
        self.env.Discord.stage(build, build_dir)
        vital_stage = self.env.Alias("stage", self.env.Install(vital_dir, self.env.Glob(os.path.join(build_dir, "*"))))
        self.env.Depends(vital_stage, build)
        self.env.Default(vital_stage)

BaseEnvironment.Module = property(lambda self: Module(self))