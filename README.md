> [!IMPORTANT]
> This project does not include any game files. You need your own GOG copy of LEGO Indiana Jones: The Original Adventures (the Windows version).

<h1 align="center">
  Indy Recompiled
</h1>

<p align="center">
  <img src="https://img.shields.io/badge/platform-macOS%20Apple%20Silicon-black?logo=apple" alt="Platform: macOS on Apple Silicon">
  <img src="https://img.shields.io/badge/status-playable-yellow" alt="Status: playable">
  <img src="https://img.shields.io/badge/license-GPL--3.0-blue" alt="License: GPL-3.0">
</p>

LEGO Indiana Jones: The Original Adventures (Traveller's Tales, 2008) is sold on GOG as a Windows game. This project translates the game's 32-bit Windows code to C ahead of time and compiles it into a native arm64 app for Apple Silicon. It does not use Wine or an emulator. Direct3D 9 calls go to [DXVK](https://github.com/doitsujin/dxvk), which draws with Vulkan on [KosmicKrisp](https://docs.mesa3d.org/drivers/kosmickrisp.html), Mesa's Vulkan driver for Metal.

The translator and the Windows runtime are [MacRecomp](https://github.com/itsdunintime104/MacRecomp), the same engine that runs [Wizard Bricks Recompiled](https://github.com/itsdunintime104/WizardBrick-Recomp) (LEGO Harry Potter: Years 1-4), [Hobbit Recompiled](https://github.com/itsdunintime104/HobbitRecompiled) and [Superheros Bricks Recompiled](https://github.com/itsdunintime104/SuperheroBrickRecomp). This repo holds only the Indy-specific parts: the function analysis of the game binary, the build settings and the notes.

This project is not affiliated with or endorsed by TT Games, LucasArts, Lucasfilm, Disney or the LEGO Group.

## Contents

- [Requirements](#requirements)
- [Get the game files](#get-the-game-files)
- [Install](#install)
- [Saves](#saves)
- [Status](#status)
- [Known issues](#known-issues)
- [Troubleshooting](#troubleshooting)
- [Building and development](#building-and-development)
- [Credits](#credits)
- [License](#license)

## Requirements

- A Mac with Apple Silicon (M1 or newer)
- LEGO Indiana Jones: The Original Adventures from GOG, Windows version
- Xcode Command Line Tools
- Homebrew with the packages listed in [Install](#install)
- Python 3.11 or newer
- About 3 GB of free disk space for the build, plus 3.6 GB for the game files

## Get the game files

The app needs the installed game folder: the one that holds `LEGOIndy.exe` and the `.DAT` files. Use one of these, with your own GOG licence:

1. **A Windows PC**: install the game with GOG and copy its folder to your Mac.
2. **GOG Galaxy inside Wine, CrossOver or Whisky**: `setup.sh` finds the game there by itself.

Only one build of `LEGOIndy.exe` is supported (sha256 `6c72717abc5b38a44ec235540176c76a2778c1d8f0dd8889b714a12391e162d1`). `setup.sh` checks it.

## Install

1. Install the Xcode Command Line Tools, and wait until the installer says it is done:
   ```sh
   xcode-select --install
   ```
2. Install [Homebrew](https://brew.sh), then the tools:
   ```sh
   brew install sdl2 sdl3 faudio vulkan-loader vulkan-headers meson ninja pkgconf glslang \
       llvm spirv-llvm-translator spirv-tools zstd autoconf automake libtool bison flex mingw-w64 python
   ```
3. Get the code and run the setup script with your game folder:
   ```sh
   git clone --recurse-submodules https://github.com/itsdunintime104/IndyRecomp.git
   cd IndyRecomp
   ./setup.sh "$HOME/Games/LEGO Indiana Jones - The Original Adventures"
   ```
4. Open **LEGO Indiana Jones - The Original Adventures** from `~/Applications`.

`setup.sh` checks your Mac, builds DXVK, vkd3d-shader and KosmicKrisp at pinned versions with the engine's patches (about 4 minutes, once), copies the game's exe, checks its sha256, translates it, builds the app and installs it. It never writes to the game folder. Without a folder argument it searches the usual places and asks.

Keep the cloned folder: the app loads the game's code from `orig.nosync/` in it. The app remembers the game folder you gave.

| Option | Effect |
|---|---|
| `--check` | Only check the requirements |
| `--force` | Translate again. Use it after `git pull` |
| `--no-install` | Build the app in `build.nosync/` without copying it to `~/Applications` |
| `--dry-run` | Print each step without running it |

The app reads two settings: `INDYRECOMP_GAME_DIR` (the game folder, if it moved after setup) and `INDYRECOMP_EXE` (the game image).

Each run writes a log to `~/Library/Logs/IndyRecomp/`. The app keeps the newest 20. Attach the log when you report a problem.

## Saves

The game writes its saves and settings to its Windows profile folder. The app redirects that folder to `~/Library/Application Support/IndyRecomp`. It never writes to the game folder.

> [!WARNING]
> Back up `~/Library/Application Support/IndyRecomp` before you play.

## Status

The game boots to its menus and into gameplay, with sound. It was played by hand on 2026-10-26.

## Known issues

- The game crashes when you quit: the C runtime's exit path (at 0x0071450A) makes an indirect call to 0x0071979E, which the translation does not have yet.
- DDS textures in the float format A32B32G32R32F (FourCC 116) are not read.
- Bink movies (the logos and the intro) are skipped.

## Troubleshooting

| Problem | Fix |
|---|---|
| "game folder not found" | Give the folder that holds `LEGOIndy.exe` as the argument of `setup.sh`, or set `INDYRECOMP_GAME_DIR` |
| `setup.sh` says the sha256 does not match | Only the GOG build of `LEGOIndy.exe` is supported |
| macOS says the app is damaged | Build the app with `setup.sh` on your own Mac. A copy from another Mac does not run |

## Building and development

The build rules, the runtime, the tools and the tests are in the engine: [engine/windows/README.md](https://github.com/itsdunintime104/MacRecomp/blob/main/windows/README.md) describes the interface this repo uses (`build/Makefile`, `game.conf`, the game hooks). By hand, after `setup.sh` has built the graphics libraries:

```sh
make -C build -j8                 # build.nosync/indy
make -C build test                # the engine's Windows runtime tests
make -C build app GAME_DIR=<game folder>
engine/windows/tools/package/check_app.sh "build.nosync/LEGO Indiana Jones - The Original Adventures.app"
```

| Path | Contents |
|---|---|
| `recon/` | function analysis of `LEGOIndy.exe`: Ghidra exports, extra function starts, CRT initialiser tables, names, switch tables |
| `runtime/src/` | Indy's game patches (`game_hooks.h`) |
| `build/Makefile`, `game.conf`, `setup.sh` | the settings the engine's build rules and setup read |
| `notes/` | how Indy was brought up: imports, boot blockers, DirectSound, stub audit |
| `engine/` | MacRecomp (submodule) |

## Credits

- **[MacRecomp](https://github.com/itsdunintime104/MacRecomp)**: the translator, the generic runtime and the Windows runtime.
- **[xboxrecomp](https://github.com/sp00nznet/xboxrecomp)** by sp00nznet: the x86-to-C translator MacRecomp's translator is forked from (MIT license).
- **[DXVK](https://github.com/doitsujin/dxvk)** (zlib license), with the macOS patches from [jm2/dxvk](https://github.com/jm2/dxvk): Direct3D 9 to Vulkan.
- **[vkd3d-shader](https://gitlab.winehq.org/wine/vkd3d)** (LGPL-2.1): compiles the game's HLSL shaders.
- **[Mesa](https://mesa3d.org) KosmicKrisp** (MIT license): Vulkan on Metal.
- **[FAudio](https://github.com/FNA-XNA/FAudio)** (zlib license): the audio output behind the game's DirectSound.
- **[SDL2](https://www.libsdl.org/)** (zlib license): window, input and controllers.
- **[Capstone](https://www.capstone-engine.org/)** (BSD license): x86 disassembly for the translator.

## License

GPL-3.0. See [LICENSE](LICENSE). Third-party code keeps its own license: see `engine/tools/xrecomp/LICENSE`, `engine/tools/xrecomp/NOTICE` and the header of `engine/runtime/src/dlmalloc.c`. The patches in `engine/windows/tools/` keep the license of the project they patch.

The repository contains no game assets, no game binary and no code generated from the game. `recon/` holds function addresses, import names and switch tables taken from the game binary, which the translator needs. The app icon is made from your own copy of the game when you build.
