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

def Build_CEF(self):
    cef = self.Init_CEF()
    self.Install_CEF()
    os.chdir(cef["root"])
    if os_info["type"] == "Windows":
        Exec("cmake", "-DCEF_RUNTIME_LIBRARY_FLAG=/MD", "-DCMAKE_BUILD_TYPE=" + self.Args["build_type"], ".")
        Exec("cmake", "--build", ".", "--config", self.Args["build_type"])
    elif os_info["type"] == "Darwin":
        os.mkdir("build")
        os.chdir("build")
        Exec("cmake", "-G", "Ninja", "-DPROJECT_ARCH=" + os_info["archi"], "-DCMAKE_BUILD_TYPE=" + self.Args["build_type"], "..")
        Exec("ninja", "-v", "-j" + os_info["nproc"], "cefsimple")
    else:
        os.mkdir("build")
        os.chdir("build")
        if shutil.which('ninja') is not None:
            Exec("cmake", "-G", "Ninja", "-DCMAKE_BUILD_TYPE=" + self.Args["build_type"], "..")
            Exec("ninja", "-v", "-j" + os_info["nproc"], "cefsimple")
        else:
            Exec("cmake", "-G", "Unix Makefiles", "-DCMAKE_BUILD_TYPE=" + self.Args["build_type"], "..")
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

def Stage_CEF(self, build):
    cef = self.Init_CEF()
    out_dir = os.path.dirname(str(build[0].abspath))
    locales = os.path.join(out_dir, "locales")
    if not os.path.isdir(locales):
        os.mkdir(locales)
    copy_nodes = []
    if os_info["type"] in ("Linux", "Windows"):
        S = os.path.join(cef["root"], "Resources")
        copy_nodes.append(
            self.Command(
                os.path.join(out_dir, "icudtl.dat"),
                os.path.join(S, "icudtl.dat"),
                Action(Copy("$TARGET", "$SOURCE"), None),
            )
        )
        for f in glob.glob(os.path.join(S, "*.pak")):
            copy_nodes.append(
                self.Command(
                    os.path.join(out_dir, os.path.basename(f)),
                    f,
                    Action(Copy("$TARGET", "$SOURCE"), None),
                )
            )
        for f in glob.glob(os.path.join(S, "locales", "*")):
            copy_nodes.append(
                self.Command(
                    os.path.join(locales, os.path.basename(f)),
                    f,
                    Action(Copy("$TARGET", "$SOURCE"), None),
                )
            )
        S = os.path.join(cef["root"], self.Args["build_type"])
        copy_nodes.append(
            self.Command(
                os.path.join(out_dir, "vk_swiftshader_icd.json"),
                os.path.join(S, "vk_swiftshader_icd.json"),
                Action(Copy("$TARGET", "$SOURCE"), None),
            )
        )
        for f in glob.glob(os.path.join(S, "*snapshot*.bin")):
            copy_nodes.append(
                self.Command(
                    os.path.join(out_dir, os.path.basename(f)),
                    f,
                    Action(Copy("$TARGET", "$SOURCE"), None),
                )
            )
        if os_info["type"] == "Linux":
            for f in glob.glob(os.path.join(S, "*.so")) + glob.glob(os.path.join(S, "*.so.*")):
                copy_nodes.append(
                    self.Command(
                        os.path.join(out_dir, os.path.basename(f)),
                        f,
                        Action(Copy("$TARGET", "$SOURCE"), None),
                    )
                )
        else:
            for f in glob.glob(os.path.join(S, "*.dll")):
                copy_nodes.append(
                    self.Command(
                        os.path.join(out_dir, os.path.basename(f)),
                        f,
                        Action(Copy("$TARGET", "$SOURCE"), None),
                    )
                )
    elif os_info["type"] == "Darwin":
        S = os.path.join(
            cef["root"],
            "build",
            "tests",
            "cefsimple",
            self.Args["build_type"],
            "cefsimple.app",
        )
        copy_nodes.append(
            self.Command(
                os.path.join(out_dir, "cefsimple.app"),
                S,
                Action(CopyTree("$TARGET", "$SOURCE"), None),
            )
        )
        for f in glob.glob(os.path.join(S, "locales", "*")):
            copy_nodes.append(
                self.Command(
                    os.path.join(locales, os.path.basename(f)),
                    f,
                    Action(Copy("$TARGET", "$SOURCE"), None),
                )
            )
    else:
        Throw_Error("Unknown OS " + os_info["type"] + ": Cannot extract CEF artifacts")
    self.Depends(build, copy_nodes)
BaseEnvironment.Stage_CEF = Stage_CEF