import os, sys, time, threading

if os.name == "nt":
    os.system("")

if sys.stdout.encoding and sys.stdout.encoding.lower() != "utf-8":
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except (AttributeError, OSError):
        pass

_LAST_STEP = ""
_EVENTS = 0
_ORIG_CP = 0

def _defaults():
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

FONT, FONT_SIZE, FONT_WEIGHT = _defaults()

def _restore_font():
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
        if info.FaceName == FONT:
            return None
        old_face = info.FaceName
        info.FaceName = FONT
        info.FontFamily = 54
        info.FontWeight = FONT_WEIGHT
        info.dwFontSize = COORD(0, FONT_SIZE)
        k.SetCurrentConsoleFontEx(handle, False, ctypes.byref(info))
        return old_face
    except Exception:
        return None

def _restore_code_page():
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

def _log_path():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "font-debug.log"))

def _log_line(text):
    try:
        import datetime
        with open(_log_path(), "a", encoding="utf-8") as f:
            f.write(f"{datetime.datetime.now():%H:%M:%S} {text}\n")
    except Exception:
        pass

def _log_event(what):
    global _EVENTS
    _EVENTS += 1
    if _EVENTS > 10:
        return
    try:
        import ctypes, csv, io, datetime, subprocess
        k = ctypes.windll.kernel32
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
        with open(_log_path(), "a", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
    except Exception:
        pass

def check():
    old_cp = _restore_code_page()
    if old_cp:
        _log_event(f"code page changed from {old_cp} back to {_ORIG_CP}")
    old_face = _restore_font()
    if old_face:
        _log_event(f"font changed from '{old_face}'")

def set_step(msg):
    global _LAST_STEP
    _LAST_STEP = msg
    check()

def _watchdog():
    while True:
        time.sleep(0.25)
        check()

if os.name == "nt" and hasattr(sys.stdout, "isatty") and sys.stdout.isatty():
    try:
        import ctypes
        _ORIG_CP = ctypes.windll.kernel32.GetConsoleOutputCP()
        _log_line(f"startup: python {sys.version.split()[0]} utf8_mode={sys.flags.utf8_mode} outputCP={_ORIG_CP} font='{FONT}' {FONT_SIZE}")
    except Exception:
        _ORIG_CP = 0

if os.name == "nt":
    check()
    threading.Thread(target=_watchdog, daemon=True).start()
