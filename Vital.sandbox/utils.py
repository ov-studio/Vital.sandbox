import os
import sys
import fnmatch
import subprocess
from SCons.Environment import Base as BaseEnvironment
from SCons.Script import Copy, Action

def VCPKG(self):
    root = os.path.join(os.path.abspath(os.path.dirname(__file__)), ".vcpkg")
    triplet = None
    if sys.platform.startswith("win"):
        triplet = "x64-windows"
    elif sys.platform == "darwin":
        triplet = "arm64-osx"
    elif sys.platform.startswith("linux"):
        triplet = "x64-linux"
    return {
        "root": root,
        "triplet": triplet
    }
BaseEnvironment.VCPKG = VCPKG

def InstallVCPKG(self):
    vcpkg = self.VCPKG()
    if not os.path.isdir(vcpkg["root"]):
        subprocess.run((
            "git", "clone",
            "https://github.com/microsoft/vcpkg.git",
            f"{vcpkg["root"]}"
        ))
    subprocess.run((f"{os.path.join(vcpkg["root"], "bootstrap-vcpkg.bat" if sys.platform.startswith("win") else "bootstrap-vcpkg.sh")}"))
    subprocess.run((
        f"{os.path.join(vcpkg["root"], "vcpkg")}", "install", 
       "rmlui[lua]", "--recurse"
    ))
BaseEnvironment.InstallVCPKG = InstallVCPKG

def BuildVCPKG(self):
    vcpkg = self.VCPKG()
    vcpkg_include = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "include")
    vcpkg_lib = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "lib")
    vcpkg_libs = []
    self.InstallVCPKG()
    self.Append(CPPDEFINES={("RML_LUA_BINDINGS_LIBRARY", "lua")})
    self.Append(CPPPATH=[vcpkg_include])
    self.Append(LIBPATH=[vcpkg_lib])
    if sys.platform.startswith("win"):
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
BaseEnvironment.BuildVCPKG = BuildVCPKG

def StageVCPKG(self, build):
    vcpkg = self.VCPKG()
    out_dir = os.path.dirname(str(build[0].abspath))
    if sys.platform.startswith("win"):
        vcpkg_bin = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "bin")
        copy_nodes = []
        for dll in os.listdir(vcpkg_bin):
            if dll.lower().endswith(".dll"):
                src = os.path.join(vcpkg_bin, dll)
                dst = os.path.join(out_dir, dll)
                copy_nodes.append(self.Command(dst, src, Action(Copy("$TARGET", "$SOURCE"), None)))
        self.Depends(build, copy_nodes)
    else:
        vcpkg_lib = os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "lib")
        copy_nodes = []
        for so_file in os.listdir(vcpkg_lib):
            if so_file.endswith(".so") or ".so." in so_file:
                src = os.path.join(vcpkg_lib, so_file)
                dst = os.path.join(out_dir, so_file)
                copy_nodes.append(self.Command(dst, src, Action(Copy("$TARGET", "$SOURCE"), None)))
        self.Depends(build, copy_nodes)
BaseEnvironment.StageVCPKG = StageVCPKG


def RGlob(self, root_path, pattern, ondisk=True, source=False, strings=False, exclude=None):
    result_nodes = []
    paths = [root_path]
    while paths:
        path = paths.pop()
        all_nodes = self.Glob(f"{os.path.abspath(path)}/*", ondisk=ondisk, source=source, exclude=exclude)
        for entry in all_nodes:
            entry_path = os.path.abspath(getattr(entry, "path", None) or str(entry))
            is_dir = False
            if hasattr(entry, "isdir"):
                is_dir = entry.isdir()
            elif hasattr(entry, "srcnode") and callable(entry.srcnode):
                src_node = entry.srcnode()
                if src_node:
                    is_dir = src_node.isdir()
            if is_dir:
                paths.append(entry_path)
            result_nodes.append(entry_path)
    filtered_nodes = [node for node in result_nodes if fnmatch.fnmatch(os.path.basename(node), pattern)]
    return sorted(filtered_nodes)
BaseEnvironment.RGlob = RGlob

def BuildConan(self, build_type):
    subprocess.run((
        "conan", "install", ".",
        "--build=missing",
        "--output-folder=.conan",
        f"--settings=build_type={build_type}"
    ))
BaseEnvironment.BuildConan = BuildConan
