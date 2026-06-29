# How to Decompile SOTN (a practical guide)

This is a working guide to the decompilation method we used on this repo, written
from the actual session that decompiled ~18 functions in the `bo6` (boss Richter)
overlay. It explains **what to do, why it works, and the traps to avoid.**

---

## 0. The mental model: what "decompilation" means here

This project is a **matching decompilation**. The goal isn't "C code that behaves
like SOTN" — it's C code that, when compiled with the original 1997 PlayStation
compiler, produces the **byte-for-byte identical** machine code as the retail game.

Why that matters: if every function compiles to the exact original bytes, then the
reassembled game *is* the original game, proven by checksum. That's the whole game.

Consequences of "matching" that shape everything below:
- The build **already works and already matches today**, because functions that
  aren't decompiled yet are assembled from the *original extracted assembly* via
  `INCLUDE_ASM(...)` macros. Decompiling a function means replacing one
  `INCLUDE_ASM` with C that compiles to the same instructions. **The output binary
  never changes** — you're translating asm→C, not changing behavior.
- "Done" for a function = the overlay still checksum-matches the original. There's
  a hard pass/fail oracle (see §4). No "looks right" — it either matches or it
  doesn't.

So decompilation here is a long sequence of: pick a function still in asm → write
C for it → prove it produces the identical bytes → commit. Repeat ~thousands of times.

---

## 1. The environment (why Docker on a Mac)

The original PlayStation compiler, `bin/cc1-psx-26` (GCC 2.6.3 for MIPS), is a
**32-bit Linux x86 binary**. It cannot run natively on Apple Silicon — Rosetta only
translates 64-bit *macOS* binaries, not 32-bit Linux ELFs. So the build runs inside
a `linux/amd64` Docker container (`Dockerfile` + `build-mac.sh`), where the original
toolchain runs unchanged and Docker/Rosetta handle the x86 emulation.

For day-to-day decomp we keep one long-lived container alive so rebuilds are fast:

```bash
docker run -d --name sotn-work --platform linux/amd64 \
  -v "$(pwd):/sotn" -v sotn_venv:/sotn/.venv -v sotn_build:/sotn/build \
  -v sotn_gocache:/home/ubuntu/go --entrypoint /bin/bash sotn-build -lc 'sleep infinity'
```

Then every build/check/diff is a `docker exec sotn-work bash -lc '...'`. The repo is
mounted in, so you edit files on the Mac with your normal tools and only *build*
inside the container. (`.venv` and `build/` live in named volumes so the container's
Linux toolchain never mixes with anything native.)

---

## 2. The core loop

For one function:

1. **Pick a function** that's still `INCLUDE_ASM`. Start small.
2. **Find its "twin"** — an already-decompiled version of the same logic elsewhere
   (see §3). This is the single biggest time-saver.
3. **Read the target assembly** and map it to C using the twin as a template (§5).
4. **Write the C**, replacing the `INCLUDE_ASM(...)` line.
5. **Build + check** in the container (§4). `✅ us` = matched.
6. If it doesn't match, **diff it** with asm-differ to see exactly where, and fix
   (§6). If it's a scheduling/regalloc problem, use the permuter (§7).
7. **Commit** with a message saying what it is and that it's verified matching.

The commands you'll run constantly:

```bash
# inside the container (PATH set up once)
. .venv/bin/activate
make build -j"$(nproc)"     # recompiles changed files, relinks overlays
./sotn.sh check us          # the oracle: prints "✅ us" if everything matches
```

---

## 3. Find the twin (the dedup insight)

SOTN ships the same code many times. The playable Richter (`src/ric/`), the
Richter you fight as a boss (`src/boss/bo6/`), Maria, the reverse-castle variants —
they share huge amounts of near-identical logic. Almost every boss function is a
**twin** of a player function that's already been decompiled.

So before writing anything, search for the already-decompiled version:

```bash
# the function is BO6_RicSetStand → look for RicSetStand
grep -rn 'RicSetStand' src/ric/ --include='*.c' | grep -v INCLUDE_ASM
```

Real example from this session — `BO6_RicSetSpeedX` turned out to be a *verbatim*
copy of `RicSetSpeedX` in `src/ric/pl_utils.c`:

```c
void RicSetSpeedX(s32 speed) {
    if (g_CurrentEntity->facingLeft == 1)
        speed = -speed;
    g_CurrentEntity->velocityX = speed;
}
```

You're rarely writing logic from scratch. You're finding the twin, then remapping
its symbols to the boss overlay's (see §5). Shared helpers sometimes live in a
header that you just `#include` — e.g. `DecelerateX`/`DecelerateY` were already in
`src/decelerate.h`, so the whole "decompilation" was replacing two `INCLUDE_ASM`
lines with `#include "../../decelerate.h"`.

Also useful: `tools/m2c` turns MIPS asm into draft C automatically (good first pass
when there's no twin), and `tools/asm-differ` shows you how close you are.

---

## 4. The oracle: `./sotn.sh check us`

This is what makes decomp tractable. `sotn-assets check` rebuilds every overlay and
compares its checksum to the original file extracted from your disk. Output:

- `✅ us` → every imported file matches byte-for-byte. Your function is correct.
- `❌ BO6 ... checksum check failed` → the BO6 overlay no longer matches.

**Prove the oracle is real once.** Early on, deliberately break a function (swap two
field writes), build, and confirm you get `❌ BO6`. Then restore it and confirm
`✅ us`. This proves the check is a true byte-level oracle and that your rebuild is
actually picking up your edits — so a green check is trustworthy, not a false pass
from a stale object.

> Note: there is **no `make check`** target — that was a red herring in an old
> script. The real command is `./sotn.sh check us`.

---

## 5. Reading the assembly and mapping it to C

The asm lives in `asm/us/boss/bo6/nonmatchings/<file>/<FuncName>.s`. Each line shows
the address, raw bytes, and the instruction. You translate it to C, using the twin
as the shape and the asm as ground truth.

Example — `BO6_RicSetAnimation`:

```mips
lui   $v0, %hi(g_CurrentEntity)
lw    $v0, %lo(g_CurrentEntity)($v0)   # v0 = g_CurrentEntity
sw    $a0, 0x4C($v0)                   # entity->anim = arg (offset 0x4C)
sh    $zero, 0x52($v0)                 # entity->poseTimer = 0 (0x52)
jr    $ra
sh    $zero, 0x50($v0)                 # entity->pose = 0 (0x50) -- delay slot
```

Confirm the offsets against the struct in `include/game.h`
(`anim`@0x4C, `pose`@0x50, `poseTimer`@0x52), and you get:

```c
void BO6_RicSetAnimation(AnimationFrame* anim) {
    g_CurrentEntity->anim = anim;
    g_CurrentEntity->poseTimer = 0;
    g_CurrentEntity->pose = 0;
}
```

Things you constantly map:
- **Struct field offsets** — `lw 0x08($v0)` is `velocityX`; check `include/game.h`.
- **Globals** — `%hi/%lo(SYMBOL)` is a global; find its address/type in
  `config/symbols.us.bobo6.txt`. The boss overlay models Richter's state as a set of
  discrete `RIC_*` globals (`RIC_step`, `RIC_velocityX`, …) *and* a `PlayerState
  g_Ric` struct. Which one a function uses is dictated by the asm.
- **Constants** — `ori $a0, $zero, 0x18` is a literal. Match it to a named constant
  (`PL_S_SLIDE = 0x18` in `bo6.h`). `FIX(x)` is fixed-point: `FIX(5.5)` = `0x58000`.
- **`jal SYMBOL`** — a function call; just call it in C. It's fine to call functions
  that are still `INCLUDE_ASM` — the symbol resolves at link time.

### The boss-vs-player gotcha: constants differ

A twin is a *template*, not a copy. The boss often uses different IDs. `BO6_RicSetSlide`
is a twin of `RicSetSlide`, but the player plays `SFX_RIC_SLIDE_SKID` (0x707) while
the boss plays `SFX_BOSS_RIC_SLIDE_SKID` (0x826). **Always read the constant from the
asm**, don't blindly copy the twin's. The asm is ground truth; the twin is guidance.

---

## 6. When it compiles but doesn't match: asm-differ

If `check` fails, see exactly where:

```bash
python3 tools/asm-differ/diff.py --version=us --overlay=bo6 --format=plain BO6_RicSetFall
```

It prints TARGET vs CURRENT side by side. Markers: `i` = different immediate/offset,
`r` = different register, `>`/`<` = an instruction present in one side only.

Most "close but not matching" cases are one of:
- **A wrong constant or offset** → fix it (easy).
- **Reordered instructions, same set** → the C is logically right but the compiler
  scheduled it differently. Reorder your statements, or add a barrier (§7).
- **One extra/missing instruction** → usually register allocation; permuter (§7).

---

## 7. Scheduling/regalloc matches: the permuter (and `do { } while(0)`)

Sometimes the C is *correct* but GCC 2.6.3 emits the instructions in a different
order or uses one extra instruction. You can't always fix this by hand. Two tools:

**`do { } while (0)` barriers.** A statement boundary that stops GCC from reordering
across it. In `BO6_RicSetFall`, GCC kept hoisting a far-global store ahead of some
array stores; wrapping the array writes in `do { ... } while (0)` forced the original
order. This is an accepted matching device in this codebase (grep for `while (0);` —
several exist, some literally tagged `// FAKE`).

**The permuter.** `tools/decomp-permuter` randomly mutates your C (reordering,
inserting temps/barriers, swapping equivalent expressions) and recompiles thousands
of variants looking for one that matches:

```bash
tools/sotn_permuter/permuter_loader.py import us_39144 BO6_RicSetFall -o bobo6
tools/sotn_permuter/permuter_loader.py permute BO6_RicSetFall
# it writes output-<score>-<n>/ dirs; score 0 = perfect match
```

`BO6_RicSetFall` started at "score 475", the permuter found a 0-score solution, and
it confirmed the `do/while(0)` insight. Take the cleanest matching variant, not
necessarily the permuter's literal output.

---

## 8. The trap that bites everyone: overlays are all-or-nothing

`./sotn.sh check us` matches a whole *overlay file* (e.g. `BO6.BIN`), not individual
functions. The overlay's data symbols (like `g_Ric`) sit **after** all the code. So
if any function you decompile compiles to even **one instruction too long**, every
symbol after it shifts, and **the entire overlay stops matching** — every other
function suddenly "fails" too.

This actually happened with `BO6_RicCheckFacing`: it was logically correct but
compiled 4 bytes (one `lui`) too long, which shifted `g_Ric` by 4 and made *every*
`g_Ric.field` access in *every* function in the overlay look wrong in the diff. The
fix isn't "find the broken function" — it's "the function I just added isn't exactly
matching." We reverted it (to keep the branch green) and flagged it for a permuter pass.

Practical rules:
- **Commit only functions that produce `✅ us`.** Never commit a near-match.
- If adding one function suddenly makes the *whole* overlay fail, the new function is
  off by some bytes — diff *it*, don't hunt the others.
- Revert anything that needs a permuter pass rather than leaving the branch red.

---

## 9. Worked examples from this session (easy → hard)

- **Trivial / verbatim** — `BO6_RicSetAnimation`, `BO6_RicSetSpeedX`: copy the twin,
  confirm offsets, done.
- **Shared header** — `DecelerateX`/`DecelerateY`: `#include "../../decelerate.h"`.
- **New extern globals** — `BO6_RicSetStep`: the boss stores step/step_s as two
  separate globals (`RIC_step`, `D_80076306`), *not* a struct, because the asm emits
  two separate `%hi` loads. Modeling them as a struct would NOT match. The asm tells
  you the representation.
- **Constant hunting** — `BO6_RicSetSlide`/`SlideKick`: twins, but with boss-specific
  SFX/blueprint IDs you must read from the asm (`SFX_BOSS_*`, `BP_25`, `BP_31`).
- **Control flow + compiler folding** — `func_us_801B9DE4` (RicSetWalk): keeping the
  twin's seemingly-redundant `if` was necessary so GCC didn't dead-store-eliminate an
  intermediate write.
- **Permuter** — `BO6_RicSetFall`: correct logic, wrong schedule → `do/while(0)` +
  permuter.
- **Reverted** — `BO6_RicCheckFacing`: one instruction too long (regalloc), broke the
  overlay; deferred to a permuter pass.

---

## 10. Bonus: the native Mac build (no emulator)

Separately from the matching PSX build, the repo has a work-in-progress **native PC
port** (`src/pc/`, SDL2 + CMake) that compiles the decompiled C for Apple Silicon and
runs without an emulator:

```bash
brew install sdl2 cmake           # one time
cmake -B pc -DWANT_LIBSND_LLE=1
cmake --build pc -j
./pc/sotn --player ric --stage nz0 --disk "disks/.../Track 2.bin"
```

It's far from the whole game — only a handful of areas are wired up
(`nz0`, `cen`, `wrp`, `st0`, `sel`) — and each additional stage needs bespoke
integration. But it's the same decompiled code running natively. (We had to add three
missing `bo4` sprite-bank stubs in `src/pc/bosses/boss_bo4.c` to make it link.)

The relationship: **decompilation feeds the port.** The more functions and data move
from asm to C, the more of the game the native port *can* build — but wiring each
stage/system into the port is its own separate effort on top of the decomp.

---

## 11. Cheat sheet

```bash
# pick the smallest remaining function in an overlay
find asm/us/boss/bo6 -path '*/nonmatchings/*' -name '*.s' \
  | while read f; do echo "$(wc -l <"$f") $f"; done | sort -n | head

# find the twin
grep -rn '<FuncNameWithoutPrefix>' src/ric/ --include='*.c' | grep -v INCLUDE_ASM

# look up a symbol's address/type
grep -n 'SYMBOL' config/symbols.us.bobo6.txt

# build + verify (in the container)
docker exec sotn-work bash -lc '. .venv/bin/activate; make build -j8 && ./sotn.sh check us'

# see the diff when it doesn't match
docker exec sotn-work bash -lc '. .venv/bin/activate; \
  python3 tools/asm-differ/diff.py --version=us --overlay=bo6 --format=plain <Func>'

# commit only when you see ✅ us
git add <file> && git commit -m "Decompile <Func> (us)"
```

**The one rule:** the assembly is ground truth. The twin is a hint, the struct is a
hint, your intuition is a hint — but a function is only done when `./sotn.sh check us`
prints `✅ us`.
