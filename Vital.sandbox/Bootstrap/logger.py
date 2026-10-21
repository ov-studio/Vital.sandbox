import os, sys, time, threading

if os.name == "nt":
    os.system("")  # enable ANSI escape processing in the Windows console

if sys.stdout.encoding and sys.stdout.encoding.lower() != "utf-8":
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except (AttributeError, OSError):
        pass

def _supports_color():
    return hasattr(sys.stdout, "isatty") and sys.stdout.isatty()

def _supports_unicode():
    try:
        "\u280b".encode(sys.stdout.encoding or "ascii")
        return True
    except (UnicodeEncodeError, LookupError):
        return False

def _console_defaults():
    # Use the font from the user's Windows console defaults, falling back to Consolas 16
    face, size, weight = "Consolas", 16, 400
    if os.name == "nt":
        try:
            import winreg
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, "Console") as key:
                try:
                    value = winreg.QueryValueEx(key, "FaceName")[0]
                    if value and value != "Terminal":
                        face = value
                except OSError:
                    pass
                try:
                    size = (winreg.QueryValueEx(key, "FontSize")[0] >> 16) or size
                except OSError:
                    pass
                try:
                    weight = winreg.QueryValueEx(key, "FontWeight")[0] or weight
                except OSError:
                    pass
        except OSError:
            pass
    return face, size, weight

CONSOLE_FONT, CONSOLE_FONT_SIZE, CONSOLE_FONT_WEIGHT = _console_defaults()

def _restore_console_font():
    # Classic Windows console can fall back to Raster Fonts mid-run; put the TrueType font back
    if os.name != "nt":
        return None
    try:
        import ctypes
        from ctypes import wintypes

        class COORD(ctypes.Structure):
            _fields_ = [("X", wintypes.SHORT), ("Y", wintypes.SHORT)]

        class CONSOLE_FONT_INFOEX(ctypes.Structure):
            _fields_ = [("cbSize", wintypes.ULONG), ("nFont", wintypes.DWORD), ("dwFontSize", COORD),
                        ("FontFamily", wintypes.UINT), ("FontWeight", wintypes.UINT), ("FaceName", wintypes.WCHAR * 32)]

        k = ctypes.windll.kernel32
        k.GetStdHandle.restype = wintypes.HANDLE
        fn = (wintypes.HANDLE, wintypes.BOOL, ctypes.POINTER(CONSOLE_FONT_INFOEX))
        k.GetCurrentConsoleFontEx.argtypes = fn
        k.SetCurrentConsoleFontEx.argtypes = fn
        handle = k.GetStdHandle(-11)
        info = CONSOLE_FONT_INFOEX()
        info.cbSize = ctypes.sizeof(CONSOLE_FONT_INFOEX)
        if not k.GetCurrentConsoleFontEx(handle, False, ctypes.byref(info)):
            return None
        if info.FaceName == CONSOLE_FONT:
            return None
        old_face = info.FaceName
        info.FaceName = CONSOLE_FONT
        info.FontFamily = 54
        info.FontWeight = CONSOLE_FONT_WEIGHT
        info.dwFontSize = COORD(0, CONSOLE_FONT_SIZE)
        k.SetCurrentConsoleFontEx(handle, False, ctypes.byref(info))
        return old_face
    except Exception:
        return None

_LAST_STEP = ""
_FONT_EVENTS = 0

def _log_line(text):
    try:
        import datetime
        path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "font-debug.log"))
        with open(path, "a", encoding="utf-8") as f:
            f.write(f"{datetime.datetime.now():%H:%M:%S} {text}\n")
    except Exception:
        pass

def _debug_font_event(what):
    # Diagnostic: record what was running when the console changed (font-debug.log in repo root)
    global _FONT_EVENTS
    _FONT_EVENTS += 1
    if _FONT_EVENTS > 10:
        return
    try:
        import ctypes, csv, io, datetime, subprocess
        k = ctypes.windll.kernel32
        path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "font-debug.log"))
        lines = [f"{datetime.datetime.now():%H:%M:%S} {what} | outputCP={k.GetConsoleOutputCP()} inputCP={k.GetConsoleCP()} | last step: {_LAST_STEP}"]
        ps = "Get-CimInstance Win32_Process | Select-Object ProcessId,ParentProcessId,Name,CommandLine | ConvertTo-Csv -NoTypeInformation"
        out = subprocess.run(["powershell", "-NoProfile", "-Command", ps], capture_output=True, text=True,
                             timeout=20, creationflags=0x08000000).stdout
        rows = {int(r["ProcessId"]): r for r in csv.DictReader(io.StringIO(out)) if r.get("ProcessId")}
        mine = {os.getpid()}
        grew = True
        while grew:
            grew = False
            for pid, r in rows.items():
                if pid not in mine and int(r["ParentProcessId"] or 0) in mine:
                    mine.add(pid)
                    grew = True
        for pid in sorted(mine - {os.getpid()}):
            r = rows[pid]
            lines.append(f"    pid {pid} (parent {r['ParentProcessId']}) {r['Name']}: {(r['CommandLine'] or '')[:220]}")
        with open(path, "a", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
    except Exception:
        pass

_ORIG_CP = 0
if os.name == "nt" and hasattr(sys.stdout, "isatty") and sys.stdout.isatty():
    try:
        import ctypes
        _ORIG_CP = ctypes.windll.kernel32.GetConsoleOutputCP()
        _log_line(f"startup: python {sys.version.split()[0]} utf8_mode={sys.flags.utf8_mode} outputCP={_ORIG_CP} font='{CONSOLE_FONT}' {CONSOLE_FONT_SIZE}")
    except Exception:
        _ORIG_CP = 0

def _restore_code_page():
    # Some tools (e.g. Conan's generated scripts run chcp 65001) change the shared console code page and leave it
    if not _ORIG_CP:
        return None
    try:
        import ctypes
        k = ctypes.windll.kernel32
        cp = k.GetConsoleOutputCP()
        if cp and cp != _ORIG_CP:
            k.SetConsoleOutputCP(_ORIG_CP)
            k.SetConsoleCP(_ORIG_CP)
            return cp
    except Exception:
        pass
    return None

def _check_font():
    old_cp = _restore_code_page()
    if old_cp:
        _debug_font_event(f"code page changed from {old_cp} back to {_ORIG_CP}")
    old_face = _restore_console_font()
    if old_face:
        _debug_font_event(f"font changed from '{old_face}'")

_check_font()

def _font_watchdog():
    while True:
        time.sleep(0.25)
        _check_font()

if os.name == "nt":
    threading.Thread(target=_font_watchdog, daemon=True).start()

class C:
    RESET  = "\033[0m"        if _supports_color() else ""
    BOLD   = "\033[1m"        if _supports_color() else ""
    HEADER = "\033[38;5;111m" if _supports_color() else ""
    GREEN  = "\033[38;5;114m" if _supports_color() else ""
    YELLOW = "\033[38;5;221m" if _supports_color() else ""
    RED    = "\033[38;5;203m" if _supports_color() else ""
    DIM    = "\033[2m"        if _supports_color() else ""

def log_step(msg):
    global _LAST_STEP
    _LAST_STEP = msg
    _check_font()
    print(f"\n{C.BOLD}{C.HEADER}==> {msg}{C.RESET}", flush=True)

def log_info(msg):
    print(f"    {C.DIM}{msg}{C.RESET}", flush=True)

def log_ok(msg):
    print(f"    {C.GREEN}{msg}{C.RESET}", flush=True)

def log_warn(msg):
    print(f"    {C.YELLOW}[WARN]{C.RESET} {msg}", flush=True)

def log_error(msg):
    print(f"    {C.RED}[ERROR]{C.RESET} {msg}", flush=True)

def spinner(label, stop_event):
    frames = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"] if _supports_unicode() else ["-", "\\", "|", "/"]
    i = 0
    while not stop_event.is_set():
        try:
            print(f"\033[2K\r  {C.DIM}{frames[i % len(frames)]} {label}...{C.RESET}", end="", flush=True)
        except UnicodeEncodeError:
            print(f"\r  {frames[i % len(frames)]} {label}...", end="", flush=True)
        i += 1
        time.sleep(0.08)
    print("\033[2K\r", end="", flush=True)