# LEGO Indiana Jones: The Original Adventures

The GOG `LEGOIndy.exe` (sha256 6c72717a...e162d1) builds against the engine's Windows runtime
(`engine/windows`). Before this repo existed, Indy was a `GAME=indy` build of WizardBrick-Recomp
(branch feat/indy-build). The history below comes from that branch. The runtime files it names
(`hp_boot.c`, now `pe_boot.c`; `pe_load.c`, `win32_core.c`, `d3dx_misc.c`) are now in
`engine/windows/runtime/src`.

What this repo sets (the engine reads it through `build/Makefile` and `game.conf`):

- `build/Makefile`: Indy's entry RVA 0x3132AC and six sections (`PE_ENTRY_RVA`, `PE_SECTION_COUNT`),
  and `runtime/src/*.c` as the game sources;
- `runtime/src/indy_patches.c`: the game patches (none yet; HP's are keyed by HP call sites). Indy has
  no shader warm-up and no player-address hooks;
- `game.conf`: executable `build.nosync/indy`; app `build.nosync/LEGO Indiana Jones - The Original
  Adventures.app` (launcher `IndyRecompiled`, bundle id `local.indyrecomp`, saves and logs in
  `IndyRecomp`, overrides `INDYRECOMP_GAME_DIR` and `INDYRECOMP_EXE`; icon from `LEGOIndy.exe`).

```sh
make -C build -j8 DEPS=<deps>                    # build.nosync/indy
make -C build app DEPS=<deps> GAME_DIR=<game folder>
engine/windows/tools/package/check_app.sh "build.nosync/LEGO Indiana Jones - The Original Adventures.app"
```

`check_app.sh` never starts the game. It checks that every Mach-O loads only system or @-relative names, that
the ICD manifest names its library relative to itself, that `codesign --verify --deep` passes, that the launcher
sets the Vulkan and DXVK variables and no `DYLD_*`, and that SDL2 opens by leaf name from a library with DXVK's
`@loader_path` rpath (DXVK opens `libSDL2-2.0.0.dylib` that way; the bare binary needs
`DYLD_FALLBACK_LIBRARY_PATH` for it).

Deps built under a temporary or linked prefix record that path: libtool and meson write the absolute install
path as the install name, and Mesa writes it into the ICD manifest. `engine/windows/tools/deps/build_*.sh` rewrite the
install names to `@rpath/<file>` (`rpath_ids` in common.sh) and the manifest's `library_path` to
`../../../lib/<file>`. make_app.sh takes the driver from `<prefix>/lib` by file name, so an old manifest does
not break packaging.

`win32_core.c` binds a `d3dx9_35.dll` import that no table lists to the `d3dx9_41.dll` entry.

The lift needs two inputs HP's analysis did not, both in `recon/`: `init_tables.txt` (the CRT
`__xi`/`__xc` tables at 0x81D50C and 0x81D000) and `add_function_starts.txt` (1760 starts Ghidra missed: code
addresses held by `.data`/`.rdata` tables or by a `mov`/`push` immediate, after int3 fill, a ret or a switch
table; and the 476 incremental-link `jmp` thunks between 0x401005 and 0x402702 that had no function). Lift:
12028 functions, 0 failed.

## Boot blockers, in order (2026-10-26)

| Stop | Cause | Fix |
|---|---|---|
| `NumberOfSections is 0x6, expected 0x7` | `pe_load.c` checked HP's header values | per-game `PE_ENTRY_RVA`, `PE_SECTION_COUNT` |
| indirect call to 0x7128B4 | CRT initialiser tables not analysed | `init_tables.txt` |
| 13 D3DX slots 0 | slices keyed by `d3dx9_41.dll`, Indy imports `d3dx9_35.dll` | `win32_core.c` falls back to the d3dx9_41 entry |
| indirect call to 0x51CE00, 0x54FB30 | handlers only in data tables (pcconfig.txt and audio-script keywords) | `add_function_starts.txt` |
| SIGBUS at guest 0 after `D3DXCreateTextureFromFileInMemory` | no DDS reader (32x32 DXT1 at 0x972F40); the game reads the null texture | `d3dx_misc.c` reads DDS |
| indirect call to 0x632CD0 | callback stored by `mov [0xAD2A28], imm` | `add_function_starts.txt` |
| window never on screen | the game asks for a fullscreen device (Windowed=0, 960x600) on a hidden window and relies on Windows d3d9 showing it | `d3d9g_windowed_in` shows the device window (`w32_ui_show_device_window`) |
| indirect call to 0x401EF6 | incremental-link thunks missing | `add_function_starts.txt` |

The last run (90 s, desktop mode) ran to the time limit: the window was shown and fullscreen
(2056x1290), about 100 frames a second, `DirectSoundCreate8` succeeded, Bink movies were skipped.
59 vertex shaders fail to compile (E5003, writes to globals that the d3dx9_35 compiler allowed); a
separate change on feat/indy-runtime selects the compiler version from the imported DLL name.

## DDS (d3dx_misc.c)

DXT1, DXT3, DXT5, and the uncompressed layouts in `g_dds_masks`; all mip levels; cube maps. MipLevels
D3DX_DEFAULT or 0 gives the full chain: levels the file lacks are box-filtered from the last file level (DXT is
decoded, filtered and encoded again). Resizing, format conversion, colour keys and skipped DDS levels fail
with an UNSUPPORTED line. The DDS files inside the .DAT archives were not inspected: the game's uncompressed
formats are unknown until a run logs one.
