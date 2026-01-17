from Bootstrap.utils import *

def Download(url, destination):
    print("Downloading:", url)
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

def Extract_Tar(path, destination):
    print("Unpacking:", path)
    temp_dir = destination + "_temp"
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)
    os.makedirs(temp_dir, exist_ok=True)
    with tarfile.open(path) as tar:
        tar.extractall(temp_dir)
    contents = os.listdir(temp_dir)
    if len(contents) == 1 and os.path.isdir(os.path.join(temp_dir, contents[0])):
        root_dir = os.path.join(temp_dir, contents[0])
    else:
        root_dir = temp_dir
    if os.path.exists(destination):
        shutil.rmtree(destination)
    os.makedirs(destination, exist_ok=True)
    for item in os.listdir(root_dir):
        src = os.path.join(root_dir, item)
        dst = os.path.join(destination, item)
        print("Moving:", dst)
        shutil.move(src, dst)
    shutil.rmtree(temp_dir)
    print("Extraction complete.")