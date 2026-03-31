from Bootstrap.utils import *

class Vendor:
    def __init__(self, env):
        self.env = env

    def build(self):
        os_info = Fetch_OS()
        log_step("Reloading Vendors")
        git = shutil.which("git")
        if not git:
            Throw_Error("git not found")
        script = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".gitreload.sh"))
        log_info("Running .gitreload.sh ...")
        if os_info["type"] == "Windows":
            bash = os.path.join(os.path.abspath(os.path.join(os.path.dirname(git), "..")), "usr", "bin", "bash.exe")
            result = subprocess.run([bash, script], capture_output=True)
        else:
            result = subprocess.run([script], capture_output=True)
        if result.returncode != 0:
            indented = "\n".join(f"    {line}" for line in result.stderr.decode().strip().splitlines())
            Throw_Error(f"Vendor reload failed:\n{indented}")
        log_ok("Done")

BaseEnvironment.Vendor = property(lambda self: Vendor(self))