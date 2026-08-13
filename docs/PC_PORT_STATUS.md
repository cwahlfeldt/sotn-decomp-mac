# Native PC Build — Status and Roadmap to a Full Playthrough

*Last updated: 2026-07-06 (branch `pc-macos-build`)*

This fork builds Castlevania: Symphony of the Night as a **native macOS/PC
executable** (`./pc/sotn`) on top of the matching-decompilation source tree.
The PSX matching build is untouched: PC support is additive (separate
`src/pc/` runtime, `config/assets.us.yaml` asset extraction, CMake build), and
`./sotn.sh check us` still verifies the byte-perfect PSX binaries.

The goal is a complete start-to-finish playthrough — Castle Entrance through
the inverted castle to the final Shaft/Dracula fight — running natively.

---

## Where things stand

### Working today

- **Engine**: `main`, `DRA.BIN` (game engine), `RIC.BIN` (Richter), and the
  weapon overlays run natively. Alucard gameplay, menus, room transitions,
  item/relic systems work.
- **24 stage overlays wired and playable** (`src/pc/stages/stage_*.c`,
  launched with `./pc/sotn --stage <name>`):
  - *First castle — complete*: ST0 (prologue), NP3 (Castle Entrance), NO3
    (Entrance after visit), NZ0 (Alchemy Lab), NO0 (Marble Gallery), DAI
    (Royal Chapel), NO2 (Olrox's Quarters), ARE (Colosseum), LIB (Long
    Library), NO1 (Outer Wall), NZ1 (Clock Tower), TOP (Castle Keep), NO4
    (Underground Caverns), CAT (Catacombs), CHI (Abandoned Mine), CEN
    (Center Cube), WRP (Warp Rooms), SEL (title/name entry), DRE (nightmare).
  - *Reverse castle*: RWRP (Reverse Warp), RNZ0 (Necromancy Lab), RCAT
    (Floating Catacombs), RARE (Reverse Colosseum), RTOP (Reverse Keep).
- **One boss overlay wired**: BO4 (`src/pc/bosses/boss_bo4.c`).
- Familiars/servants (`TT_000`–`TT_004`) are partially wired
  (`src/pc/servant_pc.c`).

### The wiring recipe is solved

Adding a stage that already has decompiled C is a known, repeatable process
(asset offsets from the overlay header → `config/assets.us.yaml` →
`extract-assets`/`build-assets` → `src/pc/stages/stage_<st>.c` + dispatch +
CMake entry → dedup shared symbols). Known importer limitations (NULL layout
pointers, flip-reference sprites, raw gfx blobs) all have established
workarounds. So for any *fully decompiled* overlay, wiring is hours, not days.

---

## What blocks a start-to-finish playthrough

In rough dependency order:

### 1. Finish RNO3 (Reverse Castle Entrance) — *in progress, close*

RNO3 exists in the repo but was never fully decompiled upstream. As of
2026-07-06 **every function has structurally-complete C** (no INCLUDE_ASM
left in the overlay); what remains is byte-matching residue, mostly
register-allocation/scheduling jitter being ground down by parallel
decomp-permuter jobs in the build container:

| File | Matching | Residual |
| --- | --- | --- |
| `e_jack_o_bones.c` | 2 of 4 functions exact | bone projectile + main entity: ~10 diff lines |
| `e_blue_venus_weed.c` (Alura Une, ported from CHI) | SetupPrims + tendril exact | root/flower/dart/spike in permuter |
| `e_nova_skeleton.c` (new: GTE laser beam) | 2740 + 27E0 (GTE glow) exact | 4 functions in/queued for permuter |
| `unk_435F8.c` (two Orobourous serpents) | both piece functions exact | 7 functions in/queued for permuter |
| `unk_46940.c` | 7 of 8 exact | `EntityAlucardWaterEffect` permuter at 1950 (from 8055) |

Two tooling discoveries from this work unblock the rest: RNO3 is NO3's code
with runtime mirroring (`#if defined(STAGE_IS_RNO3)` variants in shared
headers), and a jump-table alignment gap in the build pipeline (fixed via
`//! JTBL_ALIGN=4` in `tools/builds/gen.py`) that likely explains why these
files were never matched upstream. Once RNO3 matches, wiring it into the PC
build is routine.

### 2. Decompile 9 missing reverse-castle stages — *largest work item*

These overlays have **no splat config and no C at all** in the repo; each
needs full upstream-style decompilation (splat config + symbol map + C)
before it can be wired:

> RCEN (Reverse Center Cube), RCHI (Cave), RDAI (Anti-Chapel), RLIB
> (Forbidden Library), RNO0 (Black Marble Gallery), RNO1 (Reverse Outer
> Wall), RNO2 (Death Wing's Lair), RNO4 (Reverse Caverns), RNZ1 (Reverse
> Clock Tower).

Mitigating factor: every reverse stage is substantially shared code with its
first-castle counterpart (the RNO3 experience shows the pattern — shared
entity headers plus mirror-variant `#if` blocks), and upstream's PSP overlays
often provide reference. But this is still the bulk of the remaining effort.

### 3. Boss overlays — *most fights currently can't happen*

| Status | Overlays |
| --- | --- |
| Wired on PC | BO4 |
| Fully decompiled, needs wiring only | MAR (Maria meeting), RBO0, RBO3 (Medusa), RBO5 |
| Partially decompiled | BO0 (~68 asm functions), BO6 (~124, contains the **Richter fight** — mandatory to reach the reverse castle) |
| Missing from repo entirely | BO1, BO2, BO3, BO5, BO7, RBO1, RBO2, RBO4, RBO6, RBO7, RBO8 (**includes the final Shaft/Dracula fight**) |

The two hard progression gates are **BO6** (Richter at the Castle Keep — the
fight that flips the castle) and **RBO8** (Shaft/Dracula — the ending).
Several other bosses guard relics/items needed for traversal or the good
ending, so in practice most of this table is required.

### 4. PC runtime gaps

Independent of overlays:

- **Sound**: SPU functions are stubs (`SpuGetAllKeysStatus` etc.) — no
  music/SFX playback path.
- **CD streaming**: `UpdateCd` not implemented (affects anything that waits
  on CD reads — cutscenes, some load sequencing).
- **Save/load**: untested end-to-end (memory card emulation path).
- **Familiars**: partially wired; need finishing and testing.

---

## Suggested order of attack

1. **Finish RNO3's ~23 remaining functions** (in progress; the mirror-variant
   pattern and permuter workflow are established) and wire it — gives the
   first playable reverse-castle entrance and validates the shared-header
   variant approach used by every other reverse stage.
2. **Wire the four finished bosses** (MAR, RBO0, RBO3, RBO5) — cheap wins
   that exercise the boss-wiring path beyond BO4.
3. **Finish BO6 (Richter fight)** — unblocks the castle flip, the single most
   important progression gate.
4. **Decompile + wire reverse stages one at a time**, leaning on first-castle
   counterparts (suggested: RNO0/RLIB/RDAI first — their counterparts are
   fully matched).
5. **Boss decompilation from scratch** (BO0 finish, then the missing BO/RBO
   overlays, ending with RBO8).
6. **Runtime**: sound backend, UpdateCd, save/load — can proceed in parallel
   with any of the above.

## Reference

- Build (PSX verify): `docker exec sotn-work bash -c 'export PATH=$PATH:/usr/local/go/bin && cd /sotn && make build'`
- Run a stage natively: `./pc/sotn --stage <name>` **from the repo root**
  (asset paths are relative); "state: 2, game step: 3" in the log means
  gameplay was reached.
- After editing `pc/CMakeLists.txt`, re-run `cmake .` in `pc/` — stale
  configuration silently skips new files.
- MAD (debug stage, 3 asm functions) is not needed for a playthrough.
