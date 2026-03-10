from Bootstrap.utils import *

class VCPKG:
    def __init__(self, env):
        self.env = env

    def init(self):
        os_info = Fetch_OS()
        root = os.path.join(os.path.abspath(os.path.dirname(__file__)), ".vcpkg")
        triplets = {
            "Windows": "x64-windows",
            "Darwin": "arm64-osx",
            "Linux": "x64-linux"
        }
        return {
            "root": root,
            "triplet": triplets.get(os_info["type"])
        }

    def install(self):
        os_info = Fetch_OS()
        vcpkg = self.init()
        if not os.path.isdir(vcpkg["root"]):
            subprocess.run((
                "git", "clone",
                "https://github.com/microsoft/vcpkg.git",
                vcpkg["root"]
            ))
        bootstrap = "bootstrap-vcpkg.bat" if os_info["type"] == "Windows" else "bootstrap-vcpkg.sh"
        subprocess.run([os.path.join(vcpkg["root"], bootstrap), "-disableMetrics"])

    def build(self):
        os_info = Fetch_OS()
        vcpkg = self.init()
        vcpkg_include = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "include")
        vcpkg_lib = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "lib")
        self.install()
        self.env.Append(CPPPATH=[vcpkg_include])
        self.env.Append(LIBPATH=[vcpkg_lib])
        vcpkg_libs = []
        if os.path.isdir(vcpkg_lib):
            for f in os.listdir(vcpkg_lib):
                if os_info["type"] == "Windows" and f.endswith(".lib"):
                    vcpkg_libs.append(f[:-4])
                elif f.endswith(".a"):
                    vcpkg_libs.append(f[3:-2] if f.startswith("lib") else f)
        self.env.Append(LIBS=vcpkg_libs)

    def stage(self, build, build_dir):
        os_info = Fetch_OS()
        vcpkg = self.init()
        vcpkg_bin = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "bin")
        copy_nodes = []
        if os.path.isdir(vcpkg_bin):
            if os_info["type"] == "Windows":
                copy_nodes += self.env.RCopy(build_dir, os.path.join(vcpkg_bin, "*.dll"))
            else:
                copy_nodes += self.env.RCopy(build_dir, os.path.join(vcpkg_bin, "*.so"))
                copy_nodes += self.env.RCopy(build_dir, os.path.join(vcpkg_bin, "*.so.*"))
        self.env.Depends(build, copy_nodes)

BaseEnvironment.VCPKG = property(lambda self: VCPKG(self))