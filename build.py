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
            "sandbox_dir": os.path.join(self.script_dir, "Vital.sandbox"),
            "preset": self.info["preset"].format(platform_type=self.platform_type),
            "export_mode": "--export-release" if self.build_type == "Release" else "--export-debug",
        }

    def reload_vendors(self):
        b = self.init()
        if b["sandbox_dir"] not in sys.path:
            sys.path.insert(0, b["sandbox_dir"])
        from Bootstrap.vendor import Vendor
        Vendor(None).build()

    def build_godot_cpp(self, force=False):
        b = self.init()
        godot_dir = os.path.join(b["sandbox_dir"], "Vendor", "godot-cpp")
        stamp = os.path.join(b["sandbox_dir"], f".build_godotcpp.{self.build_type.lower()}")
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
                "use_static_crt=no"
            ])
        log_info("Compiling ...")
        result = subprocess.run([
            "scons", "-C", godot_dir,
            f"target=template_{self.build_type.lower()}",
            "use_static_cpp=no",
            "use_static_crt=no",
            f"-j{int(self.os_info['nproc'])}",
        ])
        if result.returncode != 0:
            log_error("godot-cpp build failed")
            sys.exit(result.returncode)
        open(stamp, "w").close()
        log_ok("Done")

    def build_sandbox(self):
        import tempfile
        b = self.init()
        log_step(f"Building Vital.sandbox [{self.platform_type} | {self.build_type}]")

        ignore = (
            "scons: ", "WARNING:", "platform_type=", "build_type=",
            "Auto-detected", "Building for architecture",
            "Running ", "Done", "Detecting ", "Installing ", "Bootstrapping",
            "Reloading", "Fetching", "Cloning", "Binary", "Templates",
        )

        cmd = [
            "scons", "-C", b["sandbox_dir"],
            f"platform_type={self.platform_type}",
            f"build_type={self.build_type}",
            f"target=template_{self.build_type.lower()}",
            "use_static_cpp=no",
            "use_static_crt=no",
            "build_library=no",
            f"debug_symbols={'yes' if self.build_type == 'Debug' else 'no'}",
            f"-j{int(self.os_info['nproc'])}",
        ]

        if self.verbose:
            log_path = os.path.join(tempfile.gettempdir(), "scons_extension.log")
            with open(log_path, "w", buffering=1) as log_file:
                process = subprocess.Popen(cmd, stdout=log_file, stderr=log_file)
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

    def stage_lib(self):
        b = self.init()
        build_suffix = self.build_type.lower()
        os_type = self.os_info["type"]

        lib_info = {
            "Windows": ("windows", f"vital.sdk.{build_suffix}.x86_64.dll"),
            "Linux":   ("linux",   f"vital.sdk.{build_suffix}.x86_64.so"),
            "Darwin":  ("macos",   f"vital.sdk.{build_suffix}.dylib"),
        }.get(os_type)

        if not lib_info:
            return

        subdir, lib_name = lib_info
        lib_src = os.path.join(b["sandbox_dir"], "bin", build_suffix, lib_name)
        lib_dst_dir = os.path.join(b["project_dir"], "vital.sdk", subdir)
        lib_dst = os.path.join(lib_dst_dir, lib_name)

        if os.path.exists(lib_src) and not os.path.exists(lib_dst):
            log_info(f"Staging {lib_name} to project folder ...")
            os.makedirs(lib_dst_dir, exist_ok=True)
            shutil.copy2(lib_src, lib_dst)

    def copy_config(self):
        b = self.init()
        log_step(f"Copying config files [{self.platform_type} | {self.build_type}]")
        config_exts = [".yaml"]
        configs_copied = False

        for root, dirs, files in os.walk(b["project_dir"]):
            for f in files:
                name_lower = f.lower()
                if any(name_lower.endswith(ext) for ext in config_exts):
                    shutil.copy2(os.path.join(root, f), os.path.join(b["dist_dir"], f))
                    log_info(f"Copied: {f}")
                    configs_copied = True

        if not configs_copied:
            log_info("No config files found")

    def copy_lib(self):
        b = self.init()
        log_step(f"Copying libraries [{self.platform_type} | {self.build_type} | {self.os_info['type']}]")
        build_suffix = "release" if self.build_type == "Release" else "debug"
        lib_exts = self.info["lib_exts"]

        gdextension_roots = set()
        for root, dirs, files in os.walk(b["project_dir"]):
            if any(f.endswith(".gdextension") for f in files):
                gdextension_roots.add(os.path.normpath(root))

        for root, dirs, files in os.walk(b["project_dir"]):
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
                shutil.copy2(os.path.join(root, lib), os.path.join(b["dist_dir"], lib))
                log_info(f"Copied: {lib}")

    def copy_pdb(self):
        b = self.init()
        log_step(f"Copying PDB files [{self.platform_type} | {self.build_type} | {self.os_info['type']}]")

        platform_subdir = {
            "Windows": "windows",
            "Linux":   "linux",
            "Darwin":  "macos",
        }.get(self.os_info["type"])

        if not platform_subdir:
            log_info("PDB copying not supported on this platform")
            return

        bin_dir = os.path.join(b["sandbox_dir"], ".bin", self.platform_type.lower(), platform_subdir)
        if not os.path.isdir(bin_dir):
            log_info(f"Bin directory not found, skipping: {bin_dir}")
            return

        pdbs_copied = False
        for f in os.listdir(bin_dir):
            if f.lower().endswith(".pdb"):
                shutil.copy2(os.path.join(bin_dir, f), os.path.join(b["dist_dir"], f))
                log_info(f"Copied: {f}")
                pdbs_copied = True

        if not pdbs_copied:
            log_info("No PDB files found")

    def export(self):
        b = self.init()
        os.makedirs(b["dist_dir"], exist_ok=True)
        log_step(f"Exporting Godot [{self.platform_type} | {self.build_type}]")
        log_info(f"Output → {b['output_path']}")
        godot_bin = Godot(None).get_bin()
        self.stage_lib()

        env = os.environ.copy()
        build_suffix = self.build_type.lower()
        extra_paths = [
            os.path.join(b["sandbox_dir"], "bin"),
            os.path.join(b["sandbox_dir"], f"bin/{build_suffix}"),
            os.path.join(b["project_dir"], "bin"),
        ]
        env["PATH"] = os.pathsep.join(extra_paths) + os.pathsep + env.get("PATH", "")

        result = subprocess.run([
            godot_bin, "--headless",
            "--path", b["project_dir"],
            b["export_mode"], b["preset"],
            b["output_path"]
        ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, env=env)

        for line in result.stdout.splitlines():
            if line.strip():
                log_info(line.strip())

        # Godot 4.5 crashes on shutdown on headless CI (signal 11 = 0xC0000005)
        # Affects both debug and release exports — export succeeds before the crash
        # This is a known Godot bug: https://github.com/godotengine/godot/issues/112955
        GODOT_HEADLESS_SHUTDOWN_CRASH = 3221225477
        if result.returncode != 0:
            if result.returncode == GODOT_HEADLESS_SHUTDOWN_CRASH:
                log_warn(f"Godot exited with known headless shutdown crash (exit {result.returncode}) — ignoring")
            else:
                log_error(f"Godot export failed with exit code {result.returncode}")
                sys.exit(result.returncode)

        if not os.path.exists(b["output_path"]):
            log_error(f"Export output missing: {b['output_path']}")
            sys.exit(1)

        log_ok("Done")
        self.copy_config()
        self.copy_lib()
        if self.build_type == "Debug":
            self.copy_pdb()


def main():
    parser = argparse.ArgumentParser(description="Build Vital")

    platform_group = parser.add_mutually_exclusive_group(required=True)
    platform_group.add_argument("--client", action="store_true")
    platform_group.add_argument("--server", action="store_true")
    platform_group.add_argument("--all", action="store_true", help="Build both client and server")

    build_group = parser.add_mutually_exclusive_group(required=True)
    build_group.add_argument("--debug", action="store_true")
    build_group.add_argument("--release", action="store_true")

    parser.add_argument("--skip-export", action="store_true", help="Skip Godot export")
    parser.add_argument("--rebuild-godot", action="store_true", help="Force rebuild of godot-cpp")
    parser.add_argument("--verbose", action="store_true", help="Stream compiler output live instead of spinner")

    args = parser.parse_args()
    build_type = "Release" if args.release else "Debug"
    script_dir = os.path.dirname(os.path.abspath(__file__))
    platforms = ["Client", "Server"] if args.all else ["Client"] if args.client else ["Server"]

    b = Build(script_dir, platforms[0], build_type, verbose=args.verbose)
    b.reload_vendors()
    b.build_godot_cpp(force=args.rebuild_godot)

    for platform_type in platforms:
        build = Build(script_dir, platform_type, build_type, verbose=args.verbose)
        build.build_sandbox()
        if not args.skip_export:
            build.export()

    log_step("Build complete")
    log_ok("Done")


if __name__ == "__main__":
    main()