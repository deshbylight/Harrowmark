"""Queue a job for cc_bridge.py running inside Character Creator and wait for its log.

  python Tools/reallusion/queue_job.py probe
  python Tools/reallusion/queue_job.py headshot CH03 --photo <repo-relative png> --body male
  python Tools/reallusion/queue_job.py export CH03
  python Tools/reallusion/queue_job.py file path/to/script.py   (any RLPy script)
"""
import argparse
import subprocess
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
JOBS = HERE / "jobs"

PROBE = '''import sys
log("python", sys.version)
av = RLPy.RScene.GetAvatars(RLPy.EAvatarType_All)
log("avatars", [a.GetName() for a in av])
if av:
    log("fbx licence", RLPy.RFileIO.CheckExportFbxHasLicense(av[0]))
'''

HEADSHOT = '''import runpy
m = runpy.run_path(r"{mod}")
m["run"](RLPy, log, REPO, {cid!r}, photo={photo!r}, body={body!r}, export_only={exp!r})
'''


def _cc_alive():
    out = subprocess.run(["tasklist", "/FI", "IMAGENAME eq CharacterCreator.exe", "/NH"],
                         capture_output=True, text=True).stdout
    return "CharacterCreator.exe" in out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("op", choices=["probe", "headshot", "export", "file"])
    ap.add_argument("arg", nargs="?")
    ap.add_argument("--photo")
    ap.add_argument("--body", default="male")
    ap.add_argument("--timeout", type=int, default=900)
    a = ap.parse_args()
    JOBS.mkdir(exist_ok=True)
    hb = JOBS / "_heartbeat.txt"
    if not hb.exists() or time.time() - hb.stat().st_mtime > 5:
        print("warning: no heartbeat; is CC running with the bridge loaded (Script > Load Python > cc_bridge.py)?")
    if a.op == "probe":
        src, name = PROBE, "probe"
    elif a.op == "file":
        src, name = Path(a.arg).read_text(encoding="utf-8"), Path(a.arg).stem
    else:
        src = HEADSHOT.format(mod=HERE / "cc_headshot.py", cid=a.arg, photo=a.photo, body=a.body,
                              exp=a.op == "export")
        name = f"{a.op}_{a.arg}"
    name += time.strftime("_%H%M%S")
    log = JOBS / (name + ".log")
    tmp = JOBS / (name + ".tmp")
    tmp.write_text(src, encoding="utf-8")
    tmp.rename(JOBS / (name + ".py"))
    print("queued", name)
    t0 = time.time()
    while not log.exists():
        if time.time() - t0 > a.timeout:
            run = JOBS / (name + ".running")
            if run.exists():
                print(run.read_text(encoding="utf-8"))
            raise SystemExit("timeout waiting for " + str(log))
        if int(time.time() - t0) % 15 == 0 and not _cc_alive():
            run = JOBS / (name + ".running")
            print(run.read_text(encoding="utf-8") if run.exists() else "(no progress lines)")
            raise SystemExit("CharacterCreator.exe is gone: CC crashed during the job")
        time.sleep(1)
    time.sleep(0.3)
    print(log.read_text(encoding="utf-8"))


if __name__ == "__main__":
    main()
