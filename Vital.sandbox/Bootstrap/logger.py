import sys, time, threading

def _supports_color():
    return hasattr(sys.stdout, "isatty") and sys.stdout.isatty()

class C:
    RESET  = "\033[0m"        if _supports_color() else ""
    BOLD   = "\033[1m"        if _supports_color() else ""
    HEADER = "\033[38;5;111m" if _supports_color() else ""
    GREEN  = "\033[38;5;114m" if _supports_color() else ""
    YELLOW = "\033[38;5;221m" if _supports_color() else ""
    RED    = "\033[38;5;203m" if _supports_color() else ""
    DIM    = "\033[2m"        if _supports_color() else ""

def log_step(msg):
    print(f"\n{C.BOLD}{C.HEADER}==>{C.RESET} {C.BOLD}{C.HEADER}{msg}{C.RESET}")

def log_info(msg):
    print(f"  {C.DIM}{msg}{C.RESET}")

def log_ok(msg):
    print(f"  {C.GREEN}{msg}{C.RESET}")

def log_warn(msg):
    print(f"  {C.YELLOW}[WARN]{C.RESET} {msg}")

def log_error(msg):
    print(f"  {C.RED}[ERROR]{C.RESET} {msg}")

def spinner(label, stop_event):
    frames = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]
    i = 0
    while not stop_event.is_set():
        print(f"\033[2K\r  {C.DIM}{frames[i % len(frames)]} {label}...{C.RESET}", end="", flush=True)
        i += 1
        time.sleep(0.08)
    print("\033[2K\r", end="", flush=True)