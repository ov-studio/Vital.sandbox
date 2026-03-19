from Bootstrap.utils import *

class Conan:
    def __init__(self, env):
        self.env = env

    def install(self):
        if not shutil.which("conan"):
            print("\n==> Installing conan")
            subprocess.run(
                [sys.executable, "-m", "pip", "install", "--upgrade", "conan"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=True
            )

    def build(self):
        self.install()
        print("\n==> Building conan dependencies")
        for cmd, label in [
            (["conan", "profile", "detect", "--force"], "Detecting profile"),
        ] + [
            (["conan", "install", ".",
                "--build=missing",
                "--output-folder=.conan",
                f"--settings=build_type={build_type}",
                "--settings=compiler.cppstd=17",
                "--settings=compiler.runtime=dynamic",
            ], f"Installing [{build_type}]")
            for build_type in ("Debug", "Release")
        ]:
            print(f"  {label} ...")
            result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
            if result.returncode != 0:
                print(f"  [ERROR] {label} failed:")
                print(result.stderr.decode().strip())
                sys.exit(result.returncode)
        print("  Done")

BaseEnvironment.Conan = property(lambda self: Conan(self))