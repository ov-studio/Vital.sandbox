import os, sys, time, socket, argparse, subprocess, glob, fnmatch, urllib.request, tarfile, zipfile, shutil
from platform import machine, system
from multiprocessing import cpu_count
from SCons.Environment import Base as BaseEnvironment
from SCons.Script import Copy, Action

BUILD_INFO = {
    "Windows": {
        "lib_exts": [".dll"],
        "preset": "Windows {platform_type}",
        "output_ext": ".exe",
    },
    "Linux": {
        "lib_exts": [".so"],
        "preset": "Linux/X11 {platform_type}",
        "output_ext": "",
    },
    "Darwin": {
        "lib_exts": [".dylib", ".framework"],
        "preset": "macOS {platform_type}",
        "output_ext": ".app",
    },
}

def _supports_color():
    return hasattr(sys.stdout, "isatty") and sys.stdout.isatty()

class C:
    RESET   = "\033[0m"     if _supports_color() else ""
    BOLD    = "\033[1m"     if _supports_color() else ""
    HEADER  = "\033[38;5;111m" if _supports_color() else ""  # soft blue header text
    GREEN   = "\033[38;5;114m" if _supports_color() else ""  # muted green
    YELLOW  = "\033[38;5;221m" if _supports_color() else ""  # warm yellow
    RED     = "\033[38;5;203m" if _supports_color() else ""  # soft red
    DIM     = "\033[2m"     if _supports_color() else ""

def log_step(msg):
    print(f"\n{C.BOLD}{C.HEADER}==>{C.RESET} {C.BOLD}{C.HEADER}{msg}{C.RESET}")

def log_info(msg):
    print(f"  {C.DIM}{msg}{C.RESET}")

def log_ok(msg):
    print(f"  {C.GREEN}{msg}{C.RESET}")

def log_warn(msg):
    print(f"  {C.YELLOW}[WARN]{C.RESET} {msg}")

def log_error(msg):
    print(f"  {C.RED}[ERROR]{C.RESET} {msg}")

def Fetch_OS():
    archi = machine()
    if archi == "AMD64":
        archi = "x86_64"
    return {
        "type":  system(),
        "archi": archi,
        "nproc": str(cpu_count())
    }

def Fetch_Build_Info():
    os_type = Fetch_OS()["type"]
    if os_type not in BUILD_INFO:
        Throw_Error(f"Unsupported platform: {os_type}")
    return BUILD_INFO[os_type]

def Fetch_Compiler():
    compiler = ["default"]
    if Fetch_OS()["type"] == "Windows":
        compiler.append("msvc")
    return compiler

def Throw_Error(msg):
    log_error(msg)
    sys.exit(2)

def _RGlob(self, root_path, pattern, ondisk=True, source=False, exclude=None):
    result_nodes = []
    paths = [root_path]
    while paths:
        path = paths.pop()
        for entry in self.Glob(f"{os.path.abspath(path)}/*", ondisk=ondisk, source=source, exclude=exclude):
            entry_path = os.path.abspath(getattr(entry, "path", None) or str(entry))
            is_dir = (entry.isdir() if hasattr(entry, "isdir") else
                      entry.srcnode().isdir() if hasattr(entry, "srcnode") and entry.srcnode() else False)
            if is_dir:
                paths.append(entry_path)
            result_nodes.append(entry_path)
    return sorted(node for node in result_nodes if fnmatch.fnmatch(os.path.basename(node), pattern))

def _RCopy(self, destination, src):
    return self.Command(os.path.join(destination, os.path.basename(src)), src, Action(Copy("$TARGET", "$SOURCE"), None))

def _RGlobCopy(self, destination, pattern):
    return [self.RCopy(destination, f) for f in glob.glob(pattern)]

BaseEnvironment.RGlob = _RGlob
BaseEnvironment.RCopy = _RCopy
BaseEnvironment.RGlobCopy = _RGlobCopy
