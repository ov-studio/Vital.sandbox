from Bootstrap.utils import *

def Build_Discord(self):
    if self.Args["platform_type"] == "Client":
        os_info = Fetch_OS()
        cwd = os.path.abspath(os.getcwd())
        self.Append(LIBPATH=[os.path.join(cwd, f"Vendor/discord-sdk/bin/{self.Args["build_type"].lower()}")])
        if os_info["type"] == "Windows":
            self.Append(LIBS=["discord_partner_sdk"])
BaseEnvironment.Build_Discord = Build_Discord

def Stage_Discord(self, build):
    if self.Args["platform_type"] == "Client":
        print("stage discord")
BaseEnvironment.Stage_Discord = Stage_Discord
