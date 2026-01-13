import os
import sys
import fnmatch
import subprocess
from SCons.Environment import Base as BaseEnvironment

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

def BuildVCPKG(self, platform_type):
    vcpkg = self.VCPKG()
    self.InstallVCPKG()
    self.Append(CPPDEFINES={("RML_LUA_BINDINGS_LIBRARY", "lua")})
    self.Append(CPPPATH=[os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "include")])
    self.Append(LIBPATH=[os.path.join(vcpkg["root"], "installed", vcpkg["triplet"], "lib")])
    self.Append(LIBS=[
        "rmlui",
        "lua",
        "rmlui_lua",
        "zlib",
        "libpng16",
        "freetype",
        "bz2",
        "brotlienc",
        "brotlidec",
        "brotlicommon"
    ])
BaseEnvironment.BuildVCPKG = BuildVCPKG

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
