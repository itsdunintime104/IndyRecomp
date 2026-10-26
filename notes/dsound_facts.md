# DirectSound evidence in LEGOIndy.exe (lead, 2026-10-26)

- IAT slot for DSOUND.dll ordinal 11 (DirectSoundCreate8): VA 0x02A177D4, thunk 0x769264, only call 0x52A3A2 in sub_0052A360
  (lane B; lead re-checked by parsing the import directory).
- Correction: 0x02A177A4 (thunk 0x76AD60, call 0x51C5FF) is DINPUT8!DirectInput8Create, not DirectSound. The lead misread
  which DLL an objdump `addr` line belonged to.
- Method list by vtable offset with call sites: notes/dsound.md (WizardBrick-Recomp feat/dsound 7b3aaee).
- IIDs in .rdata (one GUID block 0x876DC4-0x876E74): all seven DirectSound IIDs and DSDEVID_DefaultPlayback (0x876EB4) are present,
  but code refers to only three (address used as an immediate in .text):

| IID | .rdata VA | code site | containing function |
|---|---|---|---|
| IID_IDirectSound3DListener | 0x876E14 | 0x52A489 | sub_0052A360 |
| IID_IDirectSound3DBuffer | 0x876E04 | 0x52AF96 | sub_0052AF70 |
| IID_IDirectSoundBuffer8 | 0x876E34 | 0x52F7FB, 0x53086F | sub_0052F730, sub_00530610 |

- No code reference to IID_IDirectSoundNotify, IID_IDirectSound8, IID_IDirectSoundBuffer or DSDEVID_DefaultPlayback.
- The sound driver code sits in about 0x51C000-0x531000.
