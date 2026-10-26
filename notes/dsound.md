# DirectSound 8 in LEGOIndy.exe (2026-10-26)

Source: capstone disassembly of `LEGOIndy.exe` (sha256 6c72717a...) over 0x51C000-0x531000, plus byte
searches of `.text`. Implementation: `engine/windows/runtime/src/win32_dsound.c`; test: `engine/windows/tests/test_win32_dsound.c`.

## Import

| What | VA |
|---|---|
| IAT slot, DSOUND.dll ordinal 11 (DirectSoundCreate8) | 0x02A177D4 |
| Import thunk `jmp [0x02A177D4]` | 0x00769264 |
| Only call of the thunk | 0x0052A3A2 in fn 0x0052A360: `DirectSoundCreate8(NULL, &drv->ds, NULL)` |

0x02A177A4 / thunk 0x0076AD60 / call 0x0051C5FF is `DINPUT8.dll!DirectInput8Create` (5 arguments,
version 0x800), not DirectSound.

Objects: the sound driver is a global at 0x009E21A0. `+0xD18` IDirectSound8 (also read by the getter
0x0052B110, whose only caller 0x0058A9F1 hands it to Bink's sound system), `+0xD80` primary buffer,
`+0xD0C` sample table (0x40C bytes per sample, buffer at +0), voices at `+0xC + 0x34*i` (buffer +0,
3D buffer +4, 64 of them). Listener: global 0x009E218C. Streams: objects from fn 0x00530610 (two at
0x009E2158 and 0x009E215C), buffer at `+0x54`, a thread each (started at 0x005308AD).

## Methods called (vtable offset / slot, call sites)

IDirectSound8 (12 slots):

| Off | Method | Call sites |
|---|---|---|
| 0x0C | CreateSoundBuffer | 0x52A437 (primary), 0x52A840 (sample), 0x52F7EC, 0x53085D (stream) |
| 0x10 | GetCaps (dwSize 0x60) | 0x52A3D7 |
| 0x14 | DuplicateSoundBuffer | 0x52AC1D (one voice per play) |
| 0x18 | SetCooperativeLevel (DSSCL_PRIORITY) | 0x52A3C0 |
| 0x20 | GetSpeakerConfig | 0x52A3E8 |
| 0x08 | Release | 0x52C34E |

IDirectSoundBuffer / IDirectSoundBuffer8 (24 slots):

| Off | Method | Call sites |
|---|---|---|
| 0x00 | QueryInterface: IID_IDirectSound3DListener on the primary | 0x52A48F |
| 0x00 | QueryInterface: IID_IDirectSound3DBuffer on a voice | 0x52AF9E |
| 0x00 | QueryInterface: IID_IDirectSoundBuffer8 on a stream | 0x52F801, 0x530875 |
| 0x08 | Release | 0x52ADB9, 0x52C396, 0x52C4F6, 0x52F759, 0x52F82D, 0x530087, 0x5308D6, others |
| 0x10 | GetCurrentPosition (play only, write NULL) | 0x52F9CC |
| 0x18 | GetVolume | 0x52C457 |
| 0x24 | GetStatus (tests PLAYING 1, LOOPING 4) | 0x52AD4E, 0x52C41C, 0x52C4C8 |
| 0x2C | Lock (flags 0) | 0x52A8A5, 0x52FA9C, 0x52FADE |
| 0x30 | Play (0, 0, 0 or DSBPLAY_LOOPING) | 0x52AA3E, 0x52AD68, 0x52FF39, 0x530319, 0x530410 |
| 0x34 | SetCurrentPosition (0) | 0x5302C3, 0x5302DD |
| 0x38 | SetFormat (primary: PCM 2 ch 44100 Hz 16-bit) | 0x52A47C |
| 0x3C | SetVolume (clamped to -10000) | 0x52A1CD, 0x52A202, 0x52ACCD, 0x52B065, 0x52F815, 0x52FE09, 0x5304ED, 0x5305A2 |
| 0x44 | SetFrequency | 0x52AD3E, 0x52B0FF |
| 0x48 | Stop | 0x52ADAF, 0x52C38C, 0x52C4EC, 0x52FB49, 0x52FC18, 0x52FEE6, 0x53019D, 0x530387, 0x5303DF, 0x530552 |
| 0x4C | Unlock | 0x52A93C, 0x52FCAB |
| 0x50 | Restore (after Lock returns DSERR_BUFFERLOST) | 0x52FAAE |

IDirectSound3DBuffer (21 slots), all from fn 0x0052AF70; dwApply is DS3D_DEFERRED unless the
caller's third argument is non-zero:

| Off | Method | Call site |
|---|---|---|
| 0x40 | SetMaxDistance (1e9) | 0x52AFB7 |
| 0x44 | SetMinDistance (1e9) | 0x52AFCC |
| 0x48 | SetMode (DS3DMODE_HEADRELATIVE) | 0x52AFD9 |
| 0x4C | SetPosition | 0x52AFFE |
| 0x08 | Release | 0x52ADA5, 0x52C382, 0x52C4E2 |

IDirectSound3DListener (18 slots):

| Off | Method | Call site |
|---|---|---|
| 0x34 | SetOrientation (front 0,0,1; top 0,1,0; deferred) | 0x52A4E1 |
| 0x38 | SetPosition (0,0,0; deferred) | 0x52A4B1 |
| 0x3C | SetRolloffFactor (0; deferred) | 0x52A4F5 |
| 0x44 | CommitDeferredSettings | 0x52A502, 0x52C570, 0x52C5C9 (every update) |
| 0x08 | Release | 0x52C316 |

No call reads DSCAPS or the speaker config back (no reads of driver+0xD1C..0xD7C).

## Buffers and formats

| Buffer | DSBUFFERDESC (dwSize 0x24) | Format |
|---|---|---|
| Primary (0x52A426) | flags 0x11 PRIMARYBUFFER, CTRL3D; bytes 0; no format | SetFormat 44100 Hz stereo 16-bit |
| Sample (0x52A7A2) | flags 0xB8 CTRL3D, CTRLFREQUENCY, CTRLVOLUME, LOCSOFTWARE; guid3DAlgorithm GUID_NULL | the decoded sample's WAVEFORMATEX (fn 0x00530C00); must be mono for CTRL3D |
| Stream (0x5307B2, 0x52F7CD) | flags 0x10080 CTRLVOLUME, GETCURRENTPOSITION2; bytes 4 x nAvgBytesPerSec | the stream's WAVEFORMATEX |

Rolloff 0 and min = max = 1e9 mean the game does its own distance attenuation through SetVolume
(fn 0x0052AEA0); DirectSound 3D only pans the head-relative position.

## What the slice does

- One FAudio source voice (float stereo, device rate) per device; its pass-start callback mixes all
  playing buffers. Linear resampling, DirectSound volume and pan laws, one-shot buffers stop and
  rewind to 0 at their end.
- 3D: inverse-distance rolloff between min and max distance, pan = 10000 x sin(azimuth), normal and
  head-relative modes, DS3D_DEFERRED settings applied by CommitDeferredSettings. No HRTF, cones or
  Doppler.
- Methods not in the tables above return DSERR_UNSUPPORTED (E_NOTIMPL) and log
  `UNSUPPORTED dsound: <Interface>::<Method>` once.
