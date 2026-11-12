"""Render the clue document textures of case 1 (base colour PNGs) with Pillow.

Usage: python render_docs.py <fonts_dir> <out_dir> [case01.json]

Every clue line that a card quotes is checked against the card texts in case01.json, so the page in the
world always says what the card says. No red anywhere: red is reserved for the investigation.
"""
from __future__ import annotations

import json
import math
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

FONTS = Path(sys.argv[1])
OUT = Path(sys.argv[2])
CASE = Path(sys.argv[3]) if len(sys.argv) > 3 else Path(__file__).resolve().parent.parent / "data" / "case01.json"

TYPE = FONTS / "apache_specialelite_SpecialElite-Regular.ttf"
HAND = FONTS / "apache_homemadeapple_HomemadeApple-Regular.ttf"  # Morrow's hand and Varga's signature only
LOG = FONTS / "ofl_caveat_Caveat.ttf"  # everyone else: legible capitals for logs and chalk
PRINT = FONTS / "ofl_courierprime_CourierPrime-Regular.ttf"
PRINT_B = FONTS / "ofl_courierprime_CourierPrime-Bold.ttf"

PAPER = (232, 220, 194)
INK_BLUE = (31, 42, 68)
INK_PENCIL = (70, 70, 72)
INK_TYPE = (26, 24, 24)
INK_PRINT = (52, 58, 54)
INK_VIOLET = (78, 52, 112)
RULE_BLUE = (143, 166, 192)

CARD_TEXT = " ".join(c["text"] for c in json.loads(CASE.read_text(encoding="utf-8"))["cards"])
rng = np.random.default_rng(1949)


def quoted(s: str) -> str:
    """A clue line that must appear word for word on its card."""
    if s not in CARD_TEXT:
        raise SystemExit(f"clue text not found on any card: {s!r}")
    return s


def font(path: Path, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(str(path), size)


# ---------- paper ----------

def noise(w: int, h: int, cell: int) -> np.ndarray:
    small = rng.random((h // cell + 2, w // cell + 2)).astype(np.float32)
    img = Image.fromarray((small * 255).astype(np.uint8)).resize((w + cell, h + cell), Image.BICUBIC)
    return np.asarray(img, dtype=np.float32)[:h, :w] / 255.0


def paper(w: int, h: int, base=PAPER, age: float = 0.5, stains: int = 3) -> np.ndarray:
    arr = np.ones((h, w, 3), np.float32) * np.array(base, np.float32)
    shade = 1 - age * 0.16 * (noise(w, h, 256) - 0.5) - 0.05 * (noise(w, h, 24) - 0.5)
    shade += rng.normal(0, 0.012, (h, w)).astype(np.float32)
    yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
    d = np.sqrt(((xx - w / 2) / (w / 2)) ** 2 + ((yy - h / 2) / (h / 2)) ** 2)
    shade *= 1 - 0.22 * age * np.clip(d, 0, 1.5) ** 3
    arr *= shade[..., None]
    for _ in range(stains):
        arr = stain(arr, rng.uniform(0.1, 0.9) * w, rng.uniform(0.1, 0.9) * h, rng.uniform(0.06, 0.2) * w, 0.5 * age)
    return arr


def stain(arr: np.ndarray, cx: float, cy: float, r: float, strength: float) -> np.ndarray:
    """A tide mark: tinted blot with a darker rim."""
    h, w, _ = arr.shape
    m = Image.new("L", (w, h), 0)
    ImageDraw.Draw(m).ellipse([cx - r, cy - r * 0.8, cx + r, cy + r * 0.8], fill=255)
    m = m.filter(ImageFilter.GaussianBlur(r * 0.08))
    inner = m.filter(ImageFilter.GaussianBlur(r * 0.12))
    fill = np.asarray(m, np.float32) / 255
    rim = np.clip(fill - np.asarray(inner, np.float32) / 255 * 0.9, 0, 1)
    warp = 0.6 + 0.8 * noise(w, h, max(8, int(r / 6)))
    tint = np.array([0.88, 0.80, 0.66], np.float32)
    k = (fill * 0.35 + rim * 1.6) * strength * warp
    return arr * (1 - k[..., None] * (1 - tint))


def ruled(arr: np.ndarray, top: int, step: int, x0: int, x1: int, margin: int | None, color=RULE_BLUE, a=0.45):
    h, w, _ = arr.shape
    m = Image.new("L", (w, h), 0)
    d = ImageDraw.Draw(m)
    for y in range(top, h - step // 2, step):
        d.line([(x0, y), (x1, y)], fill=255, width=2)
    if margin is not None:
        d.line([(margin, 0), (margin, h)], fill=255, width=2)
    return apply_ink(arr, m.filter(ImageFilter.GaussianBlur(0.7)), color, a)


def apply_ink(arr: np.ndarray, mask: Image.Image, color, opacity: float = 1.0) -> np.ndarray:
    a = np.asarray(mask, np.float32)[..., None] / 255 * opacity
    return arr * (1 - a) + np.array(color, np.float32) * a


def save(arr: np.ndarray, name: str) -> Path:
    OUT.mkdir(parents=True, exist_ok=True)
    p = OUT / name
    Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8), "RGB").save(p, optimize=True)
    print("wrote", p)
    return p


# ---------- writing ----------

class Layer:
    """A coverage mask the size of the page; draw text into it, then put it on paper with an ink colour."""

    def __init__(self, w: int, h: int):
        self.img = Image.new("L", (w, h), 0)

    def typed(self, x: float, y: float, text: str, f: ImageFont.FreeTypeFont, jitter: float = 1.2) -> None:
        """Typewriter: fixed pitch, each strike slightly off line and of uneven weight."""
        d = ImageDraw.Draw(self.img)
        pitch = f.getlength("M")
        for i, ch in enumerate(text):
            if ch == " ":
                continue
            dx, dy = rng.normal(0, jitter * 0.4), rng.normal(0, jitter)
            d.text((x + i * pitch + dx, y + dy), ch, font=f, fill=int(rng.uniform(170, 255)))

    def printed(self, x: float, y: float, text: str, f: ImageFont.FreeTypeFont, anchor: str = "la") -> None:
        ImageDraw.Draw(self.img).text((x, y), text, font=f, fill=235, anchor=anchor)

    def hand(self, x: float, y: float, text: str, f: ImageFont.FreeTypeFont, slope: float = 0.0,
             wobble: float = 2.0, pressure=(190, 255)) -> float:
        """Handwriting: word by word with baseline wobble, the line tilted by `slope` degrees.
        Returns the x after the last word."""
        words = text.split(" ")
        space = f.getlength(" ") * 0.9
        width = int(sum(f.getlength(wd) for wd in words) + space * len(words) + 40)
        height = int(f.size * 2.6)
        tmp = Image.new("L", (width, height), 0)
        d = ImageDraw.Draw(tmp)
        cx = 10.0
        for wd in words:
            d.text((cx, height * 0.3 + rng.normal(0, wobble)), wd, font=f, fill=int(rng.uniform(*pressure)))
            cx += f.getlength(wd) + space * rng.uniform(0.85, 1.15)
        tmp = tmp.rotate(slope + rng.normal(0, 0.3), resample=Image.BICUBIC, expand=False)
        self.paste(tmp, int(x - 10), int(y - height * 0.3))
        return x + cx

    def paste(self, tmp: Image.Image, x: int, y: int) -> None:
        region = Image.new("L", self.img.size, 0)
        region.paste(tmp, (x, y))
        self.img = ImageChops.lighter(self.img, region)

    def line(self, pts, width: int = 2, fill: int = 235) -> None:
        ImageDraw.Draw(self.img).line(pts, fill=fill, width=width)

    def rect(self, box, width: int = 2) -> None:
        ImageDraw.Draw(self.img).rectangle(box, outline=235, width=width)

    def soft(self, r: float) -> Image.Image:
        return self.img.filter(ImageFilter.GaussianBlur(r))


def distress(mask: Image.Image, amount: float, cell: int = 6) -> Image.Image:
    """Break up a stamp or chalk stroke with noise."""
    w, h = mask.size
    n = noise(w, h, cell) * 0.6 + rng.random((h, w)).astype(np.float32) * 0.4
    a = np.asarray(mask, np.float32) * np.clip((n - amount) * 3, 0, 1)
    return Image.fromarray(a.astype(np.uint8))


def bleed(arr: np.ndarray, layer: Layer, color, wet: float) -> np.ndarray:
    """Ink that has been in the water: a soft halo, a blurred body and a sharp core."""
    halo = layer.soft(6 * wet)
    arr = apply_ink(arr, halo, color, 0.35 * wet)
    body = Image.blend(layer.soft(0.8), layer.soft(2.2 * wet), 0.5)
    return apply_ink(arr, body, color, 0.92)


# ---------- documents ----------

MARGIN_NOTE = quoted("slewed short 23:00-23:50 - CL-2 under the swing.")
SPLASH = quoted("splash 23:41 W side?")
# No. 2 hold, 140 cases from 23:00 to 00:30 (crane log, tally): a line every five minutes, two lifts to a line
# (a lift every two or three minutes, so the crane never stood idle long enough for Holt to leave the cab)
CASES = [8 if i % 3 == 0 else 7 for i in range(19)]
assert sum(CASES) == 140


def notebook() -> None:
    """C02 / A03-A05: Morrow's pocket notebook, two soaked pages."""
    w, h = 2048, 1536
    arr = paper(w, h, base=(226, 216, 190), age=0.9, stains=0)
    for cx, cy, r in [(620, 1120, 420), (1500, 380, 360), (1180, 1300, 300)]:
        arr = stain(arr, cx, cy, r, 0.8)
    for x0 in (60, w // 2 + 40):
        arr = ruled(arr, 230, 92, x0, x0 + w // 2 - 100, None, a=0.35)
    gutter = Image.new("L", (w, h), 0)
    ImageDraw.Draw(gutter).rectangle([w // 2 - 6, 0, w // 2 + 6, h], fill=255)
    arr = apply_ink(arr, gutter.filter(ImageFilter.GaussianBlur(30)), (60, 50, 40), 0.45)

    f = font(HAND, 40)
    small = font(HAND, 30)
    ink = Layer(w, h)
    # left page: the date, then Holt and the debt
    ink.hand(110, 120, "Fri 11 Nov", small, slope=0.6)
    ink.hand(110, 300, quoted("Holt - crates short again."), f, slope=0.8)
    ink.hand(110, 392, quoted("Report Monday."), f, slope=0.5)
    quoted("Owe N.V. $40 - pay Friday.")  # Homemade Apple's $ reads as #, so the $ is Caveat's
    xd = ink.hand(110, 668, "Owe N.V.", f, slope=0)
    dollar = font(LOG, 62)
    ImageDraw.Draw(ink.img).text((xd, 668 - 8), "$", font=dollar, fill=235)
    ink.hand(xd + dollar.getlength("$") + 2, 668, "40 - pay Friday.", f, slope=0)
    # right page: the Agatha and the meeting; the name is washed out
    x2 = w // 2 + 100
    ink.hand(x2, 300, quoted("AGATHA - manifest wrong."), f, slope=-0.5)
    end = ink.hand(x2, 392, "Cleared by", f, slope=-0.3)
    ink.hand(x2, 576, quoted("Meet H.M. 23:30, P5."), f, slope=0.4)  # H.M. = HarrowMark; read as the harbour master
    arr = bleed(arr, ink, INK_BLUE, wet=1.0)

    # the smudge: scribbled ink wiped by a wet thumb, illegible by construction (no letters drawn)
    smear = Layer(w, h)
    for k in range(5):
        pts = [(end + 20 + i * 8 + rng.normal(0, 3), 410 + 22 * math.sin(i * rng.uniform(0.7, 1.6) + k)
                + rng.normal(0, 6) + k * 6) for i in range(36)]
        smear.line(pts, width=int(rng.integers(4, 9)), fill=int(rng.integers(150, 230)))
    acc = np.zeros((h, w), np.float32)
    for t in range(0, 60, 3):
        acc = np.maximum(acc, np.asarray(ImageChops.offset(smear.img, t, int(t * 0.5)), np.float32) * (1 - t / 75))
    blot = Image.fromarray(acc.astype(np.uint8)).filter(ImageFilter.GaussianBlur(9))
    arr = apply_ink(arr, blot, INK_BLUE, 0.7)
    arr = apply_ink(arr, smear.soft(5), INK_BLUE, 0.6)
    arr = stain(arr, end + 170, 430, 150, 0.9)
    save(arr, "T_C02_NotebookPages_D.png")


def crane_log() -> None:
    """C07 / A10: the crane log on a clipboard, two lifts to every five-minute line, operator S.H."""
    w, h = 1536, 2048
    arr = paper(w, h, age=0.6, stains=2)
    pr = Layer(w, h)
    pr.printed(w / 2, 90, "HARROWMARK HARBOUR BOARD", font(PRINT_B, 44), "ma")
    pr.printed(w / 2, 150, "CRANE LOG  -  CRANE No. 3", font(PRINT_B, 36), "ma")
    pr.printed(120, 240, "Vessel:", font(PRINT, 30))
    pr.printed(820, 240, "Date:", font(PRINT, 30))
    cols = [120, 420, 640, 1150, 1416]
    for x, t in zip(cols, ["TIME", "HOLD", "LIFTS", "OP."]):
        pr.printed(x + 16, 352, t, font(PRINT_B, 30))
    top, row = 400, 78
    rows = 19
    for i in range(rows + 1):
        pr.line([(cols[0], top + i * row), (cols[-1], top + i * row)], 2)
    pr.line([(cols[0], 340), (cols[-1], 340)], 3)
    for x in cols:
        pr.line([(x, 340), (x, top + rows * row)], 2)
    arr = apply_ink(arr, pr.soft(0.7), INK_PRINT, 0.9)

    hw = Layer(w, h)
    f = font(LOG, 42)
    hw.hand(250, 232, "SS Saint Agatha", f, slope=0.5, wobble=1)
    hw.hand(920, 238, "11/12 Nov 49", f, slope=0.3, wobble=1)
    for i in range(rows):
        t = 23 * 60 + i * 5
        hhmm = f"{(t // 60) % 24:02d}:{t % 60:02d}"
        y = top + i * row + 14
        hw.hand(cols[0] + 22, y, hhmm, f, slope=rng.normal(0, 0.6), wobble=1)
        hw.hand(cols[1] + 22, y, "No.2", f, slope=rng.normal(0, 0.6), wobble=1)
        hw.hand(cols[2] + 22, y, f"x{(CASES[i] + 1) // 2}  x{CASES[i] // 2}", f, slope=rng.normal(0, 0.6), wobble=1)
        if hhmm == "23:40":  # Holt heard it, wrote it down, and kept slewing
            hw.hand(cols[2] + 210, y + 4, SPLASH, font(LOG, 30), slope=-1.5, wobble=0.6)
        hw.hand(cols[3] + 22, y, "S.H.", f, slope=rng.normal(0, 0.6), wobble=1)
    arr = apply_ink(arr, hw.soft(0.6), INK_PENCIL, 0.85)
    # D02: a tiny pencil note up the right margin beside the 23:40 row, only readable under the loupe
    note = Layer(560, 60)
    note.hand(10, 12, MARGIN_NOTE, font(LOG, 22), wobble=0.6)
    side = note.img.rotate(90, resample=Image.BICUBIC, expand=True)
    full = Image.new("L", (w, h), 0)
    full.paste(side, (cols[-1] + 40, top + 4 * row))
    arr = apply_ink(arr, full.filter(ImageFilter.GaussianBlur(0.5)), INK_PENCIL, 0.6)
    save(arr, "T_C07_CraneLog_D.png")
    quoted("a lift every two or three minutes, 23:00-00:30, operator S.H.")


def gate_book() -> None:
    """C08 / A11: the gate visitors' book, open at the night of 11 November."""
    w, h = 2048, 1536
    arr = paper(w, h, age=0.6, stains=2)
    pr = Layer(w, h)
    cols = [90, 330, 900, 1330, 1700, 1960]
    for x, t in zip(cols, ["TIME IN", "NAME", "BUSINESS", "TIME OUT", "INIT."]):
        pr.printed(x + 14, 120, t, font(PRINT_B, 30))
    top, row = 180, 96
    for i in range(14):
        pr.line([(cols[0], top + i * row), (cols[-1], top + i * row)], 2)
    for x in cols:
        pr.line([(x, 100), (x, top + 13 * row)], 2)
    pr.line([(cols[0], 100), (cols[-1], 100)], 3)
    arr = apply_ink(arr, pr.soft(0.7), INK_PRINT, 0.9)

    hw = Layer(w, h)
    f = font(LOG, 46)
    hw.hand(cols[1] + 20, top + 22, "Friday 11th November 1949 - night", f, slope=0.4)
    entries = [
        # card A11: "23:25 Capt. Kordes in, harbour master. 23:55 out."
        ("23:25", "Capt. Kordes", "harbour master", "23:55", "W.B."),
        ("01:25", "Police - Sgt. Brannock", "body on Pier 5", "", "W.B."),
        ("02:00", "HarrowMark agency", "Sgt. Brannock", "", ""),  # keeper away at the van: signed himself in
    ]
    assert quoted("23:25 Capt. Kordes in, harbour master. 23:55 out.")
    for i, e in enumerate(entries):
        y = top + (i + 1) * row + 24
        for x, t in zip(cols, e):
            if t:
                hw.hand(x + 20, y, t, f, slope=rng.normal(0, 0.5), wobble=1.2)
    arr = apply_ink(arr, hw.soft(0.8), INK_BLUE, 0.9)
    save(arr, "T_C08_GateBook_D.png")


def ship_log() -> None:
    """C10 / A14: SS Saint Agatha's gangway watch book, on a shelf at the head of the gangway."""
    w, h = 2048, 1536
    arr = paper(w, h, base=(225, 214, 186), age=0.7, stains=3)
    pr = Layer(w, h)
    pr.printed(w / 2, 60, "SS SAINT AGATHA  -  GANGWAY WATCH", font(PRINT_B, 42), "ma")
    cols = [90, 300, 1960]
    pr.printed(cols[0] + 14, 140, "TIME", font(PRINT_B, 30))
    pr.printed(cols[1] + 14, 140, "REMARKS", font(PRINT_B, 30))
    top, row = 200, 96
    for i in range(14):
        pr.line([(cols[0], top + i * row), (cols[-1], top + i * row)], 2)
    for x in cols:
        pr.line([(x, 120), (x, top + 13 * row)], 2)
    pr.line([(cols[0], 120), (cols[-1], 120)], 3)
    arr = apply_ink(arr, pr.soft(0.7), INK_PRINT, 0.9)

    hw = Layer(w, h)
    f = font(LOG, 46)
    hw.hand(cols[1] + 20, top + 22, "11th Nov 1949. Alongside Harrowmark, Pier 5.", f, slope=0.3)
    assert quoted("Master ashore 23:20. Aboard 00:05.")
    entries = [
        ("19:00", "Customs aboard, No.2 hold. Insp. E. Morrow."),
        ("19:50", "Customs ashore."),
        ("23:00", "Shore crane on No.2 hold. Watch: A. Petrie."),
        ("23:20", "Master ashore."),
        ("00:05", "Master aboard."),
        ("00:30", "No.2 hold done. Gang to supper."),
        ("01:15", "Police on the quay. Ship held."),
        ("", "Wind SW 5, heavy rain."),
    ]
    for i, (t, r) in enumerate(entries):
        y = top + (i + 1) * row + 24
        if t:
            hw.hand(cols[0] + 20, y, t, f, slope=rng.normal(0, 0.4))
        hw.hand(cols[1] + 20, y, r, f, slope=rng.normal(0, 0.4))
    arr = apply_ink(arr, hw.soft(0.8), (24, 24, 30), 0.9)
    save(arr, "T_C10_GangwayBook_D.png")


def manifest() -> None:
    """C12 / A16: the manifest copy, cleared and signed by L. Varga, violet customs stamp."""
    w, h = 1536, 2048
    arr = paper(w, h, base=(236, 228, 206), age=0.4, stains=1)
    pr = Layer(w, h)
    pr.printed(w / 2, 110, "CARGO MANIFEST", font(PRINT_B, 64), "ma")
    pr.printed(w / 2, 200, "(COPY - PORT OF HARROWMARK)", font(PRINT, 32), "ma")
    labels = ["Vessel", "Master", "Port of loading", "Arrived"]
    for i, lab in enumerate(labels):
        y = 320 + i * 80
        pr.printed(120, y, lab + ":", font(PRINT, 32))
        pr.line([(460, y + 40), (1400, y + 40)], 2)
    cols = [120, 420, 620, 1100, 1416]
    for x, t in zip(cols, ["MARKS", "CASES", "DESCRIPTION", "WEIGHT"]):
        pr.printed(x + 14, 700, t, font(PRINT_B, 28))
    pr.line([(120, 680), (1416, 680)], 3)
    pr.line([(120, 750), (1416, 750)], 2)
    pr.line([(120, 1000), (1416, 1000)], 2)
    for x in cols:
        pr.line([(x, 680), (x, 1000)], 2)
    pr.printed(120, 1400, "Cleared by customs officer:", font(PRINT, 32))
    pr.line([(640, 1440), (1300, 1440)], 2)
    arr = apply_ink(arr, pr.soft(0.7), INK_PRINT, 0.9)

    ty = Layer(w, h)
    f = font(TYPE, 34)
    for i, v in enumerate(["SS Saint Agatha", "J. Kordes", "Antwerp", "11 Nov 1949"]):
        ty.typed(480, 316 + i * 80, v, f)
    ty.typed(cols[0] + 16, 780, "SA 1-140", f)
    ty.typed(cols[1] + 16, 780, "140", f)
    ty.typed(cols[2] + 16, 780, "MACHINE PARTS", f)
    ty.typed(cols[3] + 16, 780, "11,200 lb", f)
    arr = apply_ink(arr, ty.soft(0.6), INK_TYPE, 0.95)

    sig = Layer(w, h)
    sig.hand(700, 1370, "L. Varga", font(HAND, 54), slope=4, wobble=0.5)
    arr = apply_ink(arr, sig.soft(0.7), INK_BLUE, 0.95)
    assert "cleared by customs officer L. Varga" in CARD_TEXT

    st = Layer(520, 300)
    st.rect([10, 10, 510, 290], 8)
    st.rect([26, 26, 494, 274], 3)
    st.printed(260, 50, "PORT OF HARROWMARK", font(PRINT_B, 34), "ma")
    st.printed(260, 110, "CUSTOMS", font(PRINT_B, 58), "ma")
    st.printed(260, 190, "CLEARED  11 NOV 1949", font(PRINT_B, 32), "ma")
    stamp = distress(st.img.rotate(-7, resample=Image.BICUBIC, expand=True), 0.25)
    full = Image.new("L", (w, h), 0)
    full.paste(stamp, (820, 1520))
    arr = apply_ink(arr, full.filter(ImageFilter.GaussianBlur(0.9)), INK_VIOLET, 0.8)
    save(arr, "T_C12_Manifest_D.png")


def draft_report() -> None:
    """C15 / A19: Morrow's unsent typed report on the Agatha. Names nobody: the clearance is the question."""
    hold = quoted("Hold search requested by telephone, 20:10.")
    w, h = 1536, 2048
    arr = paper(w, h, base=(238, 231, 212), age=0.35, stains=1)
    ty = Layer(w, h)
    f = font(TYPE, 34)
    pitch = f.getlength("M")
    lines = [
        "PORT OF HARROWMARK - CUSTOMS SERVICE",
        "REPORT OF INSPECTION                  DRAFT",
        "",
        "Vessel:    SS Saint Agatha",
        "Master:    J. Kordes",
        "Date:      11 November 1949",
        "Inspector: E. Morrow",
        "",
        "At 19:00 I examined No. 2 hold. The cargo",
        "is declared on the manifest as machine",
        "parts, 140 cases. Of the cases I opened,",
        "every one held cigarettes or whisky.",
        "",
        "The manifest was cleared at this office",
        "before the vessel docked. I cannot account",
        "for that clearance.",
        "",
        hold,
        "Recommend the vessel be held pending",
        "search.",
    ]
    x0, y0, lead = 140, 160, 64
    for i, ln in enumerate(lines):
        ty.typed(x0, y0 + i * lead, ln, f)
    arr = apply_ink(arr, ty.soft(0.6), INK_TYPE, 0.95)
    # a pencil question mark in the margin next to the clearance, by the same hand as the notebook
    pen = Layer(w, h)
    pen.hand(60, y0 + 14 * lead - 6, "?", font(HAND, 60), slope=8)
    arr = apply_ink(arr, pen.soft(0.7), INK_PENCIL, 0.8)
    _ = pitch
    save(arr, "T_C15_DraftReport_D.png")


def roster() -> None:
    """C14 / A18: the duty roster, chalk on a slate board."""
    w, h = 2048, 1024
    base = np.ones((h, w, 3), np.float32) * np.array([30, 36, 34], np.float32)
    base *= (1 + 0.18 * (noise(w, h, 128) - 0.5) + 0.05 * (noise(w, h, 8) - 0.5))[..., None]
    # old wiped chalk
    ghost = Layer(w, h)
    for _ in range(6):
        ghost.hand(rng.uniform(100, 1500), rng.uniform(100, 900), "MORROW  CL-2", font(LOG, 84), slope=rng.normal(0, 2))
    base = apply_ink(base, ghost.soft(14), (200, 205, 200), 0.10)
    ch = Layer(w, h)
    big, mid = font(LOG, 92), font(LOG, 76)
    ch.hand(120, 70, "NIGHT WATCH - FRI 11 NOV", big, slope=-0.6)
    ch.line([(120, 200), (1900, 196)], width=5)
    ch.hand(140, 290, "CL-2", big, slope=0.2)
    ch.hand(520, 290, "MORROW / VARGA", big, slope=-0.3)
    ch.hand(1380, 290, "22:00 - 02:00", big, slope=0.1)
    ch.hand(140, 470, "CL-1", big, slope=0.2)
    ch.hand(520, 470, "LAID UP - ENGINE", mid, slope=0.4)
    ch.hand(140, 680, "Office: night clerk till 22:00", mid, slope=-0.4)
    assert "Morrow and Varga on CL-2, 22:00-02:00" in CARD_TEXT
    chalk = distress(ch.soft(0.8), 0.18, cell=3)
    base = apply_ink(base, chalk.filter(ImageFilter.GaussianBlur(0.6)), (226, 228, 220), 0.92)
    save(base, "T_C14_DutyRoster_D.png")


def cash_tape() -> None:
    """C09 / A13: the Lighthouse register tape, faint violet print. Six rings by clerk N from 23:30 to 23:50."""
    w, h = 512, 2048
    arr = paper(w, h, base=(240, 236, 222), age=0.3, stains=0)
    pr = Layer(w, h)
    f = font(PRINT, 30)
    pr.printed(w / 2, 60, "THE LIGHTHOUSE", font(PRINT_B, 36), "ma")
    pr.printed(w / 2, 110, "HARROWMARK", f, "ma")
    pr.printed(w / 2, 160, "NOV 11 49", f, "ma")
    quoted("six rings between 23:30 and 23:50, clerk N.")
    rows = [
        ("22:58", "1 RYE", ".35"), ("23:17", "1 RYE", ".35"),
        ("23:31", "2 BEER", ".30"), ("23:36", "1 RYE", ".35"), ("23:40", "2 GIN", ".50"),
        ("23:44", "1 BEER", ".15"), ("23:47", "3 BEER", ".45"), ("23:50", "1 GIN", ".25"),
        ("00:12", "2 BEER", ".30"), ("00:31", "1 DBL RYE", ".70"), ("00:47", "2 RYE", ".70"),
        ("01:20", "1 COFFEE", ".10"),
    ]
    y = 240
    for t, item, price in rows:
        pr.printed(40, y, t, f)
        pr.printed(150, y, item, f)
        pr.printed(390, y, price, f)
        pr.printed(470, y, "N", f)
        y += 140
    pr.printed(w / 2, y + 10, "THANK YOU", f, "ma")
    mask = distress(pr.img, 0.12, cell=2)
    arr = apply_ink(arr, mask.filter(ImageFilter.GaussianBlur(0.7)), INK_VIOLET, 0.75)
    save(arr, "T_C09_CashTape_D.png")


def cl2_stamp() -> None:
    """C03 / D04: the "C.L. 2" stencil on the boathook grip, a grayscale decal mask (white = paint). In the world it
    sits under the tar decal: at the scene the pole looks unmarked, only the darkroom enlargement reads it."""
    w, h = 512, 256
    m = Layer(w, h)
    m.printed(w / 2, h / 2, quoted("C.L. 2"), font(PRINT_B, 120), "mm")
    mask = distress(m.soft(1.2), 0.1, cell=3).filter(ImageFilter.GaussianBlur(1.0))
    OUT.mkdir(parents=True, exist_ok=True)
    p = OUT / "T_C03_HookStencil_M.png"
    mask.save(p, optimize=True)
    print("wrote", p)


def movements_board() -> None:
    """C20 / A20: the harbour movements board at the gate booth, chalk on slate. Wiped at the 06:00 shift change."""
    w, h = 2048, 1280
    base = np.ones((h, w, 3), np.float32) * np.array([28, 38, 34], np.float32)
    base *= (1 + 0.2 * (noise(w, h, 128) - 0.5) + 0.06 * (noise(w, h, 8) - 0.5))[..., None]
    ghost = Layer(w, h)  # yesterday's movements, wiped with a damp rag
    for _ in range(7):
        ghost.hand(rng.uniform(80, 1500), rng.uniform(220, 1150), "PILOT  out 14:10", font(LOG, 80), slope=rng.normal(0, 2))
    base = apply_ink(base, ghost.soft(16), (200, 205, 200), 0.10)
    frame = Image.new("L", (w, h), 0)
    ImageDraw.Draw(frame).rectangle([0, 0, w - 1, h - 1], outline=255, width=46)
    base = apply_ink(base, frame.filter(ImageFilter.GaussianBlur(3)), (74, 58, 42), 0.95)

    ch = Layer(w, h)
    big, mid = font(LOG, 96), font(LOG, 80)
    ch.hand(130, 90, "HARBOUR MOVEMENTS", big, slope=-0.5)
    ch.hand(1250, 110, "Fri 11 Nov", mid, slope=0.3)
    ch.line([(120, 230), (1920, 226)], width=5)
    cols = [130, 640, 1160, 1560]
    for x, t in zip(cols, ["CRAFT", "STATION", "OUT", "IN"]):
        ch.hand(x, 270, t, font(LOG, 64), slope=0.2)
    ch.line([(120, 360), (1920, 362)], width=3)
    assert quoted("On station 22:00") and quoted("Out 23:53") and quoted("In 00:20")
    assert quoted("CL-2 out 16:20, in 16:45, hook lost P5")
    rows = [
        ("CL-2", "hook lost P5", "16:20", "16:45"),
        ("Tug MARGARET", "Pier 2  18:00", "21:40", "22:35"),
        ("PILOT", "Signal stn", "22:50", "00:05"),
        ("CL-2", "On station 22:00", "Out 23:53", "In 00:20"),
        ("CL-1", "laid up", "-", "-"),
    ]
    for i, row in enumerate(rows):
        y = 400 + i * 160
        for x, t in zip(cols, row):
            ch.hand(x, y, t, big if t == "CL-2" else mid, slope=rng.normal(0, 0.7))
    chalk = distress(ch.soft(0.8), 0.18, cell=3)
    base = apply_ink(base, chalk.filter(ImageFilter.GaussianBlur(0.6)), (226, 228, 220), 0.92)
    save(base, "T_C20_MovementsBoard_D.png")


def tally_sheet() -> None:
    """C21 / A21: the Agatha's discharge tally, kept on deck by the tallyman P. Doyle; the crane driver on every line."""
    w, h = 1536, 2048
    arr = paper(w, h, base=(234, 226, 204), age=0.5, stains=2)
    pr = Layer(w, h)
    pr.printed(w / 2, 80, "SS SAINT AGATHA", font(PRINT_B, 46), "ma")
    pr.printed(w / 2, 140, "DISCHARGE TALLY  -  No. 2 HOLD", font(PRINT_B, 34), "ma")
    pr.printed(120, 220, "Date:", font(PRINT, 30))
    pr.printed(820, 220, "Tallyman:", font(PRINT, 30))
    cols = [120, 400, 760, 1120, 1416]
    for x, t in zip(cols, ["TIME", "CASES", "MARKS", "CRANE"]):
        pr.printed(x + 16, 322, t, font(PRINT_B, 30))
    top, row, rows = 370, 76, 19
    pr.line([(cols[0], 310), (cols[-1], 310)], 3)
    for i in range(rows + 1):
        pr.line([(cols[0], top + i * row), (cols[-1], top + i * row)], 2)
    for x in cols:
        pr.line([(x, 310), (x, top + rows * row)], 2)
    arr = apply_ink(arr, pr.soft(0.7), INK_PRINT, 0.9)

    tm = Layer(w, h)  # the tallyman's pencil
    f = font(LOG, 42)
    tm.hand(230, 212, "11/12 Nov", f, slope=0.4, wobble=1)
    tm.hand(1010, 212, "P. Doyle", f, slope=0.2, wobble=1)
    for i in range(rows):
        t = 23 * 60 + i * 5
        y = top + i * row + 16
        tm.hand(cols[0] + 22, y, f"{(t // 60) % 24:02d}:{t % 60:02d}", f, slope=rng.normal(0, 0.6), wobble=1)
        first = sum(CASES[:i]) + 1
        tm.hand(cols[1] + 22, y, str(CASES[i]), f, slope=rng.normal(0, 0.6), wobble=1)
        tm.hand(cols[2] + 22, y, f"SA {first}-{first + CASES[i] - 1}", f, slope=rng.normal(0, 0.6), wobble=1)
    quoted("slung 140, landed 136.")
    foot = top + rows * row + 30
    tm.hand(cols[0] + 22, foot, "Slung 140.", font(LOG, 52), slope=0.3, wobble=1)
    tm.hand(cols[0] + 22, foot + 66, "Landed 136.", font(LOG, 52), slope=0.5, wobble=1)
    tm.hand(cols[2] + 22, foot + 66, "4 short - P.D.", font(LOG, 46), slope=-0.8, wobble=1)
    arr = apply_ink(arr, tm.soft(0.6), INK_PENCIL, 0.85)

    op = Layer(w, h)  # Doyle notes the crane driver on every line, in a blunt indelible pencil
    assert quoted("tallyman P. Doyle") and quoted("hoists 23:00 to 00:30") and quoted("crane S.H. on every line")
    for i in range(rows):
        op.hand(cols[3] + 40 + rng.normal(0, 6), top + i * row + 14, "S.H.", font(LOG, 46), slope=rng.normal(-3, 2), wobble=1)
    arr = apply_ink(arr, op.soft(0.7), INK_VIOLET, 0.8)
    save(arr, "T_C21_TallySheet_D.png")


def clearance() -> None:
    """C22 / A22: the Agatha's clearance certificate: master seen 23:30, cleared 23:50, hold search waived;
    harbour master's round stamp 23:50, signed R. Ashby, countersigned by Kordes and, for customs, by Varga."""
    w, h = 1536, 2048
    arr = paper(w, h, base=(238, 232, 214), age=0.35, stains=1)
    pr = Layer(w, h)
    pr.rect([70, 70, w - 70, h - 70], 4)
    pr.rect([86, 86, w - 86, h - 86], 2)
    pr.printed(w / 2, 170, "PORT OF HARROWMARK - CLEARANCE", font(PRINT_B, 50), "ma")
    pr.printed(w / 2, 240, "CERTIFICATE OF CLEARANCE OUTWARDS", font(PRINT, 34), "ma")
    labels = ["Vessel", "Master", "Tonnage", "Bound for", "Cargo"]
    for i, lab in enumerate(labels):
        y = 400 + i * 100
        pr.printed(160, y, lab + ":", font(PRINT, 34))
        pr.line([(480, y + 44), (1380, y + 44)], 2)
    body = [
        "The above vessel, having complied with the",
        "regulations of this Port, is hereby cleared",
        "to proceed to sea.",
    ]
    for i, ln in enumerate(body):
        pr.printed(160, 1000 + i * 54, ln, font(PRINT, 32))
    pr.printed(160, 1180, "Remarks:", font(PRINT, 32))
    pr.line([(400, 1220), (1380, 1220)], 2)
    pr.printed(160, 1300, "Harbour Master:", font(PRINT, 32))
    pr.line([(520, 1340), (1100, 1340)], 2)
    pr.printed(160, 1460, "Master:", font(PRINT, 32))
    pr.line([(520, 1500), (1100, 1500)], 2)
    pr.printed(160, 1620, "For Customs:", font(PRINT, 32))
    pr.line([(520, 1660), (1100, 1660)], 2)
    arr = apply_ink(arr, pr.soft(0.7), INK_PRINT, 0.9)

    ty = Layer(w, h)
    f = font(TYPE, 36)
    for i, v in enumerate(["SS Saint Agatha", "J. Kordes", "3,410 GRT", "Antwerp", "Machine parts, 140 cases"]):
        ty.typed(500, 396 + i * 100, v, f)
    ty.typed(410, 1176, quoted("Master seen 23:30. Cleared 23:50. Hold search waived."), font(TYPE, 26))
    arr = apply_ink(arr, ty.soft(0.6), INK_TYPE, 0.95)

    sig = Layer(w, h)
    quoted("Signed R. Ashby")
    sig.hand(560, 1270, "R. Ashby", font(LOG, 64), slope=2, wobble=0.6)
    assert quoted("J. Kordes")
    sig.hand(560, 1430, "J. Kordes", font(LOG, 70), slope=5, wobble=0.6)
    arr = apply_ink(arr, sig.soft(0.7), INK_BLUE, 0.95)
    assert quoted("for customs, L. Varga")
    vs = Layer(w, h)  # Varga signs in the same hand as his name in the notebook smudge
    vs.hand(560, 1585, "L. Varga", font(HAND, 60), slope=-1.5)
    arr = apply_ink(arr, vs.soft(0.7), INK_BLUE, 0.9)

    # round stamp in blue-black ink, the time set by hand in the centre
    s = 460
    st = Layer(s, s)
    d = ImageDraw.Draw(st.img)
    d.ellipse([8, 8, s - 8, s - 8], outline=235, width=10)
    d.ellipse([56, 56, s - 56, s - 56], outline=235, width=4)
    rim = font(PRINT_B, 30)
    text = "HARBOUR MASTER * PORT OF HARROWMARK * "
    for i, ch in enumerate(text):
        a = -math.pi / 2 + i * 2 * math.pi / len(text)
        r = s / 2 - 32
        glyph = Image.new("L", (40, 40), 0)
        ImageDraw.Draw(glyph).text((20, 20), ch, font=rim, fill=235, anchor="mm")
        glyph = glyph.rotate(-math.degrees(a) - 90, resample=Image.BICUBIC)
        st.paste(glyph, int(s / 2 + r * math.cos(a) - 20), int(s / 2 + r * math.sin(a) - 20))
    st.printed(s / 2, s / 2 - 70, "11 NOV 1949", font(PRINT_B, 36), "ma")
    st.printed(s / 2, s / 2 - 20, "23:50", font(PRINT_B, 84), "ma")
    stamp = distress(st.img.rotate(11, resample=Image.BICUBIC, expand=True), 0.22)
    full = Image.new("L", (w, h), 0)
    full.paste(stamp, (960, 1300))
    arr = apply_ink(arr, full.filter(ImageFilter.GaussianBlur(0.9)), (26, 34, 64), 0.82)
    save(arr, "T_C22_Clearance_D.png")


def store_book() -> None:
    """C23 / A26: the customs store book on the front counter. Varga's two entries for Friday, initialled L.V."""
    w, h = 2048, 1536
    arr = paper(w, h, age=0.5, stains=2)
    pr = Layer(w, h)
    cols = [90, 380, 1280, 1700, 1960]
    for x, t in zip(cols, ["DATE / TIME", "ITEM AND REMARKS", "RETURNED", "INIT."]):
        pr.printed(x + 14, 120, t, font(PRINT_B, 30))
    top, row = 180, 96
    for i in range(14):
        pr.line([(cols[0], top + i * row), (cols[-1], top + i * row)], 2)
    for x in cols:
        pr.line([(x, 100), (x, top + 13 * row)], 2)
    pr.line([(cols[0], 100), (cols[-1], 100)], 3)
    pr.printed(w / 2, 40, "H.M. CUSTOMS - PORT OF HARROWMARK - STORES", font(PRINT, 30), "ma")
    arr = apply_ink(arr, pr.soft(0.7), INK_PRINT, 0.9)

    assert quoted("CL-2 boathook lost overboard at Pier 5, 16:30 Fri. L.V.")
    assert quoted("Clearance stamp out to the harbour master 23:28, L.V. Back 23:52.")
    f = font(LOG, 44)
    old = Layer(w, h)  # earlier entries in other hands
    for i, (d, t, r, n) in enumerate([("Wed 9th", "Lamp oil, 2 gal., CL-1", "-", "E.M."),
                                      ("Thu 10th", "Signal flares (3) to CL-2", "-", "E.M.")]):
        y = top + i * row + 24
        for x, s in zip(cols, (d, t, r, n)):
            old.hand(x + 20, y, s, f, slope=rng.normal(0, 0.4), wobble=1.0)
    arr = apply_ink(arr, old.soft(0.8), INK_BLUE, 0.75)
    hw = Layer(w, h)
    entries = [
        ("Fri 16:30", "CL-2 boathook lost overboard at Pier 5", "", "L.V."),
        ("Fri 23:28", "Clearance stamp out to the harbour master", "23:52", "L.V."),
    ]
    for i, e in enumerate(entries):
        y = top + (i + 2) * row + 24
        for x, s in zip(cols, e):
            if s:
                hw.hand(x + 20, y, s, f, slope=rng.normal(0, 0.5), wobble=1.2)
    arr = apply_ink(arr, hw.soft(0.8), INK_BLUE, 0.92)
    save(arr, "T_C24_StoreBook_D.png")


def enlargement(arr: np.ndarray, name: str) -> None:
    """A darkroom enlargement under the loupe: black and white, coarse grain, soft at the edges, vignetted."""
    h, w, _ = arr.shape
    g = arr @ np.array([0.30, 0.59, 0.11], np.float32)
    img = Image.fromarray(np.clip(g, 0, 255).astype(np.uint8))
    sharp = np.asarray(img.filter(ImageFilter.GaussianBlur(1.2)), np.float32)
    soft = np.asarray(img.filter(ImageFilter.GaussianBlur(9)), np.float32)
    yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
    d = np.sqrt(((xx - w / 2) / (w / 2)) ** 2 + ((yy - h / 2) / (h / 2)) ** 2)
    k = np.clip((d - 0.45) / 0.6, 0, 1)
    g = sharp * (1 - k) + soft * k
    g = (g / 255) ** 1.25 * 255  # grade 3 paper: a little more contrast
    g = 128 + (g - 128) * 1.2
    grain = rng.normal(0, 1, (h, w)).astype(np.float32) * 14
    clump_small = (rng.random((h // 3, w // 3)) * 255).astype(np.uint8)
    clump = np.asarray(Image.fromarray(clump_small).resize((w, h), Image.BICUBIC), np.float32)
    g = g + grain + (clump - 128) * 0.12
    g *= 1 - 0.55 * np.clip(d - 0.55, 0, 1.2) ** 1.6
    tone = np.stack([g * 1.0, g * 0.985, g * 0.95], -1)  # fibre paper, a hair warm
    save(tone, name)


def detail_margin() -> None:
    """D02: the crane log's margin note, enlarged. Same pencil and hand as T_C07."""
    w = h = 1024
    arr = paper(w, h, age=0.4, stains=0)
    pr = Layer(w, h)  # the right edge of the table and two row lines, as on T_C07
    pr.line([(150, 0), (150, h)], 4)
    pr.line([(0, 330), (150, 330)], 4)
    pr.line([(0, 560), (150, 560)], 4)
    arr = apply_ink(arr, pr.soft(1.4), INK_PRINT, 0.9)
    hw = Layer(w, h)
    hw.hand(-60, 410, "S.H.", font(LOG, 110), wobble=1)
    hw.hand(240, 130, "slewed short", font(LOG, 110), slope=1.5, wobble=1.2)
    hw.hand(240, 310, "23:00-23:50 -", font(LOG, 110), slope=1.0, wobble=1.2)
    hw.hand(240, 490, "CL-2 under", font(LOG, 110), slope=0.6, wobble=1.2)
    hw.hand(240, 670, "the swing.", font(LOG, 110), slope=0.2, wobble=1.2)
    assert MARGIN_NOTE == "slewed short 23:00-23:50 - CL-2 under the swing."
    arr = apply_ink(arr, distress(hw.soft(1.0), 0.08, cell=2), INK_PENCIL, 0.85)
    enlargement(arr, "T_D02_CraneLogMargin_D.png")


def detail_smudge() -> None:
    """D03: Morrow's notebook, the smudged name enlarged. The wet thumb took the top of the ink; the strokes stay."""
    w = h = 1024
    arr = paper(w, h, base=(226, 216, 190), age=0.8, stains=0)
    arr = stain(arr, 600, 520, 420, 0.8)
    arr = ruled(arr, 130, 230, 0, w, None, a=0.35)
    ink = Layer(w, h)
    ink.hand(60, 300, "Cleared by", font(HAND, 96), slope=-0.3)
    assert quoted("Cleared by L. Varga.")
    name = Layer(w, h)
    name.hand(160, 530, "L. Varga.", font(HAND, 120), slope=-0.5)
    arr = bleed(arr, ink, INK_BLUE, wet=1.0)
    arr = apply_ink(arr, name.soft(2.5), INK_BLUE, 0.55)  # the letters under the smear
    smear = Layer(w, h)
    for k in range(5):
        pts = [(140 + i * 22 + rng.normal(0, 6), 600 + 50 * math.sin(i * rng.uniform(0.5, 1.2) + k) + k * 14)
               for i in range(36)]
        smear.line(pts, width=int(rng.integers(10, 20)), fill=int(rng.integers(120, 200)))
    arr = apply_ink(arr, smear.soft(22), INK_BLUE, 0.35)
    arr = stain(arr, 520, 600, 330, 0.9)
    enlargement(arr, "T_D03_NotebookSmudge_D.png")


def bar_slate() -> None:
    """C23 / A12: the slate behind the Lighthouse bar. NO CREDIT on top; Morrow's tab under it, never rubbed out."""
    w, h = 1536, 1024
    base = np.ones((h, w, 3), np.float32) * np.array([30, 34, 33], np.float32)
    base *= (1 + 0.18 * (noise(w, h, 128) - 0.5) + 0.05 * (noise(w, h, 8) - 0.5))[..., None]
    ghost = Layer(w, h)  # old tabs, wiped with a bar rag
    for _ in range(8):
        ghost.hand(rng.uniform(80, 1100), rng.uniform(300, 950), "J.K. .70", font(LOG, 72), slope=rng.normal(0, 3))
    base = apply_ink(base, ghost.soft(14), (200, 205, 200), 0.10)
    frame = Image.new("L", (w, h), 0)
    ImageDraw.Draw(frame).rectangle([0, 0, w - 1, h - 1], outline=255, width=40)
    base = apply_ink(base, frame.filter(ImageFilter.GaussianBlur(3)), (64, 50, 38), 0.95)
    ch = Layer(w, h)
    ch.hand(160, 90, "NO CREDIT.", font(LOG, 150), slope=-1.2)
    ch.line([(160, 290), (1300, 282)], width=6)
    ch.hand(160, 360, "Rye .35   Gin .25   Beer .15", font(LOG, 70), slope=0.3)
    ch.hand(160, 480, "Double .70   Coffee .10", font(LOG, 70), slope=-0.2)
    ch.hand(1000, 760, quoted("E.M. $40"), font(LOG, 64), slope=4)  # small, low in the corner, Nora's hand
    chalk = distress(ch.soft(0.8), 0.18, cell=3)
    base = apply_ink(base, chalk.filter(ImageFilter.GaussianBlur(0.6)), (226, 228, 220), 0.92)
    save(base, "T_C23_BarSlate_D.png")


def detail_stencil() -> None:
    """D04: the boathook grip enlarged. Ash under old tar; where hands wore the tar thin, stencil paint: C.L. 2."""
    w = h = 1024
    yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
    grain = np.sin(yy / 5 + 6 * noise(w, h, 96) + xx / 400) * 0.5 + 0.5  # ash grain runs along the shaft
    wood = np.stack([112 + 26 * grain, 88 + 20 * grain, 62 + 14 * grain], -1).astype(np.float32)
    shaft = Image.new("L", (w, h), 0)
    ImageDraw.Draw(shaft).rectangle([0, 250, w, 774], fill=255)
    k = np.asarray(shaft.filter(ImageFilter.GaussianBlur(18)), np.float32)[..., None] / 255
    arr = np.ones((h, w, 3), np.float32) * 22 * (1 - k) + wood * k
    sten = Layer(w, h)
    sten.printed(w / 2, h / 2, quoted("C.L. 2"), font(PRINT_B, 190), "mm")
    arr = apply_ink(arr, distress(sten.soft(1.5), 0.2, cell=4), (232, 228, 214), 0.95)
    tar = noise(w, h, 160) * 0.75 + noise(w, h, 24) * 0.25  # broad patches; worn thin where hands held it
    cover = np.clip((tar - 0.5) * 3, 0, 1) * 0.85 + 0.12
    cover = cover * (np.asarray(shaft.filter(ImageFilter.GaussianBlur(10)), np.float32) / 255)
    arr = arr * (1 - cover[..., None] * 0.9) + np.array([14, 12, 10], np.float32) * cover[..., None] * 0.9
    enlargement(arr, "T_D04_HookStencil_D.png")


def contact_sheet() -> None:
    files = sorted(OUT.glob("T_C*_D.png")) + sorted(OUT.glob("T_D*_D.png"))
    thumbs = []
    for f in files:
        im = Image.open(f).convert("RGB")
        im.thumbnail((640, 640))
        thumbs.append(im)
    cols = 3
    sheet = Image.new("RGB", (cols * 660, math.ceil(len(thumbs) / cols) * 660), (11, 16, 24))
    for i, t in enumerate(thumbs):
        sheet.paste(t, ((i % cols) * 660 + 10, (i // cols) * 660 + 10))
    sheet.save(OUT / "_contact_sheet.png")
    print("wrote", OUT / "_contact_sheet.png")


if __name__ == "__main__":
    for fn in (notebook, crane_log, gate_book, ship_log, manifest, draft_report, roster, cash_tape, cl2_stamp,
               movements_board, tally_sheet, clearance, store_book, bar_slate, detail_margin, detail_smudge,
               detail_stencil):
        fn()
    contact_sheet()
