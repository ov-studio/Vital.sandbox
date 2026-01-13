import os, subprocess
from utils import *
from SCons.Environment import Base as BaseEnvironment
from SCons.Script import Copy, Action

os_info = Fetch_OS()

def Init_VCPKG(self):
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
    vcpkg = self.Init_VCPKG()
    if not os.path.isdir(vcpkg["root"]):
        subprocess.run((
            "git", "clone",
            "https://github.com/microsoft/vcpkg.git",
            f"{vcpkg["root"]}"
        ))
    subprocess.run((f"{os.path.join(vcpkg["root"], "bootstrap-vcpkg.bat" if os_info["type"] == "Windows" else "bootstrap-vcpkg.sh")}"))
BaseEnvironment.Install_VCPKG = Install_VCPKG

def Build_VCPKG(self):
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
    vcpkg = self.Init_VCPKG()
    out_dir = os.path.dirname(str(build[0].abspath))
    vcpkg_bin = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "bin")
    copy_nodes = []
    if os.path.isdir(vcpkg_bin):
        if os_info["type"] == "Windows":
            for dll in os.listdir(vcpkg_bin):
                if dll.lower().endswith(".dll"):
                    src = os.path.join(vcpkg_bin, dll)
                    dst = os.path.join(out_dir, dll)
                    copy_nodes.append(self.Command(dst, src, Action(Copy("$TARGET", "$SOURCE"), None)))
        else:
            for so_file in os.listdir(vcpkg_lib):
                if so_file.endswith(".so") or ".so." in so_file:
                    src = os.path.join(vcpkg_lib, so_file)
                    dst = os.path.join(out_dir, so_file)
                    copy_nodes.append(self.Command(dst, src, Action(Copy("$TARGET", "$SOURCE"), None)))
    self.Depends(build, copy_nodes)
BaseEnvironment.Stage_VCPKG = Stage_VCPKG
