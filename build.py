import sys
sys.path.append("./Vital.sandbox")
from vital import *

GODOT_BIN = os.environ.get("GODOT_BIN", "godot")

PLATFORM_INFO = {
    "windows": {
        "lib_exts":   [".dll"],
        "preset":     "Windows {platform_type}",
        "output_ext": ".exe",
    },
    "linux": {
        "lib_exts":   [".so"],
        "preset":     "Linux/X11 {platform_type}",
        "output_ext": "",
    },
    "macos": {
        "lib_exts":   [".dylib", ".framework"],
        "preset":     "macOS {platform_type}",
        "output_ext": ".app",
    },
}

def get_host_platform():
    if sys.platform.startswith("win"):    return "windows"
    if sys.platform.startswith("darwin"): return "macos"
    return "linux"

def build_extension(platform_type, build_type, script_dir):
    print(f"\n==> Building Vital.extension [{platform_type} | {build_type}]")
    extension_dir = os.path.join(script_dir, "Vital.extension")
    result = subprocess.run([
        "scons",
        "-C", extension_dir,
        f"platform_type={platform_type}",
        f"build_type={build_type}"
    ])
    if result.returncode != 0:
        print(f"[ERROR] Extension build failed for {platform_type}")
        sys.exit(result.returncode)

def copy_libs(platform_type, build_type, project_dir, dist_dir, host_platform):
    print(f"\n==> Copying libraries [{platform_type} | {build_type} | {host_platform}]")
    build_suffix = "release" if build_type == "Release" else "debug"
    lib_exts = PLATFORM_INFO[host_platform]["lib_exts"]

    # Skip folders that directly contain a .gdextension file
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
            has_debug   = "debug"   in name_lower
            if (has_release or has_debug) and build_suffix not in name_lower:
                continue
            libs.append(f)

        if not libs:
            continue

        for lib in libs:
            src_file = os.path.join(root, lib)
            dst_file = os.path.join(dist_dir, lib)
            shutil.copy2(src_file, dst_file)
            print(f"  Copied: {lib}")

def export_godot(platform_type, build_type, script_dir):
    host_platform = get_host_platform()
    info          = PLATFORM_INFO[host_platform]
    preset        = info["preset"].format(platform_type=platform_type)
    output_name   = ("client" if platform_type == "Client" else "server") + info["output_ext"]
    dist_dir      = os.path.join(script_dir, ".dist", build_type.lower(), platform_type.lower())
    os.makedirs(dist_dir, exist_ok=True)
    output_path   = os.path.join(dist_dir, output_name)
    project_dir   = os.path.join(script_dir, f"Vital.{platform_type.lower()}")
    export_mode   = "--export-release" if build_type == "Release" else "--export-debug"

    print(f"\n==> Exporting Godot [{platform_type} | {build_type}] -> {output_path}")

    result = subprocess.run([
        GODOT_BIN,
        "--headless",
        "--path", project_dir,
        export_mode, preset,
        output_path
    ])

    if result.returncode != 0:
        print(f"[ERROR] Godot export failed for {platform_type}")
        sys.exit(result.returncode)

    copy_libs(platform_type, build_type, project_dir, dist_dir, host_platform)

def main():
    parser = argparse.ArgumentParser(description="Build Vital")

    platform_group = parser.add_mutually_exclusive_group(required=True)
    platform_group.add_argument("--client", action="store_true")
    platform_group.add_argument("--server", action="store_true")
    platform_group.add_argument("--all",    action="store_true", help="Build both client and server")

    build_group = parser.add_mutually_exclusive_group(required=True)
    build_group.add_argument("--debug",   action="store_true")
    build_group.add_argument("--release", action="store_true")

    parser.add_argument("--skip-extension", action="store_true", help="Skip building Vital.extension")
    parser.add_argument("--skip-export",    action="store_true", help="Skip Godot export")

    args = parser.parse_args()
    build_type = "Release" if args.release else "Debug"
    script_dir = os.path.dirname(os.path.abspath(__file__))

    platforms = []
    if args.all:      platforms = ["Client", "Server"]
    elif args.client: platforms = ["Client"]
    else:             platforms = ["Server"]

    for platform_type in platforms:
        if not args.skip_extension:
            build_extension(platform_type, build_type, script_dir)
        if not args.skip_export:
            export_godot(platform_type, build_type, script_dir)

    print("\n==> Build complete")

if __name__ == "__main__":
    main()