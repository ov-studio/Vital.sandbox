import sys
sys.path.append("./Vital.sandbox")
from vital import *

GODOT_BIN = os.environ.get("GODOT_BIN", "godot")

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

def copy_dlls(platform_type, build_type, script_dir, dist_dir):
    print(f"\n==> Copying DLLs [{platform_type} | {build_type}]")
    client_dir = os.path.join(script_dir, "Vital.client")
    build_suffix = "release" if build_type == "Release" else "debug"

    # Find folders that directly contain a .gdextension file — skip only those exact folders
    gdextension_roots = set()
    for root, dirs, files in os.walk(client_dir):
        if any(f.endswith(".gdextension") for f in files):
            gdextension_roots.add(os.path.normpath(root))

    for root, dirs, files in os.walk(client_dir):
        norm_root = os.path.normpath(root)
        # Only skip the exact folder containing the .gdextension, not its subfolders
        if norm_root in gdextension_roots:
            continue

        dlls = []
        for f in files:
            if not f.lower().endswith(".dll"):
                continue
            name_lower = f.lower()
            has_release = "release" in name_lower
            has_debug = "debug" in name_lower
            if (has_release or has_debug) and build_suffix not in name_lower:
                continue
            dlls.append(f)

        if not dlls:
            continue

        for dll in dlls:
            src_file = os.path.join(root, dll)
            dst_file = os.path.join(dist_dir, dll)
            shutil.copy2(src_file, dst_file)
            print(f"  Copied: {dll}")

def export_godot(platform_type, build_type, script_dir):
    preset = f"Windows {platform_type}"
    output_name = "client.exe" if platform_type == "Client" else "server.exe"
    dist_dir = os.path.join(script_dir, "dist")
    os.makedirs(dist_dir, exist_ok=True)
    output_path = os.path.join(dist_dir, output_name)
    project_dir = os.path.join(script_dir, "Vital.client")
    export_mode = "--export-release" if build_type == "Release" else "--export-debug"

    print(f"\n==> Exporting Godot [{platform_type} | {build_type}] -> {output_path}")

    # Backup files that Godot may move/delete during export (gdextension dependencies)
    wry_dll = os.path.join(project_dir, "vital.wry", "bin", "x86_64-pc-windows-msvc", "godot_wry.dll")
    wry_backup = wry_dll + ".bak"
    if os.path.exists(wry_dll):
        shutil.copy2(wry_dll, wry_backup)

    result = subprocess.run([
        GODOT_BIN,
        "--headless",
        "--path", project_dir,
        export_mode, preset,
        output_path
    ])

    # Restore backed up files
    if os.path.exists(wry_backup):
        shutil.copy2(wry_backup, wry_dll)
        os.remove(wry_backup)

    if result.returncode != 0:
        print(f"[ERROR] Godot export failed for {platform_type}")
        sys.exit(result.returncode)

    # Copy all DLLs from Vital.client after export
    copy_dlls(platform_type, build_type, script_dir, dist_dir)

def main():
    parser = argparse.ArgumentParser(description="Build Vital")

    # Platform
    platform_group = parser.add_mutually_exclusive_group(required=True)
    platform_group.add_argument("--client", action="store_true")
    platform_group.add_argument("--server", action="store_true")
    platform_group.add_argument("--all",    action="store_true", help="Build both client and server")

    # Build
    build_group = parser.add_mutually_exclusive_group(required=True)
    build_group.add_argument("--debug",   action="store_true")
    build_group.add_argument("--release", action="store_true")

    # Flags
    parser.add_argument("--skip-extension", action="store_true", help="Skip building Vital.extension")
    parser.add_argument("--skip-export",    action="store_true", help="Skip Godot export")

    args = parser.parse_args()
    build_type = "Release" if args.release else "Debug"
    script_dir = os.path.dirname(os.path.abspath(__file__))

    platforms = []
    if args.all:
        platforms = ["Client", "Server"]
    elif args.client:
        platforms = ["Client"]
    else:
        platforms = ["Server"]

    for platform_type in platforms:
        if not args.skip_extension:
            build_extension(platform_type, build_type, script_dir)
        if not args.skip_export:
            export_godot(platform_type, build_type, script_dir)

    print("\n==> Build complete")

if __name__ == "__main__":
    main()