import sys
sys.path.append("./Vital.sandbox")
from vital import *

class Build:
    def __init__(self, script_dir, platform_type, build_type):
        self.script_dir = script_dir
        self.platform_type = platform_type
        self.build_type = build_type
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
            print(f"\n==> godot-cpp [{self.build_type}] already built, skipping")
            return
        print(f"\n==> Building godot-cpp [{self.build_type}]")
        result = subprocess.run([
            "scons", "-C", godot_dir,
            f"target=template_{self.build_type.lower()}",
            "use_static_cpp=no",
        ])
        if result.returncode != 0:
            print("[ERROR] godot-cpp build failed")
            sys.exit(result.returncode)
        open(stamp, "w").close()

    def build_extension(self):
        b = self.init()
        print(f"\n==> Building Vital.extension [{self.platform_type} | {self.build_type}]")
        result = subprocess.run([
            "scons", "-C", b["extension_dir"],
            f"platform_type={self.platform_type}",
            f"build_type={self.build_type}",
            "build_library=no",
        ])
        if result.returncode != 0:
            print(f"[ERROR] Extension build failed for {self.platform_type}")
            sys.exit(result.returncode)

    def copy_libs(self, project_dir, dist_dir):
        print(f"\n==> Copying libraries [{self.platform_type} | {self.build_type} | {self.os_info['type']}]")
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
                print(f"  Copied: {lib}")

    def export(self):
        b = self.init()
        os.makedirs(b["dist_dir"], exist_ok=True)
        print(f"\n==> Exporting Godot [{self.platform_type} | {self.build_type}] -> {b['output_path']}")
        godot_bin = Godot(None).get_bin()
        result = subprocess.run([
            godot_bin, "--headless",
            "--path", b["project_dir"],
            b["export_mode"], b["preset"],
            b["output_path"]
        ])
        if result.returncode != 0:
            print(f"[ERROR] Godot export failed for {self.platform_type}")
            sys.exit(result.returncode)
        self.copy_libs(b["project_dir"], b["dist_dir"])


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

    args = parser.parse_args()
    build_type = "Release" if args.release else "Debug"
    script_dir = os.path.dirname(os.path.abspath(__file__))
    platforms = ["Client", "Server"] if args.all else ["Client"] if args.client else ["Server"]

    if not args.skip_extension:
        Build(script_dir, platforms[0], build_type).build_godot_cpp(force=args.rebuild_godot)

    for platform_type in platforms:
        build = Build(script_dir, platform_type, build_type)
        if not args.skip_extension:
            build.build_extension()
        if not args.skip_export:
            build.export()

    print("\n==> Build complete")

if __name__ == "__main__":
    main()