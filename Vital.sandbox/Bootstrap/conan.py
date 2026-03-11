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
        for build_type in ("Debug", "Release"):
            subprocess.run((
                "conan", "install", ".",
                "--build=missing",
                "--output-folder=.conan",
                f"--settings=build_type={build_type}",
                "--settings=compiler.cppstd=17",
                "--settings=compiler.runtime=dynamic",
            ), check=True)

BaseEnvironment.Conan = property(lambda self: Conan(self))