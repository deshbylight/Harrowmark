"""Generate original front-facing portraits for the NPCs (Headshot input in Character Creator).

Faces are invented by the model (no real people). Output:
art_src/Generated/Characters/<id>/portrait/<id>_portrait_<n>.png

Usage: python Tools/art/gen_portraits.py [CH03 CH04 ...] [--n 2]
Needs FAL_KEY in .env. Model: fal-ai/flux-pro/v1.1-ultra (raw mode), about $0.06 per image.
"""
import argparse
import os
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "art_src" / "Generated" / "Characters"
MODEL = "fal-ai/flux-pro/v1.1-ultra"

# Headshot wants: frontal, neutral, mouth closed, eyes open, even light, no hat or glasses,
# forehead and ears visible, plain background. Hats and costumes come later from CC content.
BASE = (
    "Passport-style studio photograph, head and upper shoulders, of {who}. "
    "Looking straight into the camera, head level, neutral relaxed expression, mouth closed, eyes open. "
    "Even soft frontal lighting, no harsh shadows, plain light grey seamless background. "
    "No hat, no glasses, forehead and both ears visible. {hair} "
    "Wearing {top}. Natural skin texture with pores and fine wrinkles, sharp focus, 85mm lens, "
    "colour photograph, realistic, not a celebrity."
)

CAST = {
    "CH02": dict(
        who="a 44-year-old Irish-American customs officer, average build, tired grey-blue eyes, clean-shaven, weathered harbour skin",
        hair="Short side-parted brown hair with grey at the temples, 1940s cut.",
        top="a navy wool uniform jacket collar over a white shirt and dark tie",
    ),
    "CH03": dict(
        who="a 46-year-old Hungarian-American customs officer, tall and lean, long face, hooded dark eyes, thick dark moustache, olive skin",
        hair="Slicked-back black hair, slightly receding, 1940s cut.",
        top="a navy wool uniform jacket collar over a white shirt and dark tie",
    ),
    "CH04": dict(
        who="a 38-year-old woman who runs a harbour bar, small and sturdy, sharp intelligent green eyes, a few freckles, no makeup except light powder",
        hair="Auburn-brown hair pinned up in a 1940s working updo, a few loose strands.",
        top="a dark green wool dress with a plain round collar and a grey cardigan",
    ),
    "CH05": dict(
        who="a 56-year-old heavy-set dock foreman, broad face, broken nose, small pale eyes, stubble, ruddy wind-burnt cheeks",
        hair="Thinning cropped grey hair.",
        top="a dark olive oilskin jacket collar over a grey wool knit sweater",
    ),
    "CH06": dict(
        who="a 55-year-old Dutch merchant ship captain, square jaw, deep-set blue eyes, full neatly trimmed grey-blond beard",
        hair="Short swept-back grey-blond hair.",
        top="a navy wool peacoat with a turned-up collar over a dark roll-neck sweater",
    ),
    "CH07": dict(
        who="a 52-year-old police sergeant, stocky, heavy jowls, calm hooded brown eyes, clean-shaven, slight double chin",
        hair="Short grey hair cut close at the sides.",
        top="a dark navy police tunic collar with brass buttons over a light blue shirt and black tie",
    ),
}


def load_key():
    env = ROOT / ".env"
    if env.exists():
        for line in env.read_text(encoding="utf-8").splitlines():
            if line.startswith("FAL_KEY=") and not os.environ.get("FAL_KEY"):
                os.environ["FAL_KEY"] = line.split("=", 1)[1].strip().strip('"')
    if not os.environ.get("FAL_KEY"):
        sys.exit("FAL_KEY missing")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("ids", nargs="*", default=list(CAST))
    ap.add_argument("--n", type=int, default=2)
    ap.add_argument("--seed", type=int, default=1949)
    a = ap.parse_args()
    load_key()
    import fal_client

    for cid in a.ids:
        c = CAST[cid]
        prompt = BASE.format(**c)
        d = OUT / cid / "portrait"
        d.mkdir(parents=True, exist_ok=True)
        (d / "prompt.txt").write_text(f"{MODEL}\n{prompt}\n", encoding="utf-8")
        for i in range(a.n):
            dst = d / f"{cid}_portrait_{i + 1}.png"
            if dst.exists():
                print("skip", dst)
                continue
            r = fal_client.subscribe(MODEL, arguments=dict(
                prompt=prompt, aspect_ratio="3:4", raw=True, num_images=1,
                output_format="png", safety_tolerance="2", seed=a.seed + i,
            ))
            urllib.request.urlretrieve(r["images"][0]["url"], dst)
            print("ok", dst)


if __name__ == "__main__":
    main()
