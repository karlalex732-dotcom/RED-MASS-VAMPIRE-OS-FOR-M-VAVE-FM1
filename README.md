# RED MASS VAMPIRE OS

**GPL-3.0 source repository.** The tagged `v0.5.1-alpha` GitHub Release should carry `RED-MASS-VAMPIRE-OS-v0.5.1-alpha-FM1.fwsc` as a separate downloadable asset. The Windows installer executable is not in this repository or release bundle.


Developer and contributor: **Karsei**.

Custom GPL-3.0-only firmware for the M-VAVE FM1. v0.5.1-alpha adds the RED MASS interface, uniform bold fonts, two original trap kits and six independent tracks: five instruments plus drums on track 6. The eight-voice budget is shared.

256 curated built-in sounds, including eight sampled grand-piano variations; 32 editable user slots; 64 steps per track; 32 Song Mode rows using four project sources; automation, scenes, macros, microtiming and the twelve performance tools.

Read [the English guide](docs/RED-MASS-English-Guide.md) and [upgrade notes](docs/Upgrade-v0.5-EN.md). Back up all music before updating. Older four-track projects and feature data migrate together; new FUNB/KPO5 saves require this firmware.

Build on Linux x86-64:

```sh
python3 -m pip install Pillow fonttools
sh tools/get_toolchain.sh
export JIELI_TOOLCHAIN="$HOME/.jieli/toolchain"
export AC79_SDK="$HOME/fw-AC79_AIoT_SDK"
python3 tools/build.py --release 0.5.1-alpha --identity FM-1_906
sh tests/run_tests.sh
```

Outputs: `build/red_mass-0.5.1-alpha.fwsc`, `build/red_mass.bin` and `build/release-0.5.1-alpha/`. The release folder includes licence notices and hashes. The Windows application uses a native EXE and the complete bundled editor/installer; Python is not required at runtime. See BUILDING.md for firmware SDK setup.

The host suite exercises real C audio/UI/persistence/protocol paths against a simulated display and NOR flash. A target build checks image and SRAM limits. Physical FM1 boot, flash behavior, audio timing and sustained load still need qualification.

To refresh the extracted English pack after a build, capture the real firmware display and pass the build and suite logs to the packager:

```sh
mkdir -p build/red-mass-tools
gcc -O2 -w -Ibuild/gen -Ifirmware/src -o build/red-mass-tools/capture tests/upgrade_capture.c -lm
build/red-mass-tools/capture build/red-mass-tools
python3 tools/build_desktop.py
python3 tools/stage_desktop_assets.py
python3 web/make_site.py build/red_mass-0.5.1-alpha.fwsc 0.5.1-alpha ../web-tools
python3 tools/package_red_mass.py --suite build/red-mass-suite.log --target build/red-mass-target.log --desktop-build build/desktop-build.log --desktop-check build/desktop-http.log --app-check build/app-navigation.log --visual-recheck build/final-visual-check.log
```

Capture `tools/build.py` output in `build/red-mass-target.log` and `sh tests/run_tests.sh` output in `build/red-mass-suite.log`. Capture `tools/build_desktop.py` output in `build/desktop-build.log`. Compile `tests/desktop_server_test.c` with `gcc -O2 -ffunction-sections -fdata-sections -fshort-wchar -Wl,--gc-sections`, run it with the extracted pack's absolute root as its only argument and capture the output in `build/desktop-http.log`. Run `node web/test_app.mjs` into `build/app-navigation.log`. The packager requires passing checks and retains the extracted pack's 256 audio examples. `tests/desktop_browser_test.cjs` is optional Playwright layout QA; it needs a separately installed Chromium and Playwright on the development machine. Windows execution and physical FM-1 qualification are separate from these host checks.

Original firmware contributions: Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments. Modifications are distributed under the same GPL-3.0-only licence. Complete corresponding source, original copyright notices and third-party licences are included; see LICENSE and LICENSING.md.
