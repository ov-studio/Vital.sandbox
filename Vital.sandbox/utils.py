
import os, sys, subprocess, fnmatch, urllib.request, tarfile, shutil
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

def Exec(*args):
    command = list(args)
    try:
        result = subprocess.run(
            command,
            text=True,
            check=True,
            capture_output=True  # Capture stdout/stderr
        )
        return result
    except subprocess.CalledProcessError as e:
        error_msg = f"Failed executing: {' '.join(map(str, command))}\n"
        if e.stdout:
            error_msg += f"STDOUT: {e.stdout}\n"
        if e.stderr:
            error_msg += f"STDERR: {e.stderr}"
        Throw_Error(error_msg)
    except FileNotFoundError:
        Throw_Error(f"Command not found: {command[0]}")

def Download_Remote(url, destination):
    print("Downloading:", url)
    try:
        with urllib.request.urlopen(url, timeout=60) as r, open(destination, "wb") as f:
            while True:
                chunk = r.read(8192)
                if not chunk:
                    break
                f.write(chunk)
    except (urllib.error.URLError, socket.timeout, TimeoutError, Exception) as e:
        if os.path.exists(destination):
            os.remove(destination)
        Throw_Error(f"Download failed: {e}")

def Extract_Tar(path, destination):
    print("Unpacking:", path)
    temp_dir = destination + "_temp"
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)
    os.makedirs(temp_dir, exist_ok=True)
    with tarfile.open(path) as tar:
        tar.extractall(temp_dir)
    contents = os.listdir(temp_dir)
    if len(contents) == 1 and os.path.isdir(os.path.join(temp_dir, contents[0])):
        root_dir = os.path.join(temp_dir, contents[0])
    else:
        root_dir = temp_dir
    if os.path.exists(destination):
        shutil.rmtree(destination)
    os.makedirs(destination, exist_ok=True)
    for item in os.listdir(root_dir):
        src = os.path.join(root_dir, item)
        dst = os.path.join(destination, item)
        print("Moving:", dst)
        shutil.move(src, dst)
    shutil.rmtree(temp_dir)
    print("Extraction complete.")

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

def Build_Conan(self, build_type):
    subprocess.run((
        "conan", "install", ".",
        "--build=missing",
        "--output-folder=.conan",
        f"--settings=build_type={build_type}"
    ))
BaseEnvironment.Build_Conan = Build_Conan