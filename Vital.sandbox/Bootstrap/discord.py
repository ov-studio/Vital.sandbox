from Bootstrap.utils import *
from Bootstrap.download import *

DISCORD_SDK_URL = "https://github.com/ov-studio/Vital.sandbox/releases/download/deps/discord-sdk.zip"

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
        Download(DISCORD_SDK_URL, zip_path)
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
        elif os_info["type"] == "Linux":
            self.env.Append(LIBS=["discord_partner_sdk"])
            # The runtime is staged beside the extension, not on the system path
            self.env.Append(LINKFLAGS=["-Wl,-rpath,$$ORIGIN"])

    # The SDK ships the Windows runtime under bin/, but the Linux and macOS
    # ones under lib/ with a "lib" prefix
    RUNTIME = {
        "Windows": ("bin", "discord_partner_sdk.dll"),
        "Linux":   ("lib", "libdiscord_partner_sdk.so"),
        "Darwin":  ("lib", "libdiscord_partner_sdk.dylib"),
    }

    def stage(self, build, build_dir):
        if self.env.Args["platform_type"] != "Client":
            return
        runtime = self.RUNTIME.get(Fetch_OS()["type"])
        if not runtime:
            return
        subdir, filename = runtime
        cwd = os.path.abspath(os.getcwd())
        build_type = self.env.Args["build_type"].lower()
        source = os.path.join(cwd, "Vendor", "discord-sdk", subdir, build_type, filename)
        self.env.Depends(build, self.env.RCopy(build_dir, source))

BaseEnvironment.Discord = property(lambda self: Discord(self))