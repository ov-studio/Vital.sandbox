from Bootstrap.utils import *

def Init_VCPKG(self):
    os_info = Fetch_OS()
    root = os.path.join(os.path.abspath(os.path.dirname(__file__)), ".vcpkg")
    triplet = None
    if os_info["type"] == "Windows":
        triplet = "x64-windows"
    elif os_info["type"] == "Darwin":
        triplet = "arm64-osx"
    elif os_info["type"] == "Linux":
        triplet = "x64-linux"
    return {
        "root": root,
        "triplet": triplet
    }
BaseEnvironment.Init_VCPKG = Init_VCPKG

def Install_VCPKG(self):
    os_info = Fetch_OS()
    vcpkg = self.Init_VCPKG()
    if not os.path.isdir(vcpkg["root"]):
        subprocess.run((
            "git", "clone",
            "https://github.com/microsoft/vcpkg.git",
            f"{vcpkg["root"]}"
        ))
    subprocess.run([
        os.path.join(vcpkg["root"], "bootstrap-vcpkg.bat" if os_info["type"] == "Windows" else "bootstrap-vcpkg.sh"),
        "-disableMetrics"
    ])
BaseEnvironment.Install_VCPKG = Install_VCPKG

def Build_VCPKG(self):
    os_info = Fetch_OS()
    vcpkg = self.Init_VCPKG()
    vcpkg_include = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "include")
    vcpkg_lib = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "lib")
    vcpkg_libs = []
    self.Install_VCPKG()
    self.Append(CPPPATH=[vcpkg_include])
    self.Append(LIBPATH=[vcpkg_lib])
    if os.path.isdir(vcpkg_lib):
        if os_info["type"] == "Windows":
            for f in os.listdir(vcpkg_lib):
                if f.endswith(".lib"):
                    lib_name = f[:-4]
                    vcpkg_libs.append(lib_name)
        else:
            for f in os.listdir(vcpkg_lib):
                if f.endswith(".a"):
                    lib_name = f
                    if f.startswith("lib"):
                        lib_name = f[3:-2]
                    vcpkg_libs.append(lib_name)
    self.Append(LIBS=vcpkg_libs)
BaseEnvironment.Build_VCPKG = Build_VCPKG

def Stage_VCPKG(self, build):
    os_info = Fetch_OS()
    vcpkg = self.Init_VCPKG()
    vcpkg_root = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"])
    vcpkg_bin  = os.path.join(vcpkg_root, "bin")
    copy_nodes = []
    if os_info["type"] == "Windows":
        if os.path.isdir(vcpkg_bin):
            copy_nodes += self.RCopy(build, os.path.join(vcpkg_bin, "*.dll"))
    else:
        if os.path.isdir(vcpkg_bin):
            copy_nodes += self.RCopy(build, os.path.join(vcpkg_bin, "*.so"))
            copy_nodes += self.RCopy(build, os.path.join(vcpkg_bin, "*.so.*"))
    self.Depends(build, copy_nodes)
BaseEnvironment.Stage_VCPKG = Stage_VCPKG
