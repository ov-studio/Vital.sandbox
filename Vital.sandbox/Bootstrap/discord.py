from Bootstrap.utils import *
from Bootstrap.download import *
from Bootstrap.mega import *

DISCORD_SDK_URL = "https://mega.nz/file/964FxYbB#JH3sNtOncuXcUKjn7loOJQZgAFmZLsJOpWpMDTGWZxo"

def Init_Discord(self):
    cwd = os.path.abspath(os.getcwd())
    root = os.path.join(cwd, "Vendor", "discord-sdk")
    return { "root": root }
BaseEnvironment.Init_Discord = Init_Discord

def Install_Discord(self):
    discord = self.Init_Discord()
    if os.path.isdir(discord["root"]):
        return
    zip_path = discord["root"] + ".zip"
    print("Downloading Discord SDK from Mega...")
    self.Mega_Get(DISCORD_SDK_URL, zip_path)
    Extract_Zip(zip_path, discord["root"])
    os.remove(zip_path)
    print("Discord SDK ready.")
BaseEnvironment.Install_Discord = Install_Discord

def Build_Discord(self):
    self.Install_Discord()
    if self.Args["platform_type"] == "Client":
        os_info = Fetch_OS()
        cwd = os.path.abspath(os.getcwd())
        self.Append(LIBPATH=[os.path.join(cwd, f"Vendor/discord-sdk/bin/{self.Args['build_type'].lower()}")])
        self.Append(LIBPATH=[os.path.join(cwd, f"Vendor/discord-sdk/lib/{self.Args['build_type'].lower()}")])
        if os_info["type"] == "Windows":
            self.Append(LIBS=["discord_partner_sdk"])
BaseEnvironment.Build_Discord = Build_Discord

def Stage_Discord(self, build, build_dir):
    if self.Args["platform_type"] == "Client":
        os_info = Fetch_OS()
        cwd = os.path.abspath(os.getcwd())
        discord_bin = os.path.join(cwd, f"../Vital.sandbox/Vendor/discord-sdk/bin/{self.Args['build_type'].lower()}")
        copy_nodes = []
        if os_info["type"] == "Windows":
            copy_nodes += self.RCopy(build_dir, os.path.join(discord_bin, "discord_partner_sdk.dll"))
        elif os_info["type"] == "Darwin":
            copy_nodes += self.RCopy(build_dir, os.path.join(discord_bin, "discord_partner_sdk.dylib"))
        elif os_info["type"] == "Linux":
            copy_nodes += self.RCopy(build_dir, os.path.join(discord_bin, "discord_partner_sdk.so"))
        self.Depends(build, copy_nodes)
BaseEnvironment.Stage_Discord = Stage_Discord