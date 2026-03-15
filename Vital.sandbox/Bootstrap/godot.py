from Bootstrap.utils import *
from Bootstrap.download import *

class Godot:
    def __init__(self, env):
        self.env = env

    def init(self):
        os_info = Fetch_OS()
        script_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
        version = self._get_version(script_dir)
        cache_dir = os.path.join(script_dir, ".godot", version) if version else None
        exe_name = self._get_exe_name(version, os_info["type"]) if version else None
        return {
            "script_dir":   script_dir,
            "version":      version,
            "cache_dir":    cache_dir,
            "exe_path":     os.path.join(cache_dir, exe_name) if cache_dir and exe_name else None,
            "templates_dir": self._get_templates_dir(version, os_info["type"]) if version else None,
        }

    def _get_version(self, script_dir):
        version_file = os.path.join(script_dir, "Vital.extension", "Vendor", "godot", "version.py")
        if os.path.exists(version_file):
            ns = {}
            with open(version_file) as f:
                exec(f.read(), ns)
            major = ns.get("major", 4)
            minor = ns.get("minor", 0)
            patch = ns.get("patch", 0)
            status = ns.get("status", "stable")
            for part in str(status).split("-"):
                if part in ("stable", "rc", "beta", "alpha", "dev"):
                    status = part
                    break
            return f"{major}.{minor}.{patch}-{status}" if patch else f"{major}.{minor}-{status}"
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

    def _ver_dash(self, version):
        return version.replace(".stable", "-stable").replace(".rc", "-rc").replace(".beta", "-beta")

    def _ver_short(self, version):
        return self._ver_dash(version).split("-")[0]

    def _release_base(self, version):
        return f"https://github.com/godotengine/godot/releases/download/{self._ver_dash(version)}"

    def _get_exe_name(self, version, os_type):
        vs = self._ver_short(version)
        if os_type == "Windows": return f"Godot_v{vs}-stable_win64.exe"
        if os_type == "Darwin":  return f"Godot_v{vs}-stable_macos.universal"
        return f"Godot_v{vs}-stable_linux.x86_64"

    def _get_binary_url(self, version, os_type):
        base = self._release_base(version)
        vs = self._ver_short(version)
        if os_type == "Windows": name = f"Godot_v{vs}-stable_win64.exe.zip"
        elif os_type == "Darwin": name = f"Godot_v{vs}-stable_macos.universal.zip"
        else:                     name = f"Godot_v{vs}-stable_linux.x86_64.zip"
        return f"{base}/{name}", name

    def _get_templates_url(self, version):
        vs = self._ver_short(version)
        name = f"Godot_v{vs}-stable_export_templates.tpz"
        return f"{self._release_base(version)}/{name}", name

    def _get_templates_dir(self, version, os_type):
        vd = self._ver_dash(version)
        vs = self._ver_short(version)
        status = vd.split("-", 1)[1] if "-" in vd else "stable"
        folder = f"{vs}.{status}"
        if os_type == "Windows":
            return os.path.join(os.environ["APPDATA"], "Godot", "export_templates", folder)
        elif os_type == "Darwin":
            return os.path.expanduser(f"~/Library/Application Support/Godot/export_templates/{folder}")
        else:
            return os.path.expanduser(f"~/.local/share/godot/export_templates/{folder}")

    def _extract_zip(self, zip_path, destination):
        print(f"  Extracting: {zip_path}")
        temp = destination + "_temp"
        if os.path.exists(temp):
            shutil.rmtree(temp)
        os.makedirs(temp, exist_ok=True)
        with __import__("zipfile").ZipFile(zip_path, "r") as z:
            z.extractall(temp)
        contents = os.listdir(temp)
        root = os.path.join(temp, contents[0]) if len(contents) == 1 and os.path.isdir(os.path.join(temp, contents[0])) else temp
        if os.path.exists(destination):
            shutil.rmtree(destination)
        os.makedirs(destination, exist_ok=True)
        for item in os.listdir(root):
            shutil.move(os.path.join(root, item), os.path.join(destination, item))
        shutil.rmtree(temp)

    def _extract_tpz(self, tpz_path, templates_dir):
        print(f"  Installing templates to: {templates_dir}")
        temp = tpz_path + "_temp"
        if os.path.exists(temp):
            shutil.rmtree(temp)
        os.makedirs(temp, exist_ok=True)
        with __import__("zipfile").ZipFile(tpz_path, "r") as z:
            z.extractall(temp)
        inner = os.path.join(temp, "templates")
        src = inner if os.path.isdir(inner) else temp
        if os.path.exists(templates_dir):
            shutil.rmtree(templates_dir)
        os.makedirs(templates_dir, exist_ok=True)
        for item in os.listdir(src):
            shutil.move(os.path.join(src, item), os.path.join(templates_dir, item))
        shutil.rmtree(temp)
        print("  Templates installed.")

    def install(self):
        os_info = Fetch_OS()
        godot = self.init()

        if not godot["version"]:
            Throw_Error("[ERROR] Could not detect Godot version from Vital.extension/Vendor/godot")

        print(f"\n==> Godot [{godot['version']}]")
        os.makedirs(godot["cache_dir"], exist_ok=True)

        # ── Binary ──
        if not os.path.exists(godot["exe_path"]):
            url, filename = self._get_binary_url(godot["version"], os_info["type"])
            dl_path = os.path.join(godot["cache_dir"], filename)
            print("  Downloading Godot binary...")
            Download(url, dl_path)
            self._extract_zip(dl_path, godot["cache_dir"])
            if os.path.exists(dl_path):
                os.remove(dl_path)
            if os_info["type"] != "Windows":
                os.chmod(godot["exe_path"], 0o755)
            print(f"  Binary ready: {godot['exe_path']}")
        else:
            print("  Binary cached.")

        # ── Export templates ──
        templates_dir = godot["templates_dir"]
        templates_ok = os.path.isdir(templates_dir) and any(
            f.endswith((".exe", ".so", ".dylib", "version"))
            for f in os.listdir(templates_dir)
        ) if os.path.isdir(templates_dir) else False

        if not templates_ok:
            url, filename = self._get_templates_url(godot["version"])
            tpz_path = os.path.join(godot["cache_dir"], filename)
            if not os.path.exists(tpz_path):
                print("  Downloading export templates...")
                Download(url, tpz_path)
            self._extract_tpz(tpz_path, templates_dir)
        else:
            print("  Templates cached.")

    def get_bin(self):
        self.install()
        return self.init()["exe_path"]

BaseEnvironment.Godot = property(lambda self: Godot(self))