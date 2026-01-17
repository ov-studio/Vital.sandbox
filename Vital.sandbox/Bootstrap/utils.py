
import os, sys, subprocess, glob, fnmatch, urllib.request, tarfile, shutil
from platform import machine, system
from multiprocessing import cpu_count
from SCons.Environment import Base as BaseEnvironment
from SCons.Script import Copy, Action

def Fetch_OS():
    type = system()
    archi = machine()
    if archi == "AMD64":
        archi = "x86_64"
    return {
        "type" : type,
        "archi" : archi,
        "nproc" : str(cpu_count())
    }

def Throw_Error(msg):
    print(msg)
    sys.exit(2)

def Fetch_Compiler():
    os_info = Fetch_OS()
    compiler = ["default"]
    if os_info["type"] == "Windows":
        compiler.append("msvc")
    return compiler

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

def RCopy(self, destination, src):
    dst = os.path.join(destination, os.path.basename(src))
    return self.Command(dst, src, Action(Copy("$TARGET", "$SOURCE"), None))
BaseEnvironment.RCopy = RCopy

def RGlobCopy(self, destination, pattern):
    nodes = []
    for f in glob.glob(pattern):
        nodes.append(self.RCopy(destination, f))
    return nodes
BaseEnvironment.RGlobCopy = RGlobCopy