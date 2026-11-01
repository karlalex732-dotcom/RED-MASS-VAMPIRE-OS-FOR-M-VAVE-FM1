# RED MASS VAMPIRE OS

> **v0.5.2-beta:** 216 darker voicings; 32 bass/808 sounds, eight percussion presets and both trap kits preserved. Factory numbers and names stay. See `docs/DARK-BANK-0.5.2-RU.md` for installation, A/B listening and validation.


**GPL-3.0 source repository.** The tagged `v0.5.2-beta` GitHub Release carries `RED-MASS-VAMPIRE-OS-v0.5.2-beta-FM1.fwsc` as a separate downloadable asset. The release includes a separate complete Windows installer bundle; the repository contains the corresponding source.


Developer and contributor: **Karsei**.

Custom GPL-3.0-only firmware for the M-VAVE FM1. The firmware includes the RED MASS interface, uniform bold fonts, two original trap kits and six independent tracks: five instruments plus drums on track 6. The eight-voice budget is shared.

256 curated built-in sounds, including eight sampled grand-piano variations; 32 editable user slots; 64 steps per track; 32 Song Mode rows using four project sources; automation, scenes, macros, microtiming and the twelve performance tools.

Read [the English guide](docs/RED-MASS-English-Guide.md) and [upgrade notes](docs/Upgrade-v0.5-EN.md). Back up all music before updating. Older four-track projects and feature data migrate together; new FUNB/KPO5 saves require this firmware.

Build on Linux x86-64:

```sh
python3 -m pip install Pillow fonttools
sh tools/get_toolchain.sh
export JIELI_TOOLCHAIN="$HOME/.jieli/toolchain"
export AC79_SDK="$HOME/fw-AC79_AIoT_SDK"
python3 tools/build.py --release 0.5.2-beta --identity FM-1_907
sh tests/run_tests.sh
```

Outputs: `build/red_mass-0.5.2-beta.fwsc`, `build/red_mass.bin` and `build/release-0.5.2-beta/`. The release folder includes licence notices and hashes. The Windows application uses a native EXE and the complete bundled editor/installer; Python is not required at runtime. See BUILDING.md for firmware SDK setup.

The host suite exercises real C audio/UI/persistence/protocol paths against a simulated display and NOR flash. A target build checks image and SRAM limits. Physical FM1 boot, flash behavior, audio timing and sustained load still need qualification.

For the beta sound-bank and complete installer bundle, see [the update notes](docs/DARK-BANK-0.5.2-RU.md) and [validation scope](docs/TESTING-0.5.2-RU.md). `tools/package_dark_release.py` assembles the installer, audio player, licences and rollback after the native renders and build checks. Its inputs include the preceding installer and baseline source tree.

Original firmware contributions: Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments. Modifications are distributed under the same GPL-3.0-only licence. Complete corresponding source, original copyright notices and third-party licences are included; see LICENSE and LICENSING.md.
