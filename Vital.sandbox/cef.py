import os, sys, subprocess, hashlib, glob, urllib.request
from utils import *
from SCons.Environment import Base as BaseEnvironment
from SCons.Script import Copy, Action

os_info = Fetch_OS()
cef_version = "143.0.13+g30cb3bd+chromium-143.0.7499.170" #https://cef-builds.spotifycdn.com/index.html

def Init_CEF(self):
    root = os.path.join(os.path.abspath(os.path.dirname(__file__)), ".cef")
    archi = None
    version = cef_version
    if os_info["type"] == "Windows":
        if os_info["archi"] == "x86_64" or os_info["archi"] == "AMD64":
            archi = "windows64"
        else:
            archi = "windowsarm64"
    elif os_info["type"] == "Darwin":
        if os_info["archi"] == "x86_64":
            archi = "macosx64"
        else:
            archi = "macosarm64"
    elif os_info["type"] == "Linux":
        if os_info["archi"] == "x86_64":
            archi = "linux64"
        else:
            archi = "linuxarm"
    identifier = "cef_binary_" + version.replace("+", "%2B") + "_" + archi + ".tar.bz2"
    return {
        "root" : root,
        "archi" : archi,
        "version" : version,
        "identifier" : identifier,
        "url" : "https://cef-builds.spotifycdn.com/" + identifier
    }
BaseEnvironment.Init_CEF = Init_CEF

def Install_CEF(self):
    cef_info = self.Init_CEF()
    os.makedirs(cef_info["root"], exist_ok=True)
    if not os.path.exists(cef_info["root"] + "/" + cef_info["identifier"]):
        Download_Remote(cef_info["url"], cef_info["root"] + "/" + cef_info["identifier"])
        Extract_Tar(cef_info["root"] + "/" + cef_info["identifier"], cef_info["root"])
BaseEnvironment.Install_CEF = Install_CEF