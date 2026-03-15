import sys, os, shutil, subprocess, zipfile, urllib.request, socket

# ── Download helper ────────────────────────────────────────────────────────
def _download(url, destination):
    print(f"  Downloading: {url}")
    try:
        with urllib.request.urlopen(url, timeout=120) as r, open(destination, "wb") as f:
            total = int(r.headers.get("Content-Length", 0))
            downloaded = 0
            while True:
                chunk = r.read(65536)
                if not chunk:
                    break
                f.write(chunk)
                downloaded += len(chunk)
                if total:
                    pct = downloaded * 100 // total
                    print(f"\r  {pct}% ({downloaded // 1024 // 1024}MB / {total // 1024 // 1024}MB)", end="", flush=True)
        print()
    except Exception as e:
        if os.path.exists(destination):
            os.remove(destination)
        print(f"[ERROR] Download failed: {e}")
        sys.exit(1)

def _extract_zip_flat(zip_path, destination):
    """Extract zip, stripping any single root folder."""
    print(f"  Extracting: {zip_path}")
    temp = destination + "_temp"
    if os.path.exists(temp):
        shutil.rmtree(temp)
    os.makedirs(temp, exist_ok=True)
    with zipfile.ZipFile(zip_path, "r") as z:
        z.extractall(temp)
    contents = os.listdir(temp)
    root = os.path.join(temp, contents[0]) if len(contents) == 1 and os.path.isdir(os.path.join(temp, contents[0])) else temp
    if os.path.exists(destination):
        shutil.rmtree(destination)
    os.makedirs(destination, exist_ok=True)
    for item in os.listdir(root):
        shutil.move(os.path.join(root, item), os.path.join(destination, item))
    shutil.rmtree(temp)

def _extract_tpz_to(tpz_path, templates_dir):
    """Extract .tpz (zip), move contents of inner 'templates/' folder to templates_dir."""
    print(f"  Installing templates to: {templates_dir}")
    temp = tpz_path + "_temp"
    if os.path.exists(temp):
        shutil.rmtree(temp)
    os.makedirs(temp, exist_ok=True)
    with zipfile.ZipFile(tpz_path, "r") as z:
        z.extractall(temp)
    # Inside the .tpz there's always a 'templates/' subfolder
    inner = os.path.join(temp, "templates")
    src = inner if os.path.isdir(inner) else temp
    if os.path.exists(templates_dir):
        shutil.rmtree(templates_dir)
    os.makedirs(templates_dir, exist_ok=True)
    for item in os.listdir(src):
        shutil.move(os.path.join(src, item), os.path.join(templates_dir, item))
    shutil.rmtree(temp)
    print("  Templates installed.")


# ── Host platform detection ────────────────────────────────────────────────
def get_host_platform():
    if sys.platform.startswith("win"):     return "windows"
    if sys.platform.startswith("darwin"):  return "macos"
    return "linux"


# ── Godot version from submodule ───────────────────────────────────────────
def get_godot_version(script_dir):
    version_file = os.path.join(script_dir, "Vital.extension", "Vendor", "godot", "version.py")
    if os.path.exists(version_file):
        ns = {}
        with open(version_file) as f:
            exec(f.read(), ns)
        major  = ns.get("major", 4)
        minor  = ns.get("minor", 0)
        patch  = ns.get("patch", 0)
        status = ns.get("status", "stable")
        # Strip any "godot-" prefix or version numbers from status
        # e.g. "godot-4.4.1-stable" -> "stable"
        for part in str(status).split("-"):
            if part in ("stable", "rc", "beta", "alpha", "dev"):
                status = part
                break
        return f"{major}.{minor}.{patch}-{status}" if patch else f"{major}.{minor}-{status}"

    # Fallback: git describe on the submodule
    try:
        result = subprocess.run(
            ["git", "-C", os.path.join(script_dir, "Vital.extension", "Vendor", "godot"),
             "describe", "--tags", "--exact-match"],
            capture_output=True, text=True
        )
        tag = result.stdout.strip().lstrip("v")
        if tag.startswith("godot-"):
            tag = tag[len("godot-"):]
        if tag:
            return tag
    except Exception:
        pass
    return None


# ── URL builders ───────────────────────────────────────────────────────────
def _ver_dash(version):
    """4.4.1.stable or 4.4.1-stable -> 4.4.1-stable"""
    return version.replace(".stable", "-stable").replace(".rc", "-rc").replace(".beta", "-beta")

def _ver_short(version):
    """4.4.1.stable or 4.4.1-stable -> 4.4.1"""
    return _ver_dash(version).split("-")[0]

def _release_base(version):
    vd = _ver_dash(version)
    return f"https://github.com/godotengine/godot/releases/download/{vd}"

def get_binary_url(version, host_platform):
    base = _release_base(version)
    vs   = _ver_short(version)
    if host_platform == "windows":
        name = f"Godot_v{vs}-stable_win64.exe.zip"
        return f"{base}/{name}", name, True
    elif host_platform == "macos":
        name = f"Godot_v{vs}-stable_macos.universal.zip"
        return f"{base}/{name}", name, True
    else:
        name = f"Godot_v{vs}-stable_linux.x86_64.zip"
        return f"{base}/{name}", name, True

def get_binary_exe_name(version, host_platform):
    vs = _ver_short(version)
    if host_platform == "windows": return f"Godot_v{vs}-stable_win64.exe"
    if host_platform == "macos":   return f"Godot_v{vs}-stable_macos.universal"
    return f"Godot_v{vs}-stable_linux.x86_64"

def get_templates_url(version):
    base = _release_base(version)
    vs   = _ver_short(version)
    name = f"Godot_v{vs}-stable_export_templates.tpz"
    return f"{base}/{name}", name

def get_templates_dir(version):
    vd     = _ver_dash(version)
    vs     = _ver_short(version)
    status = vd.split("-", 1)[1] if "-" in vd else "stable"
    folder = f"{vs}.{status}"   # e.g. "4.4.1.stable"
    if sys.platform.startswith("win"):
        return os.path.join(os.environ["APPDATA"], "Godot", "export_templates", folder)
    elif sys.platform.startswith("darwin"):
        return os.path.expanduser(f"~/Library/Application Support/Godot/export_templates/{folder}")
    else:
        return os.path.expanduser(f"~/.local/share/godot/export_templates/{folder}")


# ── Main setup entry point ─────────────────────────────────────────────────
def setup_godot(script_dir, host_platform):
    version = get_godot_version(script_dir)
    print(f"  Detected version string: {repr(version)}")
    if not version:
        godot_bin = os.environ.get("GODOT_BIN", "godot")
        print(f"  [WARN] Could not detect Godot version, falling back to: {godot_bin}")
        return godot_bin

    print(f"\n==> Godot [{version}]")
    cache_dir = os.path.join(script_dir, ".godot", version)
    os.makedirs(cache_dir, exist_ok=True)

    # ── Binary ──
    exe_name = get_binary_exe_name(version, host_platform)
    exe_path = os.path.join(cache_dir, exe_name)

    if not os.path.exists(exe_path):
        url, filename, is_zip = get_binary_url(version, host_platform)
        dl_path = os.path.join(cache_dir, filename)
        print(f"  Downloading Godot binary...")
        _download(url, dl_path)
        if is_zip:
            _extract_zip_flat(dl_path, cache_dir)
        if os.path.exists(dl_path):
                os.remove(dl_path)
        if host_platform != "windows":
            os.chmod(exe_path, 0o755)
        print(f"  Binary ready: {exe_path}")
    else:
        print(f"  Binary cached.")

    # ── Export templates ──
    templates_dir = get_templates_dir(version)
    version_file  = os.path.join(templates_dir, "version")

    if not os.path.exists(version_file):
        url, filename = get_templates_url(version)
        tpz_path = os.path.join(cache_dir, filename)
        if not os.path.exists(tpz_path):
            print(f"  Downloading export templates...")
            _download(url, tpz_path)
        _extract_tpz_to(tpz_path, templates_dir)
    else:
        print(f"  Templates cached.")

    return exe_path


# Alias for build.py compatibility
def ensure_templates(script_dir):
    host_platform = get_host_platform()
    setup_godot(script_dir, host_platform)