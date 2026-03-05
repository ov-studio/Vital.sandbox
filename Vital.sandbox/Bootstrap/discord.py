from Bootstrap.utils import *
from Bootstrap.download import *

DISCORD_SDK_URL = "https://mega.nz/file/964FxYbB#JH3sNtOncuXcUKjn7loOJQZgAFmZLsJOpWpMDTGWZxo"

def Init_Discord(self):
    cwd = os.path.abspath(os.getcwd())
    root = os.path.join(cwd, "Vendor", "discord-sdk")
    return {
        "root": root
    }
BaseEnvironment.Init_Discord = Init_Discord

def Install_MegaCMD(self):
    os_info = Fetch_OS()
    if os_info["type"] == "Windows":
        mega_path = os.path.join(os.environ["LOCALAPPDATA"], "MEGAcmd")
        mega_exe  = os.path.join(mega_path, "mega-get.bat")
        if os.path.exists(mega_exe):
            return mega_path
        print("Installing MEGAcmd...")
        url = "https://mega.nz/MEGAcmdSetup64.exe"
        installer = os.path.join(os.environ["TEMP"], "MEGAcmdSetup64.exe")
        Download(url, installer)
        subprocess.run([installer, "/S"], check=True)
        import time; time.sleep(5)  # wait for installer to finish
        print("MEGAcmd installed.")
        return mega_path
    elif os_info["type"] == "Darwin":
        if shutil.which("mega-get"):
            return ""
        url = "https://mega.nz/MEGAcmdSetup.dmg"
        dmg = "/tmp/MEGAcmdSetup.dmg"
        Download(url, dmg)
        subprocess.run(["hdiutil", "attach", dmg], check=True)
        subprocess.run(["sudo", "installer", "-pkg", "/Volumes/MEGAcmd/MEGAcmd.pkg", "-target", "/"], check=True)
        subprocess.run(["hdiutil", "detach", "/Volumes/MEGAcmd"], check=True)
        os.remove(dmg)
        return ""
    elif os_info["type"] == "Linux":
        if shutil.which("mega-get"):
            return ""
        distro = subprocess.run(["lsb_release", "-si"], capture_output=True, text=True).stdout.strip().lower()
        if "ubuntu" in distro or "debian" in distro:
            subprocess.run(["sudo", "apt", "install", "-y", "megacmd"], check=True)
        else:
            Throw_Error("Install MEGAcmd manually from https://mega.io/cmd")
        return ""
BaseEnvironment.Install_MegaCMD = Install_MegaCMD

def Install_Discord(self):
    discord = self.Init_Discord()
    if os.path.isdir(discord["root"]):
        return

    os_info  = Fetch_OS()
    mega_dir = self.Install_MegaCMD()

    # Use full path on Windows since PATH isn't updated in current process
    if os_info["type"] == "Windows":
        mega_get = os.path.join(mega_dir, "mega-get.bat")
    else:
        mega_get = "mega-get"

    zip_path = discord["root"] + ".zip"
    print("Downloading Discord SDK from Mega...")
    subprocess.run([mega_get, DISCORD_SDK_URL, zip_path], check=True)

    Extract_Zip(zip_path, discord["root"])
    os.remove(zip_path)
    print("Discord SDK ready.")
BaseEnvironment.Install_Discord = Install_Discord

def Build_Discord(self):
    self.Install_Discord()
    if self.Args["platform_type"] == "Client":
        os_info = Fetch_OS()
        cwd = os.path.abspath(os.getcwd())
        self.Append(LIBPATH=[os.path.join(cwd, f"Vendor/discord-sdk/bin/{self.Args["build_type"].lower()}")])
        if os_info["type"] == "Windows":
            self.Append(LIBS=["discord_partner_sdk"])
BaseEnvironment.Build_Discord = Build_Discord

def Stage_Discord(self, build, build_dir):
    if self.Args["platform_type"] == "Client":
        os_info = Fetch_OS()
        cwd = os.path.abspath(os.getcwd())
        discord_bin = os.path.join(cwd, f"../Vital.sandbox/Vendor/discord-sdk/bin/{self.Args["build_type"].lower()}")
        copy_nodes = []
        if os_info["type"] == "Windows":
            copy_nodes += self.RCopy(build_dir, os.path.join(discord_bin, "discord_partner_sdk.dll"))
        elif os_info["type"] == "Darwin":
            copy_nodes += self.RCopy(build_dir, os.path.join(discord_bin, "discord_partner_sdk.dylib"))
        elif os_info["type"] == "Linux":
            copy_nodes += self.RCopy(build_dir, os.path.join(discord_bin, "discord_partner_sdk.so"))
        self.Depends(build, copy_nodes)
BaseEnvironment.Stage_Discord = Stage_Discord
