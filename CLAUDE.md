# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A **matching decompilation** of Castlevania: Symphony of the Night. "Matching" means the rebuilt binaries are byte-for-byte identical to the originals extracted from a real copy of the game. Code that hasn't been decompiled yet stays as raw assembly (referenced via `INCLUDE_ASM`); decompiling means replacing that with C that compiles to the *exact same* instructions.

Supported versions, selected with `VERSION=` (default `us`): `us` (SLUS-00067, the reference build), `hd` (unreleased PS1 JP build), `pspeu` (PSP), `saturn`, `pc`. PSX versions use a 32-bit Linux x86 PSX compiler (`bin/cc1-psx-26`, GCC 2.6.3); PSP uses `mwccpsp`.

The repo ships no game assets or assembly — a personal copy of the game is required, and disk images go in `disks/` (e.g. `disks/sotn.us.cue`).

## Building on this machine (Apple Silicon macOS)

**The build cannot run natively on macOS.** `bin/cc1-psx-26` is a 32-bit Linux x86 ELF that macOS/Rosetta cannot execute. This fork builds inside a `linux/amd64` Docker container instead, where Rosetta accelerates the x86-64 emulation.

- One-time host setup (installs Rosetta + Docker): `./setup-mac.sh`
- Full build + match verification: `./build-mac.sh`

`build-mac.sh` builds the `sotn-build` Docker image on first run, rebuilds the native-arm helper binaries (`sotn-disk`, `sotn-assets`, `sotn_str`) as Linux ones, keeps `.venv` and `build/` in named Docker volumes (so the host's native copies never mix in), and runs extract → build → `./sotn.sh check us`. Iterating means re-running `build-mac.sh` (the build volume persists, so it's incremental) or `docker exec`-ing into a long-lived container. The `make` commands below are run *inside the container*, not on the host.

## Common commands (run inside the Linux container / on a Linux host)

Everything is driven by the `Makefile`, which wraps `./sotn.sh` (itself `go run ./tools/sotn-assets`). `make help` lists targets.

- `make extract` — disassemble code and extract assets for `$(VERSION)` (regenerates `asm/`, `assets/`, and per-overlay linker/symbol files under `build/`)
- `make build` — compile + link all overlays; success means everything that should match, matches
- `./sotn.sh check us` — verify checksums of every built file against the originals (there is **no** `make check` target)
- `make extract_disk` — dump game files from the disk image in `disks/` (run before the first extract)
- `make format` — clang-format the C and run lints/symbol cleanup
- `make test` — `tools/symbols_test.py` sanity checks
- `make context SOURCE=src/...` — generate `ctx.c` (preprocessed translation unit) for decomp.me / m2c
- `make VERSION=pspeu build` — build a different version

Comparing your C against the target during decompilation uses asm-differ via `tools/sotn_diff.py` (e.g. `tools/sotn_diff.py --version=us -b dra.bin -m dra.map`), and `tools/m2c` turns assembly into draft C.

## Architecture

**Overlay-oriented.** The game is a small `main` executable plus many overlays loaded on demand: `DRA.BIN` (engine), `RIC.BIN` (playable Richter), `WEAPON*`, stage overlays (`ST/<stage>`), and boss overlays (`BOSS/<boss>`). Each overlay is an independent build unit.

The mapping between an overlay and the tree, repeated for every overlay:
- `config/splat.<version>.<overlay>.yaml` — splat config: memory layout and which ranges are code vs data vs assets
- `config/symbols.<version>.txt`, `config/symbol_addrs*.txt`, `config/undefined_syms*.txt` — the symbol map (named functions/data and fixed PSX RAM addresses for SDK/BIOS symbols)
- `src/<overlay>/` — decompiled C (e.g. `src/main`, `src/dra`, `src/ric`, `src/st/<stage>`, `src/boss/<boss>`, `src/weapon`, `src/servant`). PSP-specific variants live in `*_psp` dirs; `src/saturn` and `src/pc` for those ports.
- `asm/<version>/<overlay>/` — splat-extracted assembly for not-yet-decompiled functions, pulled in by `INCLUDE_ASM` macros (see `include/include_asm.h`)
- shared headers in `include/`

**Build pipeline.** `tools/builds/gen.py` reads the splat configs and generates `build.ninja`. The PSX compile rule is a pipeline: `mipsel-linux-gnu-cpp | bin/cc1-psx-26 | maspsx (tools/maspsx) | mipsel-linux-gnu-as`, then `mipsel-linux-gnu-ld` links objects using the generated `.ld` linker script plus the `undefined_syms` files. `maspsx` reproduces the quirks of the original PSX SDK assembler — that fidelity is what makes byte-matching possible.

**Key tools** (many are git submodules under `tools/`, fetched by `make update-dependencies`):
- `tools/sotn-assets` (Go) — the orchestration layer behind `sotn.sh`: `build`, `extract-assets`, `build-assets`, `check`, `clean`, `bindiff`
- `tools/sotn-disk` (Go) — extract files from / rebuild a disk image
- `tools/m2c`, `tools/asm-differ`, `tools/decomp-permuter`, `tools/maspsx` — the decompilation toolchain (m2c = asm→C, asm-differ = compare, permuter = search for matching variations)
- `tools/sotn_str` (Rust) — string encoding, invoked directly by the compile pipeline (not part of the make graph, so build it explicitly when needed)
- `tools/mipsmatch`, `tools/dups` — duplicate-function detection used when splitting/sharing overlay code

**Decompiling a function (typical loop):** find an `INCLUDE_ASM` in a `src/<overlay>` file → `make context SOURCE=<that file>` and/or run m2c to get draft C → replace the `INCLUDE_ASM` with C → `make build` and diff with `tools/sotn_diff.py` until it matches → the function is done when the overlay still builds and `./sotn.sh check` passes.

## Conventions

- Placeholder names use `func_`, `D_`, or `Unk` prefixes; renaming them to meaningful names (understood via a PS1 debugger) is a valid, encouraged change. See `docs/NAMING.md` and `docs/STYLE.md`.
- C is formatted with clang-format via `make format` (config in `.clang-format`).
