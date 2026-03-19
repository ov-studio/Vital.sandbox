from Bootstrap.utils import *
from Bootstrap.download import *
from Bootstrap.mega import *

DISCORD_SDK_URL = "https://mega.nz/file/964FxYbB#JH3sNtOncuXcUKjn7loOJQZgAFmZLsJOpWpMDTGWZxo"

class Discord:
    def __init__(self, env):
        self.env = env

    def init(self):
        cwd = os.path.abspath(os.getcwd())
        return { "root": os.path.join(cwd, "Vendor", "discord-sdk") }

    def install(self):
        discord = self.init()
        if os.path.isdir(discord["root"]):
            return
        log_step("Installing Discord SDK")
        zip_path = discord["root"] + ".zip"
        self.env.Mega.get(DISCORD_SDK_URL, zip_path)
        Extract_Zip(zip_path, discord["root"])
        os.remove(zip_path)
        log_ok("Done")

    def build(self):
        self.install()
        if self.env.Args["platform_type"] != "Client":
            return
        os_info = Fetch_OS()
        cwd = os.path.abspath(os.getcwd())
        build_type = self.env.Args["build_type"].lower()
        self.env.Append(LIBPATH=[os.path.join(cwd, f"Vendor/discord-sdk/bin/{build_type}")])
        self.env.Append(LIBPATH=[os.path.join(cwd, f"Vendor/discord-sdk/lib/{build_type}")])
        if os_info["type"] == "Windows":
            self.env.Append(LIBS=["discord_partner_sdk"])

    def stage(self, build, build_dir):
        if self.env.Args["platform_type"] != "Client":
            return
        os_info = Fetch_OS()
        cwd = os.path.abspath(os.getcwd())
        discord_bin = os.path.join(cwd, f"../Vital.sandbox/Vendor/discord-sdk/bin/{self.env.Args['build_type'].lower()}")
        copy_nodes = []
        if os_info["type"] == "Windows":
            copy_nodes += self.env.RCopy(build_dir, os.path.join(discord_bin, "discord_partner_sdk.dll"))
        elif os_info["type"] == "Darwin":
            copy_nodes += self.env.RCopy(build_dir, os.path.join(discord_bin, "discord_partner_sdk.dylib"))
        elif os_info["type"] == "Linux":
            copy_nodes += self.env.RCopy(build_dir, os.path.join(discord_bin, "discord_partner_sdk.so"))
        self.env.Depends(build, copy_nodes)

BaseEnvironment.Discord = property(lambda self: Discord(self))
