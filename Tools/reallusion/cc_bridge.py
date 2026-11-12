"""Job bridge for Character Creator 5 (runs inside CC's Python, RLPy available).

Start it once per CC session: Script > Load Python > this file. (Optional autostart: the shim
written by install_bridge.py into <CC>/Bin64/OpenPlugin/, owner's choice.) It polls
Tools/reallusion/jobs/ every second:
  jobs/<name>.py   -> executed inside CC (globals: RLPy, log, REPO, JOBS)
  jobs/<name>.log  -> stdout, errors and timing of the run
  jobs/<name>.done -> the job file is renamed to this after the run
jobs/_heartbeat.txt is rewritten on every poll so an outside agent can see CC is alive.
"""
import io
import os
import time
import traceback
import contextlib

import RLPy

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
JOBS = os.path.join(HERE, "jobs")

_state = {}


def _poll():
    os.makedirs(JOBS, exist_ok=True)
    try:
        with open(os.path.join(JOBS, "_heartbeat.txt"), "w", encoding="utf-8") as f:
            f.write(time.strftime("%Y-%m-%d %H:%M:%S"))
    except OSError:
        pass
    for name in sorted(os.listdir(JOBS)):
        if not name.endswith(".py"):
            continue
        path = os.path.join(JOBS, name)
        base = path[:-3]
        buf = io.StringIO()
        running = base + ".running"

        def log(*a):
            print(*a, file=buf)
            with open(running, "a", encoding="utf-8") as f:  # survives a CC crash
                print(time.strftime("%H:%M:%S"), *a, file=f)

        t0 = time.time()
        try:
            src = open(path, encoding="utf-8").read()
            os.replace(path, base + ".done")
            g = {"__name__": "__job__", "RLPy": RLPy, "log": log, "REPO": REPO, "JOBS": JOBS}
            with contextlib.redirect_stdout(buf):
                exec(compile(src, path, "exec"), g)
            status = "OK"
        except Exception:
            buf.write(traceback.format_exc())
            status = "FAIL"
        buf.write(f"\n[{status}] {time.time() - t0:.1f}s\n")
        with open(base + ".log", "w", encoding="utf-8") as f:
            f.write(buf.getvalue())
        if os.path.exists(running):
            os.remove(running)
        break  # one job per tick keeps the UI responsive


class _Tick(RLPy.RPyTimerCallback):
    def __init__(self):
        RLPy.RPyTimerCallback.__init__(self)

    def Timeout(self):
        try:
            _poll()
        except Exception:
            pass


def start():
    if _state.get("timer"):
        return
    timer = RLPy.RPyTimer()
    cb = _Tick()
    timer.SetInterval(1000)
    timer.RegisterPyTimerCallback(cb)
    timer.Start()
    _state["timer"], _state["cb"] = timer, cb


start()
