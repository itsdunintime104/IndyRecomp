# LEGO Indiana Jones: The Original Adventures, native arm64 (plan, 2026-10-26)

Route profile: claude-only. Lead: Opus 5.5. Workers: opus-worker (owner directive: all new workers are Opus).

## Facts (checked)

- Source: GOG Windows build. `LEGOIndy.exe` PE32 i386, sha256 6c72717abc5b38a44ec235540176c76a2778c1d8f0dd8889b714a12391e162d1, no packer, no DRM section.
- Copies: `orig.nosync/LEGOIndy.exe`; game data APFS clone `game.nosync/` (never write the Downloads original).
- Imports (203 named + ordinals): same set as HP except
  - `binkw32.dll` 17 functions (3 movies only: TT logo, demo intro, Batman advert; Bink 1 `BIKi`; cutscenes are in-engine);
  - `DSOUND.dll` ordinal 11 (DirectSoundCreate8); HP uses XAudio2;
  - `XINPUT1_3.dll` ordinals 2, 3, 4 (XInputGetState, XInputSetState, XInputGetCapabilities);
  - `OLEAUT32.dll` ordinals 2, 6 (SysAllocString, SysFreeString);
  - `winmm.dll` timeGetTime, timeSetEvent, timeKillEvent;
  - kernel32 GetFileAttributesExA, ResetEvent; user32 LoadKeyboardLayoutA, MapVirtualKeyExA, ShowCursor; gdi32 DeleteObject;
  - `d3dx9_35.dll`: same 13 names as HP's d3dx9_41 except D3DXMatrixLookAtLH (HP has D3DXGetShaderConstantTable).
- Audio data: 276 OGG streams the game decodes itself, played through DirectSound.
- Pipeline (tools copied from WizardBrick-Recomp `tools/pe`, `tools/ghidra`):
  Ghidra ReanalyzePE + ExportPE (8857 functions) -> analyze_pe (9251 starts) -> run_sweep_pe (0 reg switches) ->
  ExportNameEvidence + build_names (about 1400 names, with a LEGO Saga build as the name reference) -> run_lift_pe --prefix indy.
- Indy-specific tool edits: `build_names.py` BINARY and STARTUP_TRACE; `run_lift_pe.py` UNNAMED_EPILOGS = 0x0072D209
  (`__EH_epilog3_GS`, one byte-pattern hit: call 0x712739 `__security_check_cookie`, jmp 0x72D1F5 `__EH_epilog3`).
- Since the move into the engine (`engine/windows/tools/pe`) no tool has Indy edits: run_lift_pe finds 0x0072D209 by
  its bytes, and the re-lift with the engine tools wrote byte-identical C (same lift report).
- Lift: 9717 functions, 0 failed, 75 s. 11 unhandled, all inside CRT `_memcpy` (0x712CD0) and `_memmove` (0x7172A0):
  inline jump-table bytes decoded as code; all 70 table arms have labels, so the decoded bytes are unreachable.
  8 unresolved call stubs (CRT range 0x714000-0x723000, plus 0x1071E635 outside the image): open.

## Design

Indy builds against the WizardBrick-Recomp runtime (generic Win32, D3D9 guest, D3DX, COM, input, audio), not a copy.
New generic pieces land in that runtime on branches; HP must keep building and its tests must pass.
Moving the shared Windows runtime into the MacRecomp engine is a later owner decision, after Indy boots.

Rejected: copying the HP runtime into a new Indy repo (two diverging 20k-line copies).

## Lanes (parallel, separate worktrees of WizardBrick-Recomp from origin/main)

| Lane | Branch | Deliverable |
|---|---|---|
| A build-boot | feat/indy-build | build selects a game (HP default unchanged); Indy links; boot run to first blocker |
| B dsound | feat/dsound | DirectSoundCreate8 + IDirectSound8 + IDirectSoundBuffer(8) (+3D/listener if Indy uses them) on the host audio path, host test |
| C imports | feat/indy-imports | ordinal names (DSOUND 11, XINPUT1_3 2/3/4, OLEAUT32 2/6), winmm timers, 6 small imports, Bink stubs (BinkOpen fails, movies skipped), host tests |

Contracts: thunk names `imp_<Name>` per docs/WIN32_THUNKS.md; lane C maps DSOUND ordinal 11 to the name DirectSoundCreate8,
lane B implements `imp_DirectSoundCreate8`. Each lane adds new files where it can, to keep merges trivial.
Merge order into feat/indy-build: C, then B. Lane A reruns the boot after each merge.

## Risks

- HP regression from shared-runtime edits: HP build + HP tests per lane.
- The 8 unresolved stubs may be real CRT code the analyzer missed (add_function_starts like HP).
- Indy (2008) engine is older than HP (2010): HP-era assumptions in hp_boot.c (stack, TIB, threads) may not hold.

## Game-run rules (all lanes)

pgrep -x lhp/lhp_boot/hobbit/marvel/indy empty first; private HOME in a temporary folder; RECOMP_ASSUME_FOCUS=1, RECOMP_NO_ALERT_UI=1;
`arch -arm64 /usr/bin/perl -e 'alarm N; exec @ARGV'` timeouts; stop own PIDs only, never pkill; never write the Downloads
folder, the Steam folder, the Wine prefix or ~/Library/Application Support/HarryPotterRecomp; no screenshots or recordings.
