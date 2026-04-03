import sys
sys.path.append("./Vital.sandbox")
from vital import *

class Build:
    def __init__(self, script_dir, platform_type, build_type, verbose=False):
        self.script_dir = script_dir
        self.platform_type = platform_type
        self.build_type = build_type
        self.verbose = verbose
        self.os_info = Fetch_OS()
        self.info = Fetch_Build_Info()

    def init(self):
        dist_dir = os.path.join(self.script_dir, ".dist", self.build_type.lower(), self.platform_type.lower())
        return {
            "dist_dir": dist_dir,
            "output_path": os.path.join(dist_dir, f"Vital.{self.platform_type.lower()}" + self.info["output_ext"]),
            "project_dir": os.path.join(self.script_dir, f"Vital.{self.platform_type.lower()}"),
            "extension_dir": os.path.join(self.script_dir, "Vital.extension"),
            "preset": self.info["preset"].format(platform_type=self.platform_type),
            "export_mode": "--export-release" if self.build_type == "Release" else "--export-debug",
        }

    def build_godot_cpp(self, force=False):
        b = self.init()
        godot_dir = os.path.join(b["extension_dir"], "Vendor", "godot")
        stamp = os.path.join(b["extension_dir"], f".build_godotcpp.{self.build_type.lower()}")
        if force and os.path.exists(stamp):
            os.remove(stamp)
        if os.path.exists(stamp):
            log_step(f"Building godot-cpp [{self.build_type}]")
            log_info("Already built, skipping")
            return
        log_step(f"Building godot-cpp [{self.build_type}]")
        if force:
            log_info("Cleaning previous build ...")
            subprocess.run([
                "scons", "-C", godot_dir, "--clean",
                f"target=template_{self.build_type.lower()}",
                "use_static_cpp=no",
            ])
        log_info("Compiling ...")
        result = subprocess.run([
            "scons", "-C", godot_dir,
            f"target=template_{self.build_type.lower()}",
            "use_static_cpp=no",
            f"-j{int(self.os_info['nproc'])}",
        ])
        if result.returncode != 0:
            log_error("godot-cpp build failed")
            sys.exit(result.returncode)
        open(stamp, "w").close()
        log_ok("Done")

    def build_extension(self):
        import tempfile
        b = self.init()
        log_step(f"Building Vital.extension [{self.platform_type} | {self.build_type}]")

        ignore = (
            "scons: ", "WARNING:", "platform_type=", "build_type=",
            "Auto-detected", "Building for architecture",
            "Running ", "Done", "Detecting ", "Installing ", "Bootstrapping",
            "Reloading", "Fetching", "Cloning", "Binary", "Templates",
        )

        cmd = [
            "scons", "-C", b["extension_dir"],
            f"platform_type={self.platform_type}",
            f"build_type={self.build_type}",
            "build_library=no",
            f"-j{int(self.os_info['nproc'])}",
        ]

        if self.verbose:
            log_path = os.path.join(tempfile.gettempdir(), "scons_extension.log")
            with open(log_path, "w", buffering=1) as log_file:
                process = subprocess.Popen(
                    cmd,
                    stdout=log_file,
                    stderr=log_file,
                )
                stop = threading.Event()

                def tail_log():
                    with open(log_path, "r", errors="replace") as f:
                        f.seek(0, 2)
                        while not stop.is_set() or process.poll() is None:
                            line = f.readline()
                            if not line:
                                time.sleep(0.05)
                                continue
                            stripped = line.strip()
                            if not stripped:
                                continue
                            if stripped.startswith("==>"):
                                log_step(stripped[4:].strip())
                            elif any(stripped.startswith(p) for p in ignore):
                                log_info(stripped)
                            else:
                                log_ok(stripped)

                tail_thread = threading.Thread(target=tail_log)
                tail_thread.start()
                process.wait()
                stop.set()
                tail_thread.join()

        else:
            stop = threading.Event()
            thread = threading.Thread(target=spinner, args=("Compiling", stop))
            thread.start()
            log_path = os.path.join(tempfile.gettempdir(), "scons_extension.log")
            with open(log_path, "w") as log_file:
                process = subprocess.Popen(cmd, stdout=log_file, stderr=log_file)
                process.wait()
            stop.set()
            thread.join()

        if process.returncode != 0:
            log_error("Extension build failed — last 30 lines:")
            try:
                with open(log_path, "r", errors="replace") as f:
                    lines = f.readlines()
                for line in lines[-30:]:
                    if line.strip():
                        log_error(line.strip())
            except Exception:
                pass
            sys.exit(process.returncode)

        log_ok("Done")

    def copy_libs(self, project_dir, dist_dir):
        log_step(f"Copying libraries [{self.platform_type} | {self.build_type} | {self.os_info['type']}]")
        build_suffix = "release" if self.build_type == "Release" else "debug"
        lib_exts = self.info["lib_exts"]

        gdextension_roots = set()
        for root, dirs, files in os.walk(project_dir):
            if any(f.endswith(".gdextension") for f in files):
                gdextension_roots.add(os.path.normpath(root))

        for root, dirs, files in os.walk(project_dir):
            if os.path.normpath(root) in gdextension_roots:
                continue
            libs = []
            for f in files:
                name_lower = f.lower()
                if not any(name_lower.endswith(ext) for ext in lib_exts):
                    continue
                has_release = "release" in name_lower
                has_debug = "debug" in name_lower
                if (has_release or has_debug) and build_suffix not in name_lower:
                    continue
                libs.append(f)
            for lib in libs:
                shutil.copy2(os.path.join(root, lib), os.path.join(dist_dir, lib))
                log_info(f"Copied: {lib}")

    def copy_configs(self, project_dir, dist_dir):
        log_step(f"Copying config files [{self.platform_type} | {self.build_type}]")
        config_exts = [".yaml"]
        configs_copied = False

        for root, dirs, files in os.walk(project_dir):
            for f in files:
                name_lower = f.lower()
                if any(name_lower.endswith(ext) for ext in config_exts):
                    shutil.copy2(os.path.join(root, f), os.path.join(dist_dir, f))
                    log_info(f"Copied: {f}")
                    configs_copied = True

        if not configs_copied:
            log_info("No config files found")

    def export(self):
        b = self.init()
        os.makedirs(b["dist_dir"], exist_ok=True)
        log_step(f"Exporting Godot [{self.platform_type} | {self.build_type}]")
        log_info(f"Output → {b['output_path']}")
        godot_bin = Godot(None).get_bin()

        env = os.environ.copy()
        build_suffix = self.build_type.lower()
        extra_paths = [
            os.path.join(b["extension_dir"], "bin"),
            os.path.join(b["extension_dir"], f"bin/{build_suffix}"),
            os.path.join(b["project_dir"], "bin")
        ]
        env["PATH"] = os.pathsep.join(extra_paths) + os.pathsep + env.get("PATH", "")

        result = subprocess.run([
            godot_bin, "--headless",
            "--rendering-driver", "dummy",
            "--path", b["project_dir"],
            b["export_mode"], b["preset"],
            b["output_path"]
        ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, env=env)

        for line in result.stdout.splitlines():
            if line.strip():
                log_info(line.strip())

        if result.returncode != 0:
            log_error(f"Godot export failed with exit code {result.returncode}")
            sys.exit(result.returncode)

        log_ok("Done")
        self.copy_libs(b["project_dir"], b["dist_dir"])
        self.copy_configs(b["project_dir"], b["dist_dir"])


def main():
    parser = argparse.ArgumentParser(description="Build Vital")

    platform_group = parser.add_mutually_exclusive_group(required=True)
    platform_group.add_argument("--client", action="store_true")
    platform_group.add_argument("--server", action="store_true")
    platform_group.add_argument("--all", action="store_true", help="Build both client and server")

    build_group = parser.add_mutually_exclusive_group(required=True)
    build_group.add_argument("--debug", action="store_true")
    build_group.add_argument("--release", action="store_true")

    parser.add_argument("--skip-extension", action="store_true", help="Skip building Vital.extension")
    parser.add_argument("--skip-export", action="store_true", help="Skip Godot export")
    parser.add_argument("--rebuild-godot", action="store_true", help="Force rebuild of godot-cpp")
    parser.add_argument("--verbose", action="store_true", help="Stream compiler output live instead of spinner")

    args = parser.parse_args()
    build_type = "Release" if args.release else "Debug"
    script_dir = os.path.dirname(os.path.abspath(__file__))
    platforms = ["Client", "Server"] if args.all else ["Client"] if args.client else ["Server"]

    if not args.skip_extension:
        Build(script_dir, platforms[0], build_type, verbose=args.verbose).build_godot_cpp(force=args.rebuild_godot)

    for platform_type in platforms:
        build = Build(script_dir, platform_type, build_type, verbose=args.verbose)
        if not args.skip_extension:
            build.build_extension()
        if not args.skip_export:
            build.export()

    log_step("Build complete")
    log_ok("Done")


if __name__ == "__main__":
    main()