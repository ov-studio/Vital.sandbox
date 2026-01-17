from Bootstrap.utils import *

def Build_Vendor(self):
    os_info = Fetch_OS()
    git = shutil.which("git")
    if not git:
        Throw_Error("git not found")
    script = os.path.abspath(os.path.join(os.path.dirname(__file__), "..\\..", ".gitreload.sh"))
    if os_info["type"] == "Windows":
        git_root = os.path.abspath(os.path.join(os.path.dirname(git), ".."))
        bash = os.path.join(git_root, "usr", "bin", "bash.exe")
        subprocess.run([bash, script], check=True)
    else:
        subprocess.run([script], check=True)
BaseEnvironment.Build_Vendor = Build_Vendor