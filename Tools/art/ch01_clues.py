"""Paint the D06 clothing clues onto the detective CH01 base colour texture.

The clues (CASE01 D06, ASSETS CH01) are part of the model in every view and never mentioned:
  - a dry salt tide line on both trouser legs, shin-high, salt bloom below it and on the shoes;
  - the right glove torn across the palm, the cut skin showing, a pale ash splinter in the tear;
  - pale ash sawdust on the right coat cuff and the right knee (sawing the tripod leg, A28).
Stains are dark brown, never red (red is the investigation signal).

Input:  art_src/Generated/Characters/_review/2_meshy71_ultra_TRIPO_RIG_78j.glb (Meshy 7.1 Ultra + Tripo rig)
Output: art_src/Generated/Characters/CH01/CH01_Detective_D06.glb, T_CH01_BaseColor_D06.png, T_CH01_BaseColor.png
        (clean), CH01_D06_mask.png (debug: where paint landed).

Regions come from the rig: each triangle belongs to the joint with the largest skin weight on its first vertex.
Painting runs in texture space by sampling every selected triangle densely and splatting into its texels.
Run: python Tools/art/ch01_clues.py   (numpy + Pillow only)
"""
import io
import json
import struct
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "art_src/Generated/Characters/_review/2_meshy71_ultra_TRIPO_RIG_78j.glb"
OUT = ROOT / "art_src/Generated/Characters/CH01"
HEIGHT_M = 1.85

SALT = np.array([214, 209, 196], np.float32) / 255
SKIN = np.array([196, 160, 136], np.float32) / 255
CUT = np.array([58, 36, 26], np.float32) / 255          # dried, dark brown: never red
SCUFF = np.array([118, 92, 74], np.float32) / 255       # torn leather edge
ASH = np.array([206, 186, 146], np.float32) / 255       # pale ash wood
DUST = np.array([200, 182, 140], np.float32) / 255


# ---------------------------------------------------------------- glb io
def read_glb(path):
    d = path.read_bytes()
    jl = struct.unpack_from("<I", d, 12)[0]
    g = json.loads(d[20:20 + jl])
    bin_off = 20 + jl + 8
    blen = struct.unpack_from("<I", d, 20 + jl)[0]
    return g, bytearray(d[bin_off:bin_off + blen])


def accessor(g, b, i):
    a = g["accessors"][i]
    bv = g["bufferViews"][a["bufferView"]]
    dt = {5126: np.float32, 5125: np.uint32, 5123: np.uint16, 5121: np.uint8}[a["componentType"]]
    nc = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4}[a["type"]]
    off = bv.get("byteOffset", 0) + a.get("byteOffset", 0)
    stride = bv.get("byteStride")
    item = np.dtype(dt).itemsize * nc
    if stride and stride != item:
        raw = np.frombuffer(b, np.uint8, a["count"] * stride, off).reshape(a["count"], stride)[:, :item]
        arr = np.frombuffer(raw.tobytes(), dt).reshape(a["count"], nc)
    else:
        arr = np.frombuffer(b, dt, a["count"] * nc, off).reshape(a["count"], nc)
    return arr.copy()


def write_glb(path, g, b, image_index, new_bytes, mime):
    """Rebuild the binary chunk with one image replaced."""
    target = g["images"][image_index]["bufferView"]
    out = bytearray()
    for i, bv in enumerate(g["bufferViews"]):
        src = new_bytes if i == target else b[bv.get("byteOffset", 0):bv.get("byteOffset", 0) + bv["byteLength"]]
        while len(out) % 4:
            out.append(0)
        bv["byteOffset"] = len(out)
        bv["byteLength"] = len(src)
        out += src
    while len(out) % 4:
        out.append(0)
    g["buffers"][0]["byteLength"] = len(out)
    g["images"][image_index]["mimeType"] = mime
    js = json.dumps(g, separators=(",", ":")).encode()
    js += b" " * (-len(js) % 4)
    total = 12 + 8 + len(js) + 8 + len(out)
    path.write_bytes(struct.pack("<III", 0x46546C67, 2, total) + struct.pack("<II", len(js), 0x4E4F534A) + js
                     + struct.pack("<II", len(out), 0x004E4942) + bytes(out))


# ---------------------------------------------------------------- noise
_rng = np.random.default_rng(1949)
_PERM = _rng.random(256 ** 1 * 4096).astype(np.float32)


def vnoise(p):
    """3D value noise in [0, 1], p: (N, 3)."""
    i = np.floor(p).astype(np.int64)
    f = p - i
    f = f * f * (3 - 2 * f)

    def h(dx, dy, dz):
        k = ((i[:, 0] + dx) * 73856093) ^ ((i[:, 1] + dy) * 19349663) ^ ((i[:, 2] + dz) * 83492791)
        return _PERM[k % _PERM.size]

    x0 = h(0, 0, 0) * (1 - f[:, 0]) + h(1, 0, 0) * f[:, 0]
    x1 = h(0, 1, 0) * (1 - f[:, 0]) + h(1, 1, 0) * f[:, 0]
    x2 = h(0, 0, 1) * (1 - f[:, 0]) + h(1, 0, 1) * f[:, 0]
    x3 = h(0, 1, 1) * (1 - f[:, 0]) + h(1, 1, 1) * f[:, 0]
    y0 = x0 * (1 - f[:, 1]) + x1 * f[:, 1]
    y1 = x2 * (1 - f[:, 1]) + x3 * f[:, 1]
    return y0 * (1 - f[:, 2]) + y1 * f[:, 2]


def fbm(p, octaves=4):
    s, a, t = 0.0, 0.5, 0.0
    for o in range(octaves):
        s = s + a * vnoise(p * (2 ** o) + o * 17.3)
        t += a
        a *= 0.5
    return s / t


def smooth(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0, 1)
    return t * t * (3 - 2 * t)


# ---------------------------------------------------------------- sampling
def sample_tris(tri_ids, idx, pos, nrm, uv, tex_w, tex_h):
    """Dense barycentric samples over the given triangles. Returns pos, normal, texel index."""
    v = idx[tri_ids]
    puv = uv[v] * [tex_w, tex_h]
    e1, e2 = puv[:, 1] - puv[:, 0], puv[:, 2] - puv[:, 0]
    area = 0.5 * np.abs(e1[:, 0] * e2[:, 1] - e1[:, 1] * e2[:, 0])
    res = np.clip(np.ceil(np.sqrt(area * 4)).astype(int) + 1, 2, 96)
    P, N, T = [], [], []
    for r in np.unique(res):
        sel = np.where(res == r)[0]
        ij = np.array([(a, b) for a in range(r + 1) for b in range(r + 1 - a)], np.float32) / r
        bary = np.stack([1 - ij.sum(1), ij[:, 0], ij[:, 1]], 1)            # (k, 3)
        for chunk in np.array_split(sel, max(1, len(sel) * len(bary) // 2_000_000 + 1)):
            vv = v[chunk]
            p = np.einsum("kc,tcd->tkd", bary, pos[vv]).reshape(-1, 3)
            n = np.einsum("kc,tcd->tkd", bary, nrm[vv]).reshape(-1, 3)
            t = np.einsum("kc,tcd->tkd", bary, uv[vv]).reshape(-1, 2)
            px = np.clip((t[:, 0] * tex_w).astype(int), 0, tex_w - 1)
            py = np.clip((t[:, 1] * tex_h).astype(int), 0, tex_h - 1)
            P.append(p); N.append(n); T.append(py * tex_w + px)
    n = np.concatenate(N)
    n /= np.linalg.norm(n, axis=1, keepdims=True) + 1e-9
    return np.concatenate(P), n, np.concatenate(T)


def splat(layer, texel, alpha):
    np.maximum.at(layer, texel, alpha.astype(np.float32))


def dilate(a, w, h):
    m = a.reshape(h, w)
    out = m.copy()
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            out = np.maximum(out, np.roll(np.roll(m, dy, 0), dx, 1) * 0.85)
    return out.reshape(-1)


def seg_dist(p2, a, b):
    ab = b - a
    t = np.clip(((p2 - a) @ ab) / (ab @ ab), 0, 1)
    return np.linalg.norm(p2 - (a + t[:, None] * ab), axis=1), t


# ---------------------------------------------------------------- main
def main():
    g, b = read_glb(SRC)
    prim = g["meshes"][0]["primitives"][0]
    at = prim["attributes"]
    pos = accessor(g, b, at["POSITION"]).astype(np.float64)
    nrm = accessor(g, b, at["NORMAL"]).astype(np.float64)
    uv = accessor(g, b, at["TEXCOORD_0"]).astype(np.float64)
    joints = accessor(g, b, at["JOINTS_0"])
    weights = accessor(g, b, at["WEIGHTS_0"])
    idx = accessor(g, b, prim["indices"]).reshape(-1, 3)

    scale = HEIGHT_M / pos[:, 1].max()
    pos = pos * scale                                       # metres, feet at y = 0

    skin = g["skins"][0]
    jname = [g["nodes"][n]["name"] for n in skin["joints"]]
    dom = joints[np.arange(len(joints)), weights.argmax(1)]
    vname = np.array(jname)[dom]
    tname = vname[idx[:, 0]]

    # orientation: the character faces the side the toes point to; right = R_Hand side
    toe = pos[np.isin(vname, ["L_ToeBase", "R_ToeBase"])].mean(0)
    foot = pos[np.isin(vname, ["L_Foot", "R_Foot"])].mean(0)
    fwd = np.sign(toe[2] - foot[2])
    rhand_v = pos[vname == "R_Hand"]
    right_sign = np.sign(rhand_v[:, 0].mean())
    print(f"scale {scale:.3f}  forward z{'+' if fwd > 0 else '-'}  right x{'+' if right_sign > 0 else '-'}")

    bc_img = g["textures"][g["materials"][0]["pbrMetallicRoughness"]["baseColorTexture"]["index"]]["source"]
    bvi = g["bufferViews"][g["images"][bc_img]["bufferView"]]
    img = Image.open(io.BytesIO(bytes(b[bvi.get("byteOffset", 0):bvi.get("byteOffset", 0) + bvi["byteLength"]])))
    img = img.convert("RGB")
    W, H = img.size
    base = np.asarray(img, np.float32).reshape(-1, 3) / 255
    print("base colour", W, H)
    OUT.mkdir(parents=True, exist_ok=True)
    img.save(OUT / "T_CH01_BaseColor.png")

    layers = []   # (colour, alpha array) composited in order

    def layer():
        a = np.zeros(W * H, np.float32)
        return a

    # --- 1. salt: tide line shin-high on both legs, bloom below, marks on the shoes
    leg_j = [n for n in jname if any(k in n for k in ("Calf", "Foot", "ToeBase", "KneeShare"))]
    tri_cent_y = pos[idx].mean(1)[:, 1]
    sel = np.where(np.isin(tname, leg_j) & (tri_cent_y < 0.42))[0]
    p, n, t = sample_tris(sel, idx, pos, nrm, uv, W, H)
    y = p[:, 1]
    line_h = 0.285 + 0.05 * (fbm(p * 6.0) - 0.5) + 0.01 * (vnoise(p * 40) - 0.5)
    broken = smooth(0.3, 0.55, fbm(p * 22 + 5))            # the line is dry and patchy, not painted on
    line = np.exp(-((y - line_h) / 0.0028) ** 2) * broken * (0.5 + 0.5 * fbm(p * 80))
    bloom = (y < line_h) * smooth(0.0, 0.03, line_h - y) * smooth(0.66, 0.9, fbm(p * 60, 3)) * 0.35
    haze = (y < line_h) * smooth(0.0, 0.05, line_h - y) * 0.07
    shoe = (y < 0.075) * np.exp(-((y - 0.05 - 0.012 * fbm(p * 20)) / 0.003) ** 2) * broken * 0.45
    a_salt = layer()
    splat(a_salt, t, np.clip(np.maximum.reduce([line * 0.7, bloom, haze, shoe]), 0, 0.8))
    layers.append((SALT, dilate(a_salt, W, H)))

    # --- 2. right glove: palm tear, cut, splinter
    hand_sel = np.where(np.isin(tname, ["R_Hand"]))[0]
    p, n, t = sample_tris(hand_sel, idx, pos, nrm, uv, W, H)
    palm = smooth(0.25, 0.6, -n[:, 1])                        # T-pose, palms down
    hv = rhand_v
    cx, cz = hv[:, 0].mean(), hv[:, 2].mean()
    # across the palm, slightly diagonal, from the heel of the hand toward the index side
    a2 = np.array([cx - right_sign * 0.012, cz - 0.03 * fwd])
    b2 = np.array([cx + right_sign * 0.022, cz + 0.032 * fwd])
    p2 = p[:, [0, 2]]
    d, s = seg_dist(p2, a2, b2)
    width = (0.008 + 0.003 * (fbm(p * 300) - 0.5)) * np.sin(np.pi * np.clip(s, 0.02, 0.98)) ** 0.5
    hole = smooth(0.0008, -0.0004, d - width) * palm
    rim = np.exp(-((d - width) / 0.0018) ** 2) * palm * (0.6 + 0.4 * fbm(p * 500))
    cut = np.exp(-(d / 0.0006) ** 2) * (s > 0.08) * (s < 0.9) * palm
    bruise = np.exp(-(d / 0.0035) ** 2) * hole * 0.25
    # splinter: a thin pale sliver lodged at the tear, angled off the cut
    sa = a2 + (b2 - a2) * 0.55
    sdir = np.array([right_sign * 0.9, -0.45 * fwd]); sdir /= np.linalg.norm(sdir)
    ds, ss = seg_dist(p2, sa, sa + sdir * 0.022)
    splinter = smooth(0.0013, 0.0007, ds * (1 + 0.6 * ss)) * palm
    for col, al in ((SCUFF, rim * 0.8), (SKIN, hole), (CUT, np.maximum(cut * 0.9, bruise)), (ASH, splinter)):
        a = layer(); splat(a, t, np.clip(al, 0, 1)); layers.append((col, dilate(a, W, H)))
    print("palm samples", int((palm > 0.5).sum()), "tear samples", int((hole > 0.5).sum()))

    # --- 3. sawdust on the right coat cuff and the right knee
    fore_j = [n for n in jname if n.startswith("R_Forearm")]
    sel = np.where(np.isin(tname, fore_j))[0]
    p, n, t = sample_tris(sel, idx, pos, nrm, uv, W, H)
    cuff_x = np.percentile(np.abs(pos[np.isin(vname, fore_j)][:, 0]), 99)
    near_cuff = smooth(-0.07, -0.015, np.abs(p[:, 0]) - cuff_x)
    up_front = smooth(0.0, 0.7, np.maximum(n[:, 1], n[:, 2] * fwd))   # dust settles on top and front
    clusters = smooth(0.45, 0.7, fbm(p * 30 + 3))
    grains = smooth(0.78, 0.86, vnoise(p * 1100)) * clusters + 0.12 * clusters
    a = layer(); splat(a, t, np.clip(near_cuff * up_front * grains, 0, 0.9)); layers.append((DUST, a))

    knee_j = [n for n in jname if n.startswith(("R_Calf", "R_Thigh", "R_KneeShare"))]
    sel = np.where(np.isin(tname, knee_j))[0]
    p, n, t = sample_tris(sel, idx, pos, nrm, uv, W, H)
    kv = pos[np.isin(vname, ["R_KneeShareBone"])]
    knee = kv.mean(0) if len(kv) else np.array([right_sign * 0.1, 0.5, 0])
    calf = pos[np.isin(vname, [j for j in knee_j if "Calf" in j]) & (pos[:, 1] > 0.40) & (pos[:, 1] < 0.52)]
    z_front = np.percentile(calf[:, 2] * fwd, 97) * fwd
    on_trousers = (p[:, 2] * fwd < z_front * fwd + 0.02) & (p[:, 1] < 0.56)   # the coat skirt is further out
    knee = np.array([calf[:, 0].mean(), 0.47, z_front])
    front = smooth(0.2, 0.7, n[:, 2] * fwd) * on_trousers
    r = np.linalg.norm((p - knee)[:, [0, 1]] * [1.0, 0.8], axis=1)
    patch = smooth(0.075, 0.02, r + 0.025 * (fbm(p * 25) - 0.5))
    clusters = smooth(0.4, 0.7, fbm(p * 30 + 9))
    grains = smooth(0.76, 0.85, vnoise(p * 1100)) * clusters + 0.12 * clusters
    a = layer(); splat(a, t, np.clip(patch * front * grains, 0, 0.9)); layers.append((DUST, a))
    print("knee centre", np.round(knee, 3))

    # --- composite
    out = base.copy()
    mask = np.zeros_like(base)
    for col, a in layers:
        out = out * (1 - a[:, None]) + col * a[:, None]
        mask = mask * (1 - a[:, None]) + col * a[:, None]
    res = Image.fromarray((np.clip(out, 0, 1) * 255 + 0.5).astype(np.uint8).reshape(H, W, 3))
    res.save(OUT / "T_CH01_BaseColor_D06.png")
    Image.fromarray((mask * 255).astype(np.uint8).reshape(H, W, 3)).resize((1024, 1024)).save(OUT / "CH01_D06_mask.png")
    buf = io.BytesIO(); res.save(buf, "JPEG", quality=95)
    write_glb(OUT / "CH01_Detective_D06.glb", g, b, bc_img, buf.getvalue(), "image/jpeg")
    print("wrote", OUT)


if __name__ == "__main__":
    main()
