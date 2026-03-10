import os, sys, time, socket, argparse, subprocess, glob, fnmatch, urllib.request, tarfile, zipfile, shutil
from platform import machine, system
from multiprocessing import cpu_count
from SCons.Environment import Base as BaseEnvironment
from SCons.Script import Copy, Action

def Fetch_OS():
    archi = machine()
    if archi == "AMD64":
        archi = "x86_64"
    return {
        "type":  system(),
        "archi": archi,
        "nproc": str(cpu_count())
    }

def Fetch_Compiler():
    compiler = ["default"]
    if Fetch_OS()["type"] == "Windows":
        compiler.append("msvc")
    return compiler

def Throw_Error(msg):
    print(msg)
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