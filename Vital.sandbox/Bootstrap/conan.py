from Bootstrap.utils import *
import importlib.util

CONAN = [sys.executable, "-m", "conans.conan"]

class Conan:
    def __init__(self, env):
        self.env = env

    def install(self):
        if importlib.util.find_spec("conan") is None:
            log_step("Installing Conan")
            subprocess.run(
                [sys.executable, "-m", "pip", "install", "--upgrade", "conan"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=True
            )

    def _run_live(self, cmd, label):
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True
        )
        for line in process.stdout:
            stripped = line.strip()
            if stripped:
                log_info(stripped)
        process.wait()
        return process.returncode

    def build(self):
        self.install()
        log_step("Building Conan")
        for cmd, label in [
            ([*CONAN, "profile", "detect", "--force"], "Detecting profile"),
            ([*CONAN, "install", ".",
                "--build=missing",
                "-v", "quiet",
                "--output-folder=.conan",
                "--settings=build_type=Release",
                "--settings=compiler.cppstd=17",
                "--settings=compiler.runtime=dynamic",
                "--settings=compiler.version=194"
            ], "Installing packages")
        ]:
            log_info(f"{label} ...")
            returncode = self._run_live(cmd, label)
            if returncode != 0:
                log_error(f"{label} failed (exit code {returncode})")
                sys.exit(returncode)
        log_ok("Done")

BaseEnvironment.Conan = property(lambda self: Conan(self))