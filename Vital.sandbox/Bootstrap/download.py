from Bootstrap.utils import *

def Download(url, destination):
    log_info(f"Downloading {os.path.basename(destination)} ...")
    try:
        with urllib.request.urlopen(url, timeout=60) as r, open(destination, "wb") as f:
            while True:
                chunk = r.read(8192)
                if not chunk:
                    break
                f.write(chunk)
    except (urllib.error.URLError, socket.timeout, TimeoutError, Exception) as e:
        if os.path.exists(destination):
            os.remove(destination)
        Throw_Error(f"Download failed: {e}")

def _Extract(path, destination, open_fn):
    log_info(f"Extracting {os.path.basename(path)} ...")
    temp_dir = destination + "_temp"
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)
    os.makedirs(temp_dir, exist_ok=True)
    with open_fn(path) as archive:
        archive.extractall(temp_dir)
    contents = os.listdir(temp_dir)
    root_dir = os.path.join(temp_dir, contents[0]) if len(contents) == 1 and os.path.isdir(os.path.join(temp_dir, contents[0])) else temp_dir
    if os.path.exists(destination):
        shutil.rmtree(destination)
    os.makedirs(destination, exist_ok=True)
    for item in os.listdir(root_dir):
        shutil.move(os.path.join(root_dir, item), os.path.join(destination, item))
    shutil.rmtree(temp_dir)

def Extract_Tar(path, destination):
    _Extract(path, destination, tarfile.open)

def Extract_Zip(path, destination):
    _Extract(path, destination, lambda p: zipfile.ZipFile(p, "r"))
