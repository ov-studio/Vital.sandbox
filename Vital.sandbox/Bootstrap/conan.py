from Bootstrap.utils import *

def Install_Conan(self):
    if not shutil.which("conan"):
        subprocess.run(
            [sys.executable, "-m", "pip", "install", "--upgrade", "conan"],
            stdout = subprocess.DEVNULL,
            stderr = subprocess.DEVNULL,
            check = True
        )
BaseEnvironment.Install_Conan = Install_Conan

def Build_Conan(self):
    self.Install_Conan()
    subprocess.run((
        "conan", "install", ".",
        "--build=missing",
        "--output-folder=.conan",
        f"--settings=build_type={self.Args["build_type"]}"
    ))
BaseEnvironment.Build_Conan = Build_Conan