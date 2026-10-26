# Shared Windows runtime into MacRecomp; IndyRecomp repo (plan, 2026-10-26)

Owner decisions (2026-10-26): repo name **IndyRecomp** (private, itsdunintime104); the shared Windows runtime moves into
the **MacRecomp engine**, so HP (WizardBrick-Recomp) and Indy (IndyRecomp) are thin game repos with `engine/` as a
submodule, like Hobbit and Marvel. Route profile: claude-only; Opus workers (owner directive). No game launches.

## Design

- Engine gets one new subtree `windows/` (no collision with the Mach-O `runtime/` and `tools/package`):
  - `windows/runtime/src`, `windows/runtime/include`: every generic file from WizardBrick-Recomp `runtime/` (win32_*,
    d3d9_*, d3dx_*, com_guest, pe_load, frame_trace, input_script, the boot driver hp_boot.c/hp_boot_main.c renamed to
    pe_boot*.c with GAME_* settings, gen_d3d9_guest.*).
  - `windows/tools/{pe,win32,d3d9,deps,ghidra,vkd3d,package,...}`: generic tools. Per-game constants become inputs
    derived from the binary or read from the game repo (build_names: --binary and a startup-names file;
    run_lift_pe: find `__EH_epilog3_GS` by its byte pattern instead of a typed VA).
  - `windows/tests`: the tests of the generic files + `run_all.sh`.
  - `windows/build/game.mk`: the build rules. A game repo's `build/Makefile` sets GAME_ID, GAME_PREFIX, GEN,
    GAME_SRCS, GAME_CFLAGS, APP_* and includes it. No GAME=hp|indy switch inside the engine.
  - `windows/setup.sh`: the generic setup (from HP setup.sh) driven by the game repo's `game.conf` (shell variables:
    names, exe file name, sha256, lift prefix, recon paths). Each game repo's `setup.sh` is a thin wrapper.
- Game repos keep only game-specific files: recon/, notes/, docs/, game patches (hp_patches/hp_hud/hp_nav,
  indy_patches), their tests, build/Makefile, game.conf, setup.sh wrapper, README, LICENSE.
- Public-repo rule (MacRecomp is public): no /Users paths, local user name, decompiled pseudocode, game files or
  game-specific addresses in the engine. Game addresses stay in game repos.

Rejected: HP repo as Indy's submodule (game depends on game); copying the runtime (two drifting copies).

## Slices

| # | Repo / branch | Deliverable | Depends |
|---|---|---|---|
| S1 | MacRecomp feat/windows-runtime (from origin/main) | windows/ subtree from WizardBrick-Recomp feat/indy-build 2bb8208; game.mk; generic tools; windows tests pass in the engine | - |
| S2 | WizardBrick-Recomp refactor/engine-windows (from feat/indy-build) | moved files removed; build/Makefile + game.conf + setup.sh wrapper; engine/ -> S1 | S1 |
| S3 | IndyRecomp main (new repo, git init) | recon/, notes/, indy_patches, build/Makefile, game.conf, setup.sh wrapper, README (HP style), LICENSE, engine/ -> S1 | S1 |

S2 and S3 run in parallel after S1.

| S4 | MacRecomp refactor/mac-split (after S1-S3 merge) | owner 2026-10-26: one engine, Mac and Windows separated. Mac-only parts (Mach-O analyzer, Feral front end, Cg/GL, Mac packaging) move to `mac/`; shared lifter and runtime core stay at one shared place; Hobbit/Marvel submodule bumps with byte-identical generated C, recomp-check PASS, builds | S1-S3 merged |

## Checks

- S1: windows tests in the engine (all HP generic tests: 20 pass, d3dx_shader may skip); existing engine pytest suite
  still passes; leak scan (grep /Users, user name, game VAs in windows/) clean.
- S2: HP compile inputs identical by file name (make -n normalised for path prefixes); `make` links lhp; HP game tests
  pass; `make app` + check_app.sh pass. No game launch.
- S3: `make` links indy; `make app` + check_app.sh pass; leak scan; setup.sh --dry-run works on the GOG folder.

## Risks

- HP regression through path moves: S2's make -n diff and both test suites.
- Public leak through moved comments: S1 leak scan before any push.
- setup.sh generalisation breaks HP install: S2 runs `setup.sh --dry-run` against the owner's Steam folder.
