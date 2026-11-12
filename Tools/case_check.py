"""Check a case file for logical soundness, redundancy and solvability.

Usage: python case_check.py path/to/case01.json

Checks:
  1. Data: ids unique; places, shots, details, gates and references resolve.
  2. Truth: every anchor, detail and true statement agrees with the ground truth; every false statement
     with assertions contradicts it; anchors and details never contradict each other.
  3. Theory sweep: for every WHO and WITH WHAT the player can pin, the assertions of a false slot are
     refuted by at least two anchors or details, at least one of them still obtainable after night 1
     (or coming from the forced tutorial shot); the true slots are refuted by nothing. A pinned WHO only
     asserts the murder window once the player holds a time-of-death card, so an alibi counts as a late
     refuter only if some time-of-death card is late as well. A detail is as late as its source shot.
  4. Lies: every false statement with assertions has at least two refuters that do not sit behind the gate
     the lie itself opens, at least one obtainable after night 1 (mistakes need one).
  5. Monologue: the table ends with an unconditional line.
  6. Reference paths: each path is replayed with film, costs, gates, topic conditions, expiry, darkroom
     details, mid-night developing, timed shots, night windows, pre-exposed plates (they use up a plate of
     night 1 and arrive at the first developing) and suspects unlocked by a card; red-thread results,
     dawn voice-over, ending and rating must match.
Exit code 0 when everything passes, 1 otherwise.
"""
from __future__ import annotations

import itertools
import json
import sys
from pathlib import Path

FAILS: list[str] = []
NOTES: list[str] = []
HARD = ("anchor", "detail")


def fail(msg: str) -> None:
    FAILS.append(msg)


def minutes(hhmm: str) -> int:
    """Minutes on the case clock: 12:00 is 0, so the night runs forward through midnight."""
    h, m = map(int, hhmm.split(":"))
    if h < 12:
        h += 24
    return (h - 12) * 60 + m


class Case:
    def __init__(self, data: dict):
        self.d = data
        self.places: dict[str, str | None] = data["places"]
        self.cards = {c["id"]: c for c in data["cards"]}
        self.shots = {s["id"]: s for s in data["shots"]}
        self.costs = data["costs"]
        self.source: dict[str, str] = {}  # card id -> shot id that yields it (anchors) or it comes from (details)
        for s in data["shots"]:
            for a in s["yields"]:
                self.source[a] = s["id"]
        for c in data["cards"]:
            if c["kind"] == "detail":
                self.source[c["id"]] = c["from"]

    # ---- places and assertions ----
    def ancestors(self, p: str) -> list[str]:
        out = []
        while p is not None:
            out.append(p)
            p = self.places[p]
        return out

    def compatible(self, a: str, b: str) -> bool:
        return a in self.ancestors(b) or b in self.ancestors(a)

    @staticmethod
    def overlap(f1: str, t1: str, f2: str, t2: str) -> bool:
        return minutes(f1) <= minutes(t2) and minutes(f2) <= minutes(t1)

    def at_conflict(self, x: list, y: list) -> bool:
        return x[0] == y[0] and self.overlap(x[2], x[3], y[2], y[3]) and not self.compatible(x[1], y[1])

    def conflicts(self, a: dict, b: dict) -> list[str]:
        """Reasons why two assertion sets cannot both be true."""
        why = []
        for x in a.get("at", []):
            for y in b.get("at", []):
                if self.at_conflict(x, y):
                    why.append(f"{x[0]}: {x[1]} {x[2]}-{x[3]} vs {y[1]} {y[2]}-{y[3]}")
        for k, v in a.get("prop", {}).items():
            if k in b.get("prop", {}) and b["prop"][k] != v:
                why.append(f"{k}: {v} vs {b['prop'][k]}")
        return why

    # ---- theory slots ----
    def theory(self, slot: str, value: str, timed: bool = True) -> dict:
        """Assertions a pinned WHO or WITH WHAT makes. WHO asserts the murder window only when timed."""
        if slot == "WHO":
            t = self.d["theory_who"]
            th = {"id": f"WHO={value}", "kind": "theory", "slot": "WHO",
                  "prop": {t["prop"]: self.d["people"][value]["height"]}}
            if timed:
                th["at"] = [[value, t["place"], t["from"], t["to"]]]
            return th
        w = self.d["weapons"][value]
        return {"id": f"WITH={value}", "kind": "theory", "slot": "WITH", "prop": dict(w.get("prop", {})),
                "at": [list(x) for x in w.get("at", [])]}

    def red(self, a: dict, b: dict) -> str:
        kinds = {a["kind"], b["kind"]}
        if kinds & set(HARD) and "theory" in kinds:
            return "break" if self.conflicts(a, b) else "none"
        if kinds & set(HARD) and "statement" in kinds:
            if not self.conflicts(a, b):
                return "none"
            stmt = a if a["kind"] == "statement" else b
            return "wrong" if stmt.get("false_kind") == "mistake" else "lie"
        if kinds == {"statement", "theory"}:
            return "word"
        return "none"

    # ---- obtainability ----
    def gate_of(self, cid: str) -> str | None:
        shot = self.shots.get(self.source.get(cid, ""))
        return shot.get("requires", {}).get("gate") if shot else None

    def late(self, cid: str) -> bool:
        """Obtainable after night 1, or guaranteed by the forced tutorial shot."""
        shot = self.shots[self.source[cid]]
        if shot.get("forced") or shot.get("pre_exposed"):
            return True
        if shot.get("window") and shot["window"]["night"] == 1:
            return False
        return shot.get("expires") is None  # a detail is as late as the shot it is enlarged from

    def hard(self) -> list[dict]:
        return [x for x in self.d["cards"] if x["kind"] in HARD]


def check_data(c: Case) -> None:
    ids = [x["id"] for x in c.d["cards"]] + list(c.shots)
    for dup in {i for i in ids if ids.count(i) > 1}:
        fail(f"data: duplicate id {dup}")
    for p, parent in c.places.items():
        if parent is not None and parent not in c.places:
            fail(f"data: place {p} has unknown parent {parent}")
    if "hub" in c.d:
        if c.d["hub"] not in c.places:
            fail(f"data: hub {c.d['hub']} is not a place")
        if "drive" not in c.costs:
            fail("data: a case with a hub needs costs.drive")
    every_at = [x for card in c.d["cards"] for x in card.get("at", [])] + c.d["truth"]["at"]
    for x in every_at:
        if x[1] not in c.places:
            fail(f"data: unknown place {x[1]} in {x}")
    yielded = [a for s in c.d["shots"] for a in s["yields"]]
    for a in [x["id"] for x in c.d["cards"] if x["kind"] == "anchor"]:
        if yielded.count(a) != 1:
            fail(f"data: anchor {a} is yielded by {yielded.count(a)} shots (must be exactly 1)")
    for s in c.d["shots"]:
        if s["place"] not in c.places:
            fail(f"data: shot {s['id']} has unknown place {s['place']}")
        for a in s["yields"]:
            if c.cards.get(a, {}).get("kind") != "anchor":
                fail(f"data: shot {s['id']} yields {a}, which is not an anchor")
        if "detail" in s:
            d = c.cards.get(s["detail"])
            if not d or d["kind"] != "detail" or d["from"] != s["id"]:
                fail(f"data: shot {s['id']} detail {s['detail']} does not point back to it")
    for d in [x for x in c.d["cards"] if x["kind"] == "detail"]:
        if c.shots.get(d["from"], {}).get("detail") != d["id"]:
            fail(f"data: detail {d['id']} is not listed on shot {d['from']}")
    for s in c.d["shots"]:
        w = s.get("window")
        if w and not 1 <= w["night"] <= len(c.d["nights"]):
            fail(f"data: shot {s['id']} window night {w['night']} is outside the case")
    for t in c.d["theory_who"]["time_from"]:
        if c.cards.get(t, {}).get("kind") not in HARD:
            fail(f"data: time-of-death source {t} is not an anchor or detail")
    for s in c.d["shots"]:
        if s.get("pre_exposed") and (s.get("forced") or s.get("window") or s.get("requires") or s.get("expires")):
            fail(f"data: pre-exposed shot {s['id']} cannot be forced, timed, gated or expiring")
    for p, card in c.d.get("unlock", {}).items():
        if p not in c.d["suspects"]:
            fail(f"data: unlocked person {p} is not a suspect")
        if c.cards.get(card, {}).get("kind") not in HARD:
            fail(f"data: unlock card {card} of {p} is not an anchor or detail")
    proof = c.d["endings"]["proof"]
    for k in proof.get("holds_any", []):
        if k not in c.cards:
            fail(f"data: proof card {k} does not exist")
    forced = [s["id"] for s in c.d["shots"] if s.get("forced")]
    if len(forced) != 1:
        fail(f"data: expected exactly one forced tutorial shot, got {forced}")
    opened = {x.get("opens") for x in c.d["cards"]} - {None}
    for s in c.d["shots"]:
        g = s.get("requires", {}).get("gate")
        if g and g not in opened:
            fail(f"data: gate {g} of shot {s['id']} is opened by no statement")
    for w in c.d["weapons"]:
        if not c.cards.get(w, {}).get("object"):
            fail(f"data: weapon {w} is not an object card")
    for p in c.d["suspects"]:
        if "height" not in c.d["people"].get(p, {}):
            fail(f"data: suspect {p} has no height")
    for m in c.d["truth"]["motives"]:
        if c.cards.get(m, {}).get("motive_of") != c.d["truth"]["killer"]:
            fail(f"data: true motive {m} is not a motive card of the killer")


def check_truth(c: Case) -> None:
    truth = {"at": c.d["truth"]["at"], "prop": c.d["truth"]["prop"]}
    for card in c.d["cards"]:
        if not card.get("at") and not card.get("prop"):
            continue
        why = c.conflicts(card, truth)
        must_be_true = card["kind"] in HARD or card.get("true", False)
        if must_be_true and why:
            fail(f"truth: {card['id']} should be true but contradicts the truth: {why}")
        if not must_be_true and not why:
            fail(f"truth: {card['id']} is marked false but agrees with the truth")
    for a, b in itertools.combinations(c.hard(), 2):
        if c.conflicts(a, b):
            fail(f"truth: {a['id']} and {b['id']} contradict each other")
    t = c.d["truth"]
    for slot, value in (("WHO", t["killer"]), ("WITH", t["weapon"])):
        why = c.conflicts(c.theory(slot, value), truth)
        if why:
            fail(f"truth: the true theory {slot}={value} contradicts the truth: {why}")


def refuters(c: Case, item: dict, exclude_gate: str | None = None) -> list[str]:
    return [h["id"] for h in c.hard() if c.conflicts(item, h) and not (exclude_gate and c.gate_of(h["id"]) == exclude_gate)]


def check_theories(c: Case) -> list[tuple]:
    rows = []
    t = c.d["truth"]
    slots = [("WHO", p, p == t["killer"]) for p in c.d["suspects"]]
    slots += [("WITH", w, w == t["weapon"]) for w in c.d["weapons"]]
    time_late = any(c.late(t) for t in c.d["theory_who"]["time_from"])
    for slot, value, is_true in slots:
        th = c.theory(slot, value)
        refs = refuters(c, th)
        untimed = set(refuters(c, c.theory(slot, value, timed=False)))
        late = [r for r in refs if c.late(r) and (r in untimed or time_late)]
        rows.append((th["id"], is_true, refs))
        if is_true and refs:
            fail(f"theory: true slot {th['id']} is refuted by {refs}")
        if not is_true and len(refs) < 2:
            fail(f"theory: false slot {th['id']} has {len(refs)} refuter(s) {refs}, needs 2")
        if not is_true and not late:
            fail(f"theory: false slot {th['id']} can only be refuted on night 1 ({refs})")
    return rows


def check_lies(c: Case) -> None:
    for s in c.d["cards"]:
        if s["kind"] != "statement" or s.get("true", True):
            continue
        if not s.get("at") and not s.get("prop"):
            NOTES.append(f"lie {s['id']} has no assertions: flavour only, cannot be stamped")
            continue
        refs = refuters(c, s, exclude_gate=s.get("opens"))
        late = [r for r in refs if c.late(r)]
        need = 1 if s.get("false_kind") == "mistake" else 2
        if len(refs) < need:
            fail(f"lies: {s['id']} has {len(refs)} usable refuter(s) {refs}, needs {need}")
        if not late:
            fail(f"lies: {s['id']} can only be exposed on night 1 ({refs})")
        gated = [h["id"] for h in c.hard() if c.conflicts(s, h) and h["id"] not in refs]
        extra = f" (+ {gated} behind its own gate)" if gated else ""
        NOTES.append(f"{'mistake' if need == 1 else 'lie'} {s['id']} refuted by {refs}{extra}")


def check_monologue(c: Case) -> None:
    table = c.d["monologue"]
    if not table or table[-1]["when"]:
        fail("monologue: the last line must have an empty condition (fallback)")


def vo_line(c: Case, pinned: dict, refuted: set[str]) -> str:
    for row in c.d["monologue"]:
        w = row["when"]
        if "who" in w and pinned["who"] != w["who"]:
            continue
        if "with_in" in w and pinned["with"] not in w["with_in"]:
            continue
        if "lie_open" in w and w["lie_open"] in refuted:
            continue
        if any(pinned[k] is None for k in w.get("filled", [])):
            continue
        if any(pinned[k] is not None for k in w.get("empty", [])):
            continue
        if "empty_any" in w and all(pinned[k] is not None for k in w["empty_any"]):
            continue
        if "why_wrong" in w:
            owner = c.cards[pinned["why"]].get("motive_of") if pinned["why"] else None
            if (owner != pinned["who"]) != w["why_wrong"]:
                continue
        return row["id"]
    return "?"


def replay(c: Case, name: str, path: list[dict]) -> None:
    nights = c.d["nights"]
    night = 0
    clock = minutes(nights[0][0])
    pre = [s["id"] for s in c.d["shots"] if s.get("pre_exposed")]
    film = c.costs["film_per_night"] - len(pre)  # night 1 starts with the plates already exposed
    hand: set[str] = set()
    taken: set[str] = set()  # shots taken
    developable: set[str] = set()  # details whose shot has been developed at a dawn
    refuted: set[str] = set()
    gates: set[str] = set()
    met: set[str] = set()
    pinned: dict[str, str | None] = {"who": None, "with": None, "why": None}
    wrong_red = 0
    forced = next(s["id"] for s in c.d["shots"] if s.get("forced"))
    tag = f"path {name}"

    def spend(n: int, step: dict) -> None:
        nonlocal clock
        clock += n
        if clock > minutes(nights[night][1]):
            fail(f"{tag}: night {night + 1} runs out of time at step {step}")

    def requirements_met(req: dict, what: str, step: dict) -> bool:
        missing = [x for x in req.get("cards", []) if x not in hand]
        if missing:
            fail(f"{tag}: {what} needs cards {missing} at step {step}")
            return False
        if "gate" in req and req["gate"] not in gates:
            fail(f"{tag}: {what} is behind closed gate {req['gate']} at step {step}")
            return False
        if "refuted" in req and req["refuted"] not in refuted:
            fail(f"{tag}: {what} needs {req['refuted']} refuted at step {step}")
            return False
        return True

    def item(x: str) -> dict | None:
        if x in ("WHO", "WITH"):
            v = pinned["who" if x == "WHO" else "with"]
            if v is None:
                fail(f"{tag}: red thread to the empty {x} slot")
                return None
            timed = any(t in hand for t in c.d["theory_who"]["time_from"])
            return c.theory(x, v, timed=timed)
        if x not in hand:
            fail(f"{tag}: red thread uses {x}, which the player does not hold")
            return None
        return c.cards[x]

    def close(step: dict) -> None:
        proof = c.d["endings"]["proof"]
        proven = (pinned["who"] == proof["who"] and pinned["with"] == proof["with"]
                  and pinned["why"] in proof["why"]
                  and ("refuted" not in proof or proof["refuted"] in refuted)
                  and ("holds_any" not in proof or any(k in hand for k in proof["holds_any"])))
        attached = step.get("attach")
        if attached and attached not in hand:
            fail(f"{tag}: attaches {attached}, which the player does not hold")
        ending = "?"
        for e in c.d["endings"]["order"]:
            if "proven" in e and e["proven"] != proven:
                continue
            if "attached" in e and e["attached"] != attached:
                continue
            if "who" in e and e["who"] != pinned["who"]:
                continue
            ending = e["id"]
            break
        grade = "?"
        for r in c.d["rating"]:
            if "proven" in r and r["proven"] != proven:
                continue
            if "ending" in r and r["ending"] != ending:
                continue
            if night + 1 > r.get("max_night", 99) or wrong_red > r.get("max_wrong_red", 99):
                continue
            grade = r["grade"]
            break
        if ending != step.get("expect") or grade != step.get("rating"):
            fail(f"{tag}: ended {ending} rating {grade}, expected {step.get('expect')} {step.get('rating')}")
        NOTES.append(f"{tag}: closed on night {night + 1} ({clock - minutes(nights[night][0])} min in), "
                     f"{wrong_red} wrong red thread(s), proven={proven}: {ending} rating {grade}")

    def develop_pre_exposed() -> None:
        for s in pre:  # the plates exposed before night 1 come out of the first bath
            if s not in taken:
                taken.add(s)
                hand.update(c.shots[s]["yields"])

    # The agency office (the hub: darkroom, loupe, its own subjects) is in town, a drive away from the
    # scene. Night 1 starts at the scene (the prologue drive); later nights start at the office.
    hub = c.d.get("hub")
    at_hub = False

    def where_ok(place_is_hub: bool, what: str, step: dict) -> bool:
        if hub and place_is_hub != at_hub:
            fail(f"{tag}: {what} needs a drive {'to the office' if place_is_hub else 'to the scene'} first (step {step})")
            return False
        return True

    first_shot = True
    for step in path:
        do = step["do"]
        if do == "talk":
            card = c.cards[step["card"]]
            if card["id"] in hand:
                fail(f"{tag}: topic {card['id']} taken twice")
                continue
            if not requirements_met(card.get("requires", {}), card["id"], step):
                continue
            if not where_ok(False, f"talk {card['id']}", step):
                continue
            spend(card.get("cost", c.costs["topic"]), step)
            hand.add(card["id"])
            met.add(card["speaker"])
        elif do == "shoot":
            shot = c.shots[step["shot"]]
            if shot.get("pre_exposed"):
                fail(f"{tag}: {shot['id']} is pre-exposed and cannot be shot")
                continue
            if first_shot and shot["id"] != forced:
                fail(f"{tag}: the first photo must be the tutorial shot {forced}")
            first_shot = False
            if shot["id"] in taken:
                fail(f"{tag}: {shot['id']} photographed twice")
                continue
            if shot.get("expires") is not None and night + 1 > shot["expires"]:
                fail(f"{tag}: {shot['id']} is gone after night {shot['expires']} (step {step})")
                continue
            w = shot.get("window")
            if w and (night + 1 != w["night"] or not minutes(w["from"]) <= clock <= minutes(w["to"])):
                fail(f"{tag}: {shot['id']} only happens on night {w['night']} {w['from']}-{w['to']} (step {step})")
                continue
            if not requirements_met(shot.get("requires", {}), shot["id"], step):
                continue
            if not where_ok(hub in c.ancestors(shot["place"]), shot["id"], step):
                continue
            if film <= 0:
                fail(f"{tag}: out of film on night {night + 1} at {shot['id']}")
                continue
            film -= 1
            spend(c.costs["photo"], step)
            taken.add(shot["id"])
            hand.update(shot["yields"])
        elif do == "drive":
            if not hub:
                fail(f"{tag}: drive in a case without a hub")
                continue
            spend(c.costs["drive"], step)  # a fade and a short car shot
            at_hub = not at_hub
        elif do == "develop":
            if not where_ok(True, "develop", step):
                continue
            spend(c.costs["develop"], step)  # in the darkroom mid-night
            develop_pre_exposed()
            developable.update(c.shots[s]["detail"] for s in taken if "detail" in c.shots[s])
        elif do == "enlarge":
            d = step["detail"]
            if not where_ok(True, f"enlarge {d}", step):
                continue
            if d not in developable:
                fail(f"{tag}: enlarges {d} before its photo was developed")
                continue
            hand.add(d)
        elif do == "red":
            a, b = item(step["a"]), item(step["b"])
            if a is None or b is None:
                continue
            result = c.red(a, b)
            if result != step["expect"]:
                fail(f"{tag}: red {step['a']}-{step['b']} gave {result}, expected {step['expect']}")
            if result in ("lie", "wrong"):
                stmt = a if a["kind"] == "statement" else b
                refuted.add(stmt["id"])
                if "opens" in stmt:
                    gates.add(stmt["opens"])
            elif result == "break":
                th = a if a["kind"] == "theory" else b
                pinned["who" if th["slot"] == "WHO" else "with"] = None
            elif result == "none":
                wrong_red += 1
                spend(c.costs["wrong_red"], step)
        elif do == "pin":
            unlock = c.d.get("unlock", {})
            if "who" in step and step["who"] in unlock:
                if unlock[step["who"]] not in hand:
                    fail(f"{tag}: WHO={step['who']} pinned without {unlock[step['who']]}")
            elif "who" in step and step["who"] not in met:
                fail(f"{tag}: WHO={step['who']} pinned but the player never talked to them")
            if "who" in step and step["who"] not in c.d["suspects"]:
                fail(f"{tag}: WHO={step['who']} is not a suspect")
            for k in ("with", "why"):
                if k in step and step[k] not in hand:
                    fail(f"{tag}: {k.upper()}={step[k]} pinned but not held")
            if "with" in step and step["with"] not in c.d["weapons"]:
                fail(f"{tag}: WITH={step['with']} is not a weapon card")
            if "why" in step and "motive_of" not in c.cards[step["why"]]:
                fail(f"{tag}: WHY={step['why']} is not a motive card")
            for k in ("who", "with", "why"):
                if k in step:
                    pinned[k] = step[k]
        elif do == "end_night":
            used = clock - minutes(nights[night][0])
            budget = minutes(nights[night][1]) - minutes(nights[night][0])
            NOTES.append(f"{tag}: night {night + 1} used {used} of {budget} min, "
                         f"{c.costs['film_per_night'] - film} of {c.costs['film_per_night']} frames")
            if night + 1 >= len(nights):
                close(step)  # the ship sails: forced report with whatever is pinned
                break
            vo = vo_line(c, pinned, refuted)
            if "expect_vo" in step and vo != step["expect_vo"]:
                fail(f"{tag}: dawn {night + 1} voice-over {vo}, expected {step['expect_vo']}")
            NOTES.append(f"{tag}: dawn {night + 1} voice-over {vo}")
            develop_pre_exposed()
            developable.update(c.shots[s]["detail"] for s in taken if "detail" in c.shots[s])
            night += 1
            clock = minutes(nights[night][0])
            film = c.costs["film_per_night"]
            at_hub = bool(hub)  # dawn is in the darkroom; the next night starts at the office
        elif do == "close":
            close(step)
            break
        else:
            fail(f"{tag}: unknown step {step}")
    if forced not in taken:
        fail(f"{tag}: the tutorial shot {forced} was never taken")


def main() -> int:
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent / "data" / "case01.json"
    c = Case(json.loads(path.read_text(encoding="utf-8")))
    check_data(c)
    check_truth(c)
    rows = check_theories(c)
    check_lies(c)
    check_monologue(c)
    for name, steps in c.d["reference_paths"].items():
        replay(c, name, steps)

    n_hard = len(c.hard())
    print(f"case {c.d['id']}: {len(c.cards)} cards ({n_hard} anchors and details), {len(c.shots)} shots, "
          f"{len(c.d['suspects'])} suspects x {len(c.d['weapons'])} weapons\n")
    print("theory sweep (slot: refuted by; * = only on night 1)")
    for tid, is_true, refs in rows:
        mark = "TRUE " if is_true else "false"
        shown = ", ".join(r if c.late(r) else r + "*" for r in refs) if refs else "nothing (survives)"
        print(f"  {mark} {tid:<14} -> {shown}")
    print()
    for n in NOTES:
        print("  note:", n)
    print()
    if FAILS:
        print(f"FAIL ({len(FAILS)})")
        for f in FAILS:
            print("  -", f)
        return 1
    print("OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
