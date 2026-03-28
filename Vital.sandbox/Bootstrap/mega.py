from Bootstrap.utils import *
from Bootstrap.download import *

class Mega:
    def __init__(self, env):
        self.env = env

    def init(self):
        os_info = Fetch_OS()
        if os_info["type"] == "Windows":
            mega_path = os.path.join(os.environ["LOCALAPPDATA"], "MEGAcmd")
            return {
                "path": mega_path,
                "exe": os.path.join(mega_path, "mega-get.bat")
            }
        else:
            return {
                "path": "",
                "exe": shutil.which("mega-get") or "mega-get"
            }

    def install(self):
        os_info = Fetch_OS()
        megacmd = self.init()
        if os.path.exists(megacmd["exe"]) or shutil.which("mega-get"):
            return
        log_step("Installing MEGAcmd")
        if os_info["type"] == "Windows":
            installer = os.path.join(os.environ["TEMP"], "MEGAcmdSetup64.exe")
            Download("https://mega.nz/MEGAcmdSetup64.exe", installer)
            log_info("Running installer ...")
            result = subprocess.run([installer, "/S"], capture_output=True)
            if result.returncode != 0:
                Throw_Error(f"MEGAcmd installation failed:\n{result.stderr.decode().strip()}")
            time.sleep(5)
        elif os_info["type"] == "Darwin":
            dmg = "/tmp/MEGAcmdSetup.dmg"
            Download("https://mega.nz/MEGAcmdSetup.dmg", dmg)
            log_info("Mounting installer ...")
            subprocess.run(["hdiutil", "attach", dmg], check=True, stdout=subprocess.DEVNULL)
            subprocess.run(["sudo", "installer", "-pkg", "/Volumes/MEGAcmd/MEGAcmd.pkg", "-target", "/"], check=True, stdout=subprocess.DEVNULL)
            subprocess.run(["hdiutil", "detach", "/Volumes/MEGAcmd"], check=True, stdout=subprocess.DEVNULL)
            os.remove(dmg)
        elif os_info["type"] == "Linux":
            distro = subprocess.run(["lsb_release", "-si"], capture_output=True, text=True).stdout.strip().lower()
            if "ubuntu" in distro or "debian" in distro:
                log_info("Running apt install ...")
                subprocess.run(["sudo", "apt", "install", "-y", "megacmd"], check=True, stdout=subprocess.DEVNULL)
            else:
                Throw_Error("Install MEGAcmd manually from https://mega.io/cmd")
        log_ok("Done")

    def get(self, url, destination):
        self.install()
        megacmd = self.init()
        log_info(f"Fetching {os.path.basename(destination)} ...")
        result = subprocess.run([megacmd["exe"], url, destination], capture_output=True)
        if result.returncode != 0:
            Throw_Error(f"Mega download failed:\n{result.stderr.decode().strip()}")

BaseEnvironment.Mega = property(lambda self: Mega(self))
