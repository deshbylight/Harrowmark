"""Install the CC job bridge: writes <CC>/Bin64/OpenPlugin/HarrowMarkBridge/main.py, a shim that runs
Tools/reallusion/cc_bridge.py from this repo at CC startup. Remove that folder to uninstall.

Usage: python Tools/reallusion/install_bridge.py ["D:/Reallusion/iClone 8/Character Creator 5"]
"""
import sys
from pathlib import Path

DEFAULT_CC = Path(r"D:\Reallusion\iClone 8\Character Creator 5")
BRIDGE = Path(__file__).resolve().with_name("cc_bridge.py")

SHIM = '''# HarrowMark job bridge shim (installed by {src}).
import runpy
_g = runpy.run_path(r"{bridge}", run_name="harrowmark_bridge")


def initialize_plugin():
    pass


def run_script():
    pass
'''


def main():
    cc = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_CC
    plug = cc / "Bin64" / "OpenPlugin" / "HarrowMarkBridge"
    plug.mkdir(parents=True, exist_ok=True)
    (plug / "main.py").write_text(SHIM.format(src=Path(__file__).name, bridge=BRIDGE), encoding="utf-8")
    jobs = BRIDGE.parent / "jobs"
    jobs.mkdir(exist_ok=True)
    (jobs / ".gitignore").write_text("*\n!.gitignore\n", encoding="utf-8")
    print("installed", plug / "main.py")


if __name__ == "__main__":
    main()
