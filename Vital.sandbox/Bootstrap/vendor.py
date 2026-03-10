from Bootstrap.utils import *

class Vendor:
    def __init__(self, env):
        self.env = env

    def build(self):
        os_info = Fetch_OS()
        git = shutil.which("git")
        if not git:
            Throw_Error("git not found")
        script = os.path.abspath(os.path.join(os.path.dirname(__file__), "..\\..", ".gitreload.sh"))
        if os_info["type"] == "Windows":
            bash = os.path.join(os.path.abspath(os.path.join(os.path.dirname(git), "..")), "usr", "bin", "bash.exe")
            subprocess.run([bash, script], check=True)
        else:
            subprocess.run([script], check=True)

BaseEnvironment.Vendor = property(lambda self: Vendor(self))