import sys
sys.path.append("./Vital.sandbox")
from vital import *

def main():
    parser = argparse.ArgumentParser(description="Build Vital")
    
    # Platform
    platform_group = parser.add_mutually_exclusive_group(required=True)
    platform_group.add_argument("--client", action="store_true")
    platform_group.add_argument("--server", action="store_true")
    
    # Build
    build_group = parser.add_mutually_exclusive_group(required=True)
    build_group.add_argument("--debug",   action="store_true")
    build_group.add_argument("--release", action="store_true")

    # Arguments
    args = parser.parse_args()
    platform_type = "Client" if args.client else "Server"
    build_type = "Debug" if args.debug else "Release"

    # Path to Vital.extension
    script_dir = os.path.dirname(os.path.abspath(__file__))
    extension_dir = os.path.join(script_dir, "Vital.extension")
    cmd = [
        "scons",
        "-C", extension_dir,
        f"platform_type={platform_type}",
        f"build_type={build_type}"
    ]
    result = subprocess.run(cmd)
    sys.exit(result.returncode)

if __name__ == "__main__":
    main()