from Bootstrap.utils import *

def Build_Discord(self):
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
