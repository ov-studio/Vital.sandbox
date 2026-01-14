import os, sys, subprocess, hashlib, glob, urllib.request
from utils import *
from SCons.Environment import Base as BaseEnvironment
from SCons.Script import Copy, Action

os_info = Fetch_OS()
cef_version = "143.0.13+g30cb3bd+chromium-143.0.7499.170" #https://cef-builds.spotifycdn.com/index.html

def Init_CEF(self):
    root = os.path.join(os.path.abspath(os.path.dirname(__file__)), "Vendor", ".cef")
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
    else:
        Throw_Error("CEF not supported on " + os_info["type"])
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
        if not shutil.which("cmake"):
            Throw_Error("You need to install the 'cmake' tool")
        Exec("cmake", "-DCEF_RUNTIME_LIBRARY_FLAG=/MD", "-DCMAKE_BUILD_TYPE=" + self.Args["build_type"], ".")
        Exec("cmake", "--build", ".", "--config", self.Args["build_type"])
    elif os_info["type"] == "Darwin":
        if (shutil.which("ninja") is None and OSTYPE == "Darwin"):
            Throw_Error("You need to install the 'ninja' tool for macOS builds")
        os.mkdir("build")
        os.chdir("build")
        Exec("cmake", "-G", "Ninja", "-DPROJECT_ARCH=" + os_info["archi"], "-DCMAKE_BUILD_TYPE=" + self.Args["build_type"], "..")
        Exec("ninja", "-v", "-j" + os_info["nproc"], "cefsimple")
        self.Append(CPPDEFINES=['CEF_USE_SANDBOX', 'WRAPPING_CEF_SHARED', '__STDC_CONSTANT_MACROS', '__STDC_FORMAT_MACROS'])
    else:
        if not (shutil.which("ninja") or shutil.which("make")):
            Throw_Error("You need to install either the 'ninja' or GNU Make tool")
        os.mkdir("build")
        os.chdir("build")
        if shutil.which("ninja") is not None:
            Exec("cmake", "-G", "Ninja", "-DCMAKE_BUILD_TYPE=" + self.Args["build_type"], "..")
            Exec("ninja", "-v", "-j" + os_info["nproc"], "cefsimple")
        else:
            Exec("cmake", "-G", "Unix Makefiles", "-DCMAKE_BUILD_TYPE=" + self.Args["build_type"], "..")
            Exec("make", "cefsimple", "-j" + os_info["nproc"])
        env.Append(CPPDEFINES=['CEF_USE_SANDBOX', '_FILE_OFFSET_BITS=64', '__STDC_CONSTANT_MACROS', '__STDC_FORMAT_MACROS'])
    self.Append(CPPPATH=[cef["root"]])
    self.Append(LIBPATH=[cef["root"] + "/" + self.Args["build_type"]])
    self.Append(LIBS=["libcef"])
BaseEnvironment.Build_CEF = Build_CEF

def Stage_CEF(self, build):
    cef = self.Init_CEF()
    out_dir = os.path.dirname(str(build[0].abspath))
    locales = os.path.join(out_dir, "locales")
    copy_nodes = []
    if not os.path.isdir(locales):
        os.mkdir(locales)
    if os_info["type"] in ("Linux", "Windows"):
        R = os.path.join(cef["root"], "Resources")
        B = os.path.join(cef["root"], self.Args["build_type"])
        copy_nodes.append(
            self.Command(
                os.path.join(out_dir, "icudtl.dat"),
                os.path.join(R, "icudtl.dat"),
                Action(Copy("$TARGET", "$SOURCE"), None),
            )
        )
        copy_nodes += self.RGlobCopy(out_dir, os.path.join(R, "*.pak"))
        copy_nodes += self.RGlobCopy(locales, os.path.join(R, "locales", "*"))
        copy_nodes.append(
            self.Command(
                os.path.join(out_dir, "vk_swiftshader_icd.json"),
                os.path.join(B, "vk_swiftshader_icd.json"),
                Action(Copy("$TARGET", "$SOURCE"), None),
            )
        )
        copy_nodes += self.RGlobCopy(out_dir, os.path.join(B, "*snapshot*.bin"))
        if os_info["type"] == "Linux":
            copy_nodes += self.RGlobCopy(out_dir, os.path.join(B, "*.so"))
            copy_nodes += self.RGlobCopy(out_dir, os.path.join(B, "*.so.*"))
        else:
            copy_nodes += self.RGlobCopy(out_dir, os.path.join(B, "*.dll"))
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
        copy_nodes += self.RGlobCopy(locales, os.path.join(S, "locales", "*"))
    self.Depends(build, copy_nodes)
BaseEnvironment.Stage_CEF = Stage_CEF