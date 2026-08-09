from Bootstrap.utils import *
from Bootstrap.download import *

VITAL_WRY_REPO    = "ov-studio/Vital.wry"
VITAL_WRY_API     = f"https://api.github.com/repos/{VITAL_WRY_REPO}/releases/latest"
VITAL_WRY_VERSION = "version"

class Wry:
    def __init__(self, script_dir, client_dir):
        self.script_dir   = script_dir
        self.client_dir   = client_dir
        self.install_dir  = os.path.join(client_dir, "vital.wry")
        self.version_path = os.path.join(self.install_dir, VITAL_WRY_VERSION)

    def _fetch_release(self):
        import json
        try:
            req = urllib.request.Request(
                VITAL_WRY_API,
                headers={"Accept": "application/vnd.github+json", "User-Agent": "vital-bootstrap"}
            )
            with urllib.request.urlopen(req, timeout=30) as r:
                return json.loads(r.read().decode())
        except Exception as e:
            Throw_Error(f"Failed to fetch Vital.wry release info: {e}")

    def _installed_version(self):
        if os.path.exists(self.version_path):
            with open(self.version_path) as f:
                return f.read().strip()
        return None

    def _write_version(self, version):
        with open(self.version_path, "w") as f:
            f.write(version)

    def build(self):
        log_step("Bootstrapping Vital.wry")

        release = self._fetch_release()
        version = release.get("tag_name", "unknown")
        current = self._installed_version()

        if current == version and os.path.isdir(self.install_dir):
            log_info(f"Already up to date ({version})")
            return

        assets = release.get("assets", [])
        asset  = next((a for a in assets if a["name"].endswith(".zip") and "Vital.wry" in a["name"]), None)
        if not asset:
            Throw_Error(f"No zip asset found in Vital.wry release {version}")

        url      = asset["browser_download_url"]
        zip_name = asset["name"]
        zip_path = os.path.join(self.script_dir, zip_name)

        log_info(f"Version: {version}")
        log_info(f"Asset:   {zip_name}")

        Download(url, zip_path)

        log_info("Installing to Vital.client/vital.wry/ ...")
        if os.path.exists(self.install_dir):
            shutil.rmtree(self.install_dir)
        os.makedirs(self.install_dir, exist_ok=True)

        with zipfile.ZipFile(zip_path, "r") as z:
            z.extractall(self.install_dir)

        os.remove(zip_path)
        self._write_version(version)
        log_ok(f"Vital.wry {version} installed")