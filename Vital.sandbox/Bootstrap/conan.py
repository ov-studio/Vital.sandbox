from Bootstrap.utils import *

class Conan:
    def __init__(self, env):
        self.env = env

    def install(self):
        if not shutil.which("conan"):
            subprocess.run(
                [sys.executable, "-m", "pip", "install", "--upgrade", "conan"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=True
            )

    def build(self):
        self.install()
        subprocess.run(("conan", "profile", "detect", "--force"), check=True)
        subprocess.run((
            "conan", "install", ".",
            "--build=missing",
            "--output-folder=.conan",
            f"--settings=build_type={self.env.Args['build_type']}"
        ))

BaseEnvironment.Conan = property(lambda self: Conan(self))