"""Scale a skinned glTF binary (.glb) uniformly so the character stands at a given height.

AI generators (Meshy, Tripo) normalise characters to about 1 m. UE imports glTF metres as metres, so the
detective would come in at 96 cm. Scaling the source keeps the skeleton, the bind pose and the mesh in step:
mesh positions, every node translation and the translation part of every inverse bind matrix.

Run: python Tools/art/glb_scale.py IN.glb OUT.glb --height 1.85   (standard library only)
"""
import argparse
import json
import struct
from pathlib import Path


def read_glb(path):
    b = Path(path).read_bytes()
    magic, _, _ = struct.unpack_from("<III", b, 0)
    if magic != 0x46546C67:
        raise SystemExit(f"{path} is not a glb")
    jlen, _ = struct.unpack_from("<II", b, 12)
    g = json.loads(b[20:20 + jlen])
    blen, _ = struct.unpack_from("<II", b, 20 + jlen)
    return g, bytearray(b[28 + jlen:28 + jlen + blen])


def write_glb(path, g, buf):
    js = json.dumps(g, separators=(",", ":")).encode()
    js += b" " * (-len(js) % 4)
    buf = bytes(buf) + b"\0" * (-len(buf) % 4)
    total = 12 + 8 + len(js) + 8 + len(buf)
    Path(path).write_bytes(struct.pack("<III", 0x46546C67, 2, total) + struct.pack("<II", len(js), 0x4E4F534A) + js
                           + struct.pack("<II", len(buf), 0x004E4942) + buf)


def view(g, acc):
    bv = g["bufferViews"][acc["bufferView"]]
    return bv.get("byteOffset", 0) + acc.get("byteOffset", 0), bv.get("byteStride", 0)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src")
    ap.add_argument("dst")
    ap.add_argument("--height", type=float, required=True, help="target height in metres (feet to top of head)")
    a = ap.parse_args()
    g, buf = read_glb(a.src)

    pos_ids = {p["attributes"]["POSITION"] for m in g["meshes"] for p in m["primitives"]}
    top = max(g["accessors"][i]["max"][1] for i in pos_ids)
    bottom = min(g["accessors"][i]["min"][1] for i in pos_ids)
    s = a.height / (top - bottom)

    shared = {}
    for i in pos_ids:
        bv = g["accessors"][i]["bufferView"]
        shared.setdefault(bv, []).append(i)
    for i in pos_ids:
        acc = g["accessors"][i]
        if acc["componentType"] != 5126 or acc["type"] != "VEC3" or "sparse" in acc:
            raise SystemExit(f"accessor {i}: only plain float VEC3 positions are supported")
        if len(shared[acc["bufferView"]]) > 1 and i != shared[acc["bufferView"]][0]:
            continue  # several accessors on one view: the first pass scaled them all
        off, stride = view(g, acc)
        stride = stride or 12
        bv_len = g["bufferViews"][acc["bufferView"]]["byteLength"]
        count = acc["count"] if len(shared[acc["bufferView"]]) == 1 else bv_len // stride
        for k in range(count):
            o = off + k * stride
            x, y, z = struct.unpack_from("<3f", buf, o)
            struct.pack_into("<3f", buf, o, x * s, y * s, z * s)
    for i in pos_ids:
        acc = g["accessors"][i]
        acc["min"] = [v * s for v in acc["min"]]
        acc["max"] = [v * s for v in acc["max"]]

    for n in g["nodes"]:
        if "translation" in n:
            n["translation"] = [v * s for v in n["translation"]]
        if "matrix" in n:
            n["matrix"] = [v * s if idx in (12, 13, 14) else v for idx, v in enumerate(n["matrix"])]

    for skin in g.get("skins", []):
        acc = g["accessors"][skin["inverseBindMatrices"]]
        off, stride = view(g, acc)
        stride = stride or 64
        for k in range(acc["count"]):
            o = off + k * stride + 48  # column-major: elements 12-14 are the translation
            x, y, z = struct.unpack_from("<3f", buf, o)
            struct.pack_into("<3f", buf, o, x * s, y * s, z * s)

    if g.get("animations"):
        raise SystemExit("animations present: their translation channels would need scaling too (not supported)")
    write_glb(a.dst, g, buf)
    print(f"scaled x{s:.4f}: {top - bottom:.3f} m -> {a.height:.3f} m, wrote {a.dst}")


if __name__ == "__main__":
    main()
