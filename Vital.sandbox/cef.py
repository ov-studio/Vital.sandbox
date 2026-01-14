import os, sys, subprocess, hashlib, glob, urllib.request
from utils import *
from SCons.Environment import Base as BaseEnvironment
from SCons.Script import Copy, Action

os_info = Fetch_OS()
cef_version = "143.0.13+g30cb3bd+chromium-143.0.7499.170" #https://cef-builds.spotifycdn.com/index.html

def Init_CEF(self):
    root = os.path.join(os.path.abspath(os.path.dirname(__file__)), ".cef")
    archi = None
    version = cef_version
    if os_info["type"] == "Windows":
        if os_info["archi"] == "x86_64" or os_info["archi"] == "AMD64":
            archi = "windows64"
        else:
            archi = "windowsarm64"
    elif os_info["type"] == "Darwin":
        if os_info["archi"] == "x86_64":
            archi = "macosx64"
        else:
            archi = "macosarm64"
    elif os_info["type"] == "Linux":
        if os_info["archi"] == "x86_64":
            archi = "linux64"
        else:
            archi = "linuxarm"
    identifier = "cef_binary_" + version.replace("+", "%2B") + "_" + archi + ".tar.bz2"
    return {
        "root" : root,
        "archi" : archi,
        "version" : version,
        "identifier" : identifier,
        "url" : "https://cef-builds.spotifycdn.com/" + identifier
    }
BaseEnvironment.Init_CEF = Init_CEF

def Install_CEF(self):
    cef = self.Init_CEF()
    if not os.path.exists(cef["root"]):
        os.makedirs(cef["root"], exist_ok=True)
        Download_Remote(cef["url"], cef["root"] + "/" + cef["identifier"])
        Extract_Tar(cef["root"] + "/" + cef["identifier"], cef["root"])
BaseEnvironment.Install_CEF = Install_CEF

def Build_CEF(self, build_type):
    cef = self.Init_CEF()
    self.Install_CEF()
    os.chdir(cef["root"])
    if os_info["type"] == "Windows":
        Exec("cmake", "-DCEF_RUNTIME_LIBRARY_FLAG=/MD", "-DCMAKE_BUILD_TYPE=" + build_type, ".")
        Exec("cmake", "--build", ".", "--config", build_type)
    elif os_info["type"] == "Darwin":
        os.mkdir("build")
        os.chdir("build")
        Exec("cmake", "-G", "Ninja", "-DPROJECT_ARCH=" + os_info["archi"], "-DCMAKE_BUILD_TYPE=" + build_type, "..")
        Exec("ninja", "-v", "-j" + os_info["nproc"], "cefsimple")
    else:
        os.mkdir("build")
        os.chdir("build")
        if shutil.which('ninja') is not None:
            Exec("cmake", "-G", "Ninja", "-DCMAKE_BUILD_TYPE=" + build_type, "..")
            Exec("ninja", "-v", "-j" + os_info["nproc"], "cefsimple")
        else:
            Exec("cmake", "-G", "Unix Makefiles", "-DCMAKE_BUILD_TYPE=" + build_type, "..")
            Exec("make", "cefsimple", "-j" + os_info["nproc"])

    """
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
    """
BaseEnvironment.Build_CEF = Build_CEF