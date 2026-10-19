RED MASS VAMPIRE OS - English Field Guide

M-VAVE FM1 | v0.3.0-alpha | FM-1_903

The same released firmware, with an English instruction and campaign package.

Start here

RED MASS VAMPIRE OS is custom open-source firmware for the M-VAVE FM1, built on RED MASS VAMPIRE OS. This English edition contains the same v0.3.0-alpha device firmware as the earlier release: package identity FM-1_903. The video and guides do not install a new firmware revision.

What this release gives you

- Six independent tracks, up to 64 steps each. 5 + 1 combines five synth tracks with track 6 for drums; FLEX lets you use six instrument tracks.
- Track 6: a 16-pad SMPKIT drum engine, two visible banks of eight lanes, ghost/normal/accent hits, one-shot playback, velocity and hi-hat choke.
- 128 built-in TRAP VAMPIRE sounds across ten engines, including 16 FM6 patches; the original 64 VAMPIRE sounds remain available.
- Probability, ratchets x1-x4, 14 step conditions, FILL, Euclid, pattern transforms, A/B scene morphing and performance effects.
- 44.1 kHz / 16-bit stereo USB audio, USB/TRS MIDI input, USB MIDI output, per-track output routing, editor and complete backup.

> This remains an alpha release. Host tests and a target build passed; independent end-to-end hardware qualification is still pending. Save a backup before installing.

Make your first beat

1. Press HOME briefly. Turn ALGORITHM to select track 6. The selected track number appears in the header.
2. Hold HOME for about one second to open MENU. Use ALGORITHM for the GROOVE tab, PRESETS for LAYOUT, and a small knob or OCT+ / OCT- to set 5 + 1. Briefly press HOME to close.
3. If track 6 is not on SMPKIT: tap SAVE until PRESETS appears, turn KNOB 2 to SMPKIT, and use KNOB 1 or PRESETS to select REDMASS KIT. Return with HOME.
4. Tap SEQ briefly, repeating until STEP appears. A drum grid replaces the melodic piano roll.
5. Turn KNOB 2 (LANE) to KD. Tap white keys 1, 7 and 11. Choose SD, then tap white keys 5 and 13. Choose CH, then tap every other white key.
6. Press PLAY. Turn SELECT to change BPM. If BPM LOCKED appears, hold GLO while turning SELECT, or disable BPM LOCK in MENU > CONTROL.
7. Turn ALGORITHM to track 1, press HOME, select a sound with PRESETS and play. You can switch tracks while the pattern continues.
8. Stop with PLAY and save the complete project as described in the saving chapter.

> On the drum STEP screen, white keys are step switches, not the 16 drum performance keys. Press the same white key again to remove that drum hit. Outside the grid, SMPKIT can be played from the white keys.

A useful habit

Choose a track first, then choose a page. The four small knobs edit the four labels visible above the graph. Page buttons cycle through related pages when tapped again.

Install from a computer

Before the update

- Export your current projects, user presets, samples and settings. Use a complete backup when your installed firmware supports it; stock firmware may require its own export method.
- Keep a verified copy of your official M-VAVE firmware for returning to stock. Official firmware is not included in this pack.
- Use a USB data cable connected directly to the computer. Power the FM1 on and close DAWs, MIDI utilities and other browser tabs using its MIDI ports.
- Extract this entire ZIP first. Run commands from its top-level folder, not from inside the compressed archive.

Windows terminal method

Install Python 3.12 if it is not already installed. Open Command Prompt in the extracted folder, then run:

py -3.12 -m pip install mido python-rtmidi
py -3.12 source\tools\fm1_install.py --info
py -3.12 source\tools\fm1_install.py firmware\RED-MASS-VAMPIRE-OS-v0.3.0-alpha-FM1.fwsc

Check that the reported model is FM-1. Answer y when the installer asks. Do not remove USB or interrupt power while it writes. The expected completion message is: done: the FM-1 runs FM-1_903.

macOS and Linux

Use python3 instead of py -3.12, and forward slashes in paths. Install mido and python-rtmidi in a virtual environment if your system Python requires it.

python3 -m pip install mido python-rtmidi
python3 source/tools/fm1_install.py --info
python3 source/tools/fm1_install.py firmware/RED-MASS-VAMPIRE-OS-v0.3.0-alpha-FM1.fwsc

> If the model or package check fails, stop and resolve the mismatch. Do not use --force to bypass it. An interrupted update should be resumed with the same verified package using the supported installer.

Browser tools & backups

A ready-to-serve local web folder is included. The installer already points to this exact RED MASS VAMPIRE OS package, and the editor supports the new drum and condition fields. Use Chrome or Edge with Web MIDI and SysEx permission.

Start the local tools

py -3.12 -m http.server 8765 --directory web-tools

On macOS/Linux replace py -3.12 with python3. Leave that terminal running. Open the following local addresses in Chrome or Edge:

| Tool | Local address |
| --- | --- |
| Installer | http://localhost:8765/webapp/installer/ |
| Editor | http://localhost:8765/webapp/editor/ |

Install in the browser

1. Connect the powered FM1 by USB, open the local installer, and allow MIDI/SysEx access.
2. Check that the page describes RED MASS VAMPIRE OS v0.3.0-alpha, then use Install. Keep power and USB connected until completion.
3. The success identity is FM-1_903. Disconnect other applications from the MIDI ports if the device cannot be found.

Save and restore a complete backup

1. Open the editor and choose Connect. Select the real FM1; do not select Mock device.
2. Open Projects and use Save backup. Keep the downloaded JSON on your computer. It includes current music, A-D projects, user sounds, user samples and settings when the running firmware supports complete backup.
3. To restore, use Restore backup and select that archive. Review the confirmation: restoration replaces current music, saved data, samples and settings. Keep USB connected until it finishes.

> A browser mock is a demonstration, not a connected instrument. Only trust changes to the FM1 after connecting the real MIDI device. Older firmware cannot read new FUN9 projects.

The panel, explained

| Control | What it does |
| --- | --- |
| ALGORITHM | Select track 1-4 on ordinary pages. In MENU: select a tab. |
| SELECT | Global tempo. With BPM LOCK ON: hold GLO while turning it. |
| PRESETS | Select sounds on HOME and most sound pages. On STEP/CHANCE: move the step cursor. In MENU: select a row. |
| KNOB 1-4 | Edit the four labels shown on the screen. Action labels are selected by turning a knob right, then performed with OCT+. |
| HOME tap / hold | Tap: return HOME or close MENU. Hold about 0.7 seconds: open MENU. |
| SEQ tap / hold | Tap: cycle sequencer pages. Hold: open SONG. |
| PLAY | Start/stop playback. In SONG: play the chain. |
| REC tap / hold | Tap: arm/disarm live recording. On STEP, PATTERN or CHANCE, hold: ask to clear this track sequence. |
| SAVE tap / hold | Tap: open save pages. Hold about 0.7 seconds: one-level UNDO/REDO. |
| OCT+ / OCT- | Normally octave up/down; in dialogs, confirm/cancel; in MENU, increase/decrease the selected value. |
| FX / GLO / SCL / EDIT hold | Open a quick layer. Double-tap to keep it open; tap its button again to close. |

Unlock BPM

Hold HOME > ALGORITHM to CONTROL > PRESETS to BPM LOCK > OCT- to OFF > HOME. SELECT now changes BPM normally. To change tempo without removing the lock, hold GLO and turn SELECT.

Undo and redo

Hold SAVE about one second. Hold it again to redo. This covers sound/pattern loads, engine changes, sequence clearing and the supported pattern transforms. Continuous preset browsing returns to the sound before the browsing session. Ordinary individual note edits and normal knob changes do not have a universal undo history. Project loading is not undoable.

> If STOP TO UNDO appears, stop the song chain with PLAY and retry. Close MENU before using SAVE undo. OCT- in a SET layer restores the values that layer captured when it opened.

Edit drum steps

1. Select track 6 with ALGORITHM and use SMPKIT / REDMASS KIT.
2. Tap SEQ until STEP appears. Choose the drum with KNOB 2 (LANE).
3. The sixteen white keys are steps 1-16 of the current page. Tap to add a hit; tap again to remove it. This only changes the selected drum lane.
4. To layer sounds, choose another LANE and press the same white key. The step can contain several drum hits.
5. KNOB 1 (STEP) moves the cursor. KNOB 3 (HIT): clockwise adds the selected hit; counterclockwise removes it.
6. KNOB 4 (VEL) steps through GHOST, NORM and ACC for the chosen pad at the cursor. Changing VEL creates the hit if it is empty.

Banks and pages

KNOB 2 chooses all 16 pads and switches the visible lane bank automatically. The first eight black keys select/play lanes in the visible bank. The ninth black key held with a white key toggles that hit's accent. The last two black keys move to the previous/next 16-step page; KNOB 1 can also cross page boundaries.

Tap SEQ to PATTERN. KNOB 1 sets LEN up to 64. The other knobs set division, swing and gate. Steps beyond LEN cannot be edited with white keys.

> For a single drum hit, use its white key or HIT. EDIT on STEP clears the entire cursor step, including other drum sounds, and advances the cursor. Hold REC only when you intend to clear the whole track sequence; OCT+ confirms and OCT- cancels.

Meet the drum pads

SMPKIT is a sample drum engine. REDMASS KIT is the built-in 16-pad set. Outside the drum STEP grid, white keys play these pads from left to right. Track 6 owns the full 16-pad grid and the upper eight/ghost persistence fields.

| Pad | Label | Sound / role | MIDI note |
| --- | --- | --- | --- |
| 1 | KD | Kick | 36 |
| 2 | SD | Snare | 38 |
| 3 | CL | Clap | 39 |
| 4 | CH | Closed hat | 42 |
| 5 | OH | Open hat | 46 |
| 6 | TM | Tom | 45 |
| 7 | RM | Rim / side stick | 37 |
| 8 | CB | Cowbell | 56 |
| 9 | LT | Low tom | 41 |
| 10 | MT | Mid tom | 43 |
| 11 | HT | High tom | 47 |
| 12 | CR | Crash | 49 |
| 13 | RD | Ride | 51 |
| 14 | TA | Tambourine | 54 |
| 15 | SH | Shaker | 70 |
| 16 | CU | Custom percussion | 75 |

Playing and recording

On the drum grid the first eight black keys play/select the visible lanes. With REC armed, they record into the running pattern; white keys still toggle steps. Outside the grid, play the white pad keys or send the GM notes above from MIDI.

Open and closed hats share a choke relationship: a closed hat cuts the open hat. Hits use one-shot playback with velocity. The shared voice budget may steal older voices during dense simultaneous hits.

> USER KIT 1-3 use the same sample storage as SAMPLE, GRAIN and SLICE. An empty user kit is silent. Importing a kit replaces that user sample slot; see the custom kits chapter.

Make loops evolve

Select the track and tap SEQ until CHANCE appears. KNOB 1 selects the step; KNOB 2 sets CHANCE (0-100%); KNOB 3 sets RATCH (x1-x4); KNOB 4 sets COND.

| Condition | When it plays |
| --- | --- |
| ALL | Every eligible pass, subject to chance. |
| FIRST / LATER | Only the first pass / only later passes. |
| 1:2 / 2:2 | The selected pass in each two-pass cycle. |
| 1:3 / 2:3 / 3:3 | The selected pass in each three-pass cycle. |
| 1:4 / 2:4 / 3:4 / 4:4 | The selected pass in each four-pass cycle. |
| FILL / !FILL | While FILL is active / while FILL is inactive. |

Perform a fill

Hold FX and the far-right white key G5 to activate FILL temporarily. Release it to leave the fill. For a persistent fill, use MENU > GROOVE > FILL. Set special hits to FILL and ordinary alternatives to !FILL.

Conditions are evaluated once for the whole step; all ratchet repeats follow that decision. On drums these are step-wide controls, not an independent condition for every pad at the same step.

> A useful starting point: CHANCE 75%, RATCH x2 on a hat step, COND 4:4 for an ending variation. Save a full project to preserve new conditions and drum fields.

Record notes & automation

Recording start modes

Select the destination track. Hold HOME > GROOVE > REC START and choose NOW, NOTE or 1 BAR. Close MENU. With internal clock, NOW starts immediately, NOTE waits for the first played note, and 1 BAR gives one bar of count-in. These start behaviors are for internal clock; an external clock follows its source.

1. Set the pattern length on SEQ > PATTERN. Use PLAY to stop first if you want to start a fresh take.
2. Tap REC to arm the selected track. Arming while stopped starts the transport using the chosen start mode. When playback is already running, REC arms the track for the current loop.
3. Play keys or MIDI. The notes are added into the loop. On the drum grid use lane keys for live entry; white keys keep their step-switch function.
4. Tap REC again to disarm. Press PLAY to stop. Save the project.

Melodic step entry

For a synth track, open SEQ > STEP with recording disarmed. Play the note or chord for the cursor step; keys held together form a chord, up to four stored notes. Releasing them advances the cursor. The screen's four knobs edit the step's note/time/velocity/flags according to their labels. EDIT clears the entire cursor step.

Automation

Record eligible knob moves while recording is armed and the sequencer runs. The automation icon marks parameters it drives. On SEQ > AUTOMATION, KNOB 1 enables/disables playback; select the clear action with KNOB 4 and OCT+ when you need to remove track automation. The device may ask for confirmation.

> Scene morphing requires recording disarmed and active recorded automation on that track disabled. STOP TO EDIT / STOP TO RECORD indicates a running song chain; stop the chain before the requested edit.

Explore and save sounds

Browse the factory sounds

Press HOME and turn PRESETS. The factory list runs across engines and then used user slots. Or tap SAVE until PRESETS: KNOB 1 / PRESETS browses sounds, KNOB 2 chooses an engine, KNOB 3 marks a favorite and KNOB 4 filters favorites. ALGORITHM still selects the destination track.

The 128 TRAP VAMPIRE sounds are already built into this firmware. The original 64 VAMPIRE sounds are also retained. JSON files in bank are editable library exports, not a required firmware installation step. Sound loads keep the current sequence; hold SAVE to undo a load.

Import the English pack banks in the editor

1. Connect the real FM1 in the local editor. Open Library and choose Import.
2. Select bank/RED-MASS-Trap-Vampire-128.json, or a 32-sound A/B/C/D subset. Library storage on the computer is separate from the FM1's 32 user preset slots.
3. Audition a library sound on the selected track. Copy/store only the sounds you want in user slots; importing 128 library entries does not create 128 device user slots.

Save an edited sound on the instrument

Stop playback, tap SAVE to USER, choose a slot with KNOB 1, turn the SAVE action knob (KNOB 4) right, then press OCT+. A used slot asks before overwrite. Enter the name on the on-screen keyboard and press OCT+ to write; OCT- cancels.

> A user preset stores the sound, its FM6 patch where relevant and the older 16-step sketch. Save a complete project for full 64-step patterns, all 16 pads, ghost hits, conditions and automation.

Build patterns in new ways

Euclidean rhythms

1. Select a track and stop playback. Tap SEQ until EUCLID appears.
2. KNOB 1 sets LEN, KNOB 2 PULSE, KNOB 3 ROT and KNOB 4 LANE. Pulses distribute hits over the selected length; rotation moves that rhythm around the loop.
3. Press OCT+ to apply. If steps or automation already exist, review the replacement prompt and press OCT+ again to confirm, or OCT- to cancel.
4. Audition with PLAY. Hold SAVE to undo/redo the applied rhythm.

Pattern transformations

Tap SEQ until TRANSFORM. KNOB 1 chooses OP, KNOB 2 AMT and KNOB 3 SEED. Apply with OCT+ while stopped; confirm replacement when asked. The operations include rotation, reversal, transposition, duplication, humanization and shuffle. Related automation moves with the steps.

| Operation | Use it for |
| --- | --- |
| ROTATE | Move the pattern around the loop. |
| REVERSE | Read the pattern in the opposite order. |
| TRANSPOSE | Shift melodic pitches. Drum hits remain their pad identities. |
| DUPLICATE | Repeat material into a longer pattern, within the 64-step limit. |
| HUMANIZE | Create controlled variation using the amount/seed. |
| SHUFFLE | Reorder steps and their associated locks. |

> The amount and seed shown on these tools are working values in RAM. The resulting steps and automation are saved in the project. These actions have one-level SAVE undo/redo; normal step-by-step edits do not.

Morph and perform

Capture A and B

1. Choose a track and prepare sound A. Tap FX until SCENES. Turn KNOB 1 right to select A, then press OCT+ to capture.
2. Edit the sound using the same engine. Return to SCENES, turn KNOB 2 right to select B, then OCT+ to capture.
3. Turn KNOB 3 (MORPH) from 0 to 100. Eligible continuous parameters blend between A and B.
4. KNOB 4 selects RESET; OCT+ returns to A.

> A/B captures live in RAM and are not stored as reusable performance scenes. Save the resulting current sound/project if you want to keep it. Both scenes must use the same engine. Disarm REC and turn recorded automation off before morphing.

Perform effects

Hold FX to use the on-screen performance map: repeat, reverse, filters, tape stop, freeze, harmonizer and track mutes. Double-tap FX to keep the map open. MENU > CONTROL > FX LATCH makes effects toggle. With latch enabled, FX + OCT- turns latched effects/macros off.

Master groove effects

Tap FX until GROOVE FX. KNOB 1 DUST adds master texture; KNOB 2 DUCK ducks synth tracks from the kick. Zero disables each effect. Imported older projects start these new effects at zero.

The regular FX pages retain track distortion/SLICER and chorus, delay and reverb sends. The shared voice and effect budgets still apply.

Shape the instrument

Choose an engine and edit it

Select the destination track with ALGORITHM. In SAVE > PRESETS, KNOB 2 chooses the engine. Press EDIT to cycle its parameter pages; KNOB 1-4 follow the visible labels. HOME exposes that engine's four main controls. The available synthesis includes virtual analog, six-operator FM, phase distortion, chiptune, multisamples, formants, three oscillators, organ, granular, physical models, noise, slicing, the VAMPIRE sources and drum engines.

Envelope, LFO and modulation

ENV opens envelope pages. LFO opens modulation pages and the four-slot modulation matrix: on MOD, KNOB 1 selects a slot, KNOB 2 source, KNOB 3 destination and KNOB 4 amount. The labels and graph show the current destination. Use the web editor for the complete FM6 operator/algorithm patch, including SysEx import.

Scale, chords and arpeggiator

SCL selects root, scale and keyboard quantization. WHITE maps the scale to white keys; SNAP rounds chromatic input; SEQ also quantizes sequenced melodic playback. Drum pads retain their note map. Chord settings let one key generate an in-scale chord. ARP cycles mode, rate, octave, gate, probability and ordering pages. OCT+ / OCT- shift keyboard octave on ordinary pages.

Slice an audio source

Choose SLICE and a built-in or user sample. EDIT > SLICES lets you choose and move slice markers with KNOB 1/2 and use the displayed split/join actions with OCT+. A key selects/plays its slice. Use the editor for importing and trimming audio. SLICE shares USR1-3 with the other sample engines and user drum kits.

Choose the visual character

Hold HOME > DISPLAY. COLOR includes VAMP and CRYPT. WAVE selects LIVE, GLOW, MIRROR or BARS. STYLE, LARGE, ANIM and LEDS change screen style, label size, animations and panel lighting. ALGORITHM changes tabs; PRESETS changes rows; small knobs or OCT+/- change values. HOME closes and saves settings.

Save projects and songs

Save the current project

1. Stop playback with PLAY. Tap SAVE repeatedly until PROJECT.
2. KNOB 1 selects one of four project slots, A-D.
3. Turn KNOB 4 right to choose SAVE, then press OCT+. Confirm overwrite with OCT+ if the slot is used.
4. Name the project on the on-screen keyboard. Press OCT+ to write, or OCT- to cancel.

The new FUN9 project format preserves the full drum grid, ghost hits, conditions, per-track routing, FM6 patches, automation and project name. Saved projects live on the instrument; complete backup archives live on the computer.

Load a project

On PROJECT choose the slot with KNOB 1, select LOAD with KNOB 3, then OCT+. Loading replaces the current project and is not covered by SAVE undo. Save your work or export a backup first.

Build a song chain

Hold SEQ to open SONG. KNOB 1 chooses a row, KNOB 2 its project slot A-D, and KNOB 3 its repeat count. Up to 16 rows can chain the four saved projects. Press PLAY on SONG to play the chain. Stop the chain before editing, recording, clearing or applying pattern tools.

Compatibility and rollback

This release reads and migrates older FUN1-FUN8 projects. Older releases do not read FUN9. Before returning to older firmware, retain a backup compatible with that earlier release and export important new music. Official M-VAVE firmware cannot open RED MASS VAMPIRE OS/RED MASS VAMPIRE OS projects.

> A/B scene captures and tool working values are RAM-only. Save their resulting sound, steps and automation rather than relying on the temporary captures surviving a power cycle.

Connect the studio

Record the stereo master over USB

Connect the FM1 by USB and select its audio input in your DAW. It sends the stereo master at 44.1 kHz, 16-bit. This is a master stereo feed, not four separate audio outputs. MENU > AUDIO > USB LEVEL selects MASTER or FIXED. SPEAKER EQ also shapes the headphone/line and USB signal; use FLAT for a neutral recording.

If audio is missing on macOS 13-15, set MENU > SYSTEM > USB SERIAL to OFF, close MENU, and let the FM1 reconnect. USB MIDI, the editor and installer remain available.

Route each track

1. Select the track with ALGORITHM before opening MENU.
2. In GROOVE set TRACK OUT to AUDIO, MIDI or BOTH.
3. Set MIDI CH to AUTO or a fixed channel 1-16.
4. Set MIDI SEND to KEYS, SEQ+ARP or ALL, then HOME to close.

| AUTO channel | 5 + 1 | FLEX |
| --- | --- | --- |
| Track 1 | 1 | 1 |
| Track 2 | 2 | 2 |
| Track 3 | 3 | 3 |
| Track 6 | 10 | 4 |

In 5 + 1, incoming channels 1-3 address tracks 1-3, channel 10 addresses drums, and other channels address the selected track. FLEX also retains the base SYSTEM routing options. Match your controller/DAW channel to the intended track.

Clock and physical ports

Use GLO > GLOBAL > CLOCK to choose internal, USB or TRS clock. With external clock, the source determines playback timing. USB carries MIDI output. The TRS MIDI jack is input only in this firmware. Bluetooth is not implemented.

> When controlling an external synth through your splitter, use an appropriate USB MIDI host/interface for FM1 USB output. Do not connect its TRS MIDI input expecting MIDI output.

Build your own drum kit

Prepare 1-16 WAV files in the pad order from the map chapter. The tool reads each WAV, converts it to mono 22,050 Hz and encodes IMA ADPCM. Each slot is 80 KiB including metadata: roughly 7.4 seconds combined mono audio after encoding, not 7.4 seconds per pad. Trim long tails before packing.

Build files without an instrument

py -3.12 source\tools\fm1_kit.py build MYKIT mykit kick.wav snare.wav clap.wav

This creates mykit.hdr, mykit.bin, mykit.fsmp and a mapping mykit.json in the current folder. With fewer than 16 WAV files, remaining pads are silent. For full pad placement supply a WAV for every preceding position.

Upload directly to slot 1

py -3.12 -m pip install mido python-rtmidi
py -3.12 source\tools\fm1_kit.py load 1 MYKIT kick.wav snare.wav clap.wav

Connect the FM1, close other MIDI applications, stop playback and run the command. Slot numbers are 1, 2 or 3. After a successful upload, select SMPKIT > USER KIT 1 (or the slot you used). On macOS/Linux use python3 and forward slashes.

Important storage behavior

- USER KIT 1-3 are the same USR1-3 sample slots used by SAMPLE, GRAIN and SLICE. Loading a kit replaces that slot's previous audio for every engine that uses it.
- The .fsmp file is a packed sample-slot file; it is not an OS update and must not be sent to the firmware installer.
- mykit.json from this tool is a pad map, not a preset bank or complete backup. Do not import it as a sound library.
- An empty USER KIT is intentionally silent. Back up sample slots before replacing or erasing them.

> If the tool reports too long, shorten or trim the WAVs. It will not silently create an oversized slot. Keep the complete backup JSON if you need to restore the previous kit.

Troubleshooting

| What you see | What to do |
| --- | --- |
| BPM LOCKED | MENU > CONTROL > BPM LOCK > OFF; or hold GLO and turn SELECT. |
| NOTHING TO UNDO | No supported load/clear/transform snapshot is available. Individual note edits do not build a universal undo history. |
| STOP TO SAVE / APPLY | Stop playback before flash writes or pattern generation. |
| STOP TO EDIT / RECORD / UNDO | Stop the running or armed song chain with PLAY, then retry. |
| CAPTURE A AND B | Capture both sides on FX > SCENES before turning MORPH. |
| SCENES NEED SAME ENG | Use the same engine for A and B and the current track. |
| DISARM REC FIRST / TURN AUTOMATION OFF | Disarm live recording and disable that track's recorded automation before scene morphing. |
| USER KIT is silent | Upload a kit to the matching USR slot. Unused pads in a partial kit stay silent. |
| White keys do not play drums | On drum STEP they toggle steps. Return HOME to play the pad map. |
| Cannot find FM1 in browser | Use Chrome/Edge, a USB data cable, real device mode and MIDI/SysEx permission. Close other MIDI users. |
| USB audio missing on older macOS | MENU > SYSTEM > USB SERIAL > OFF; close MENU and allow USB reconnect. |
| Older firmware refuses project | FUN9 is new. Restore a compatible older backup or return to v0.3 before reading it. |

If panel labels and actions disagree

Use MENU > SYSTEM > CALIBRATION and follow the on-screen label prompts. Hardware calibration can also be entered by holding OCT- and OCT+ while powering on. Calibration changes physical button/encoder mapping; use it only when you actually need to correct the mapping.

If an update stops

Keep the verified package and the installer log. Reopen the supported installer and resume the interrupted update. Never substitute a package for another model or use --force as a general troubleshooting step.

Limits, credits & evidence

What is implemented

Six tracks; eight shared sounding voices. Individual engine limits still apply: PHYS 3, VAMPIRE/VOICE 4 and FM6 6. Sixteen pad identities do not mean sixteen simultaneous voices. There is one built-in sample kit, three user sample kits, and the retained synthesized DRUM kits.

This release implements selected Groove/Sloop-style workflows on RED MASS VAMPIRE OS/RED MASS VAMPIRE OS. It is not a binary merge of those firmware products. It does not provide eight Groove tracks, eleven persistent performance scenes, 37 Sloop factory kits, Bluetooth, TRS MIDI output or the closed M-VAVE stock sound engine.

Validation

The complete host suite passed. Target image, RAM and pool checks passed. New tests cover 16 pads, all 14 conditions, ghost hits, count-in modes, MIDI routing, FUN9 persistence and real editor protocol replies. Logs and the release manifest are in validation. Those checks do not establish full real-device boot, audio, USB and flash reliability.

Firmware: RED-MASS-VAMPIRE-OS-v0.3.0-alpha-FM1.fwsc
Identity: FM-1_903
Bytes: 609849
SHA256:
181f226587744e339e7e3cfa551803253733eacfda972e34e0782cfc829bfc57

Credits and redistribution

RED MASS VAMPIRE OS is based on RED MASS VAMPIRE OS by Leo Kuroshita / Hugelton Instruments. Firmware is GPL-3.0-only; corresponding source, LICENSE, LICENSING.md and third-party notices are included. Inter Tight and Pirata One are used in the campaign artwork with their font notices. This is an independent project for the M-VAVE FM1, not an official M-VAVE release.

About the Reels

The 40.8-second vertical video shows UI pixels rendered by the released C firmware, not a filmed physical FM1. The original 100 BPM instrumental is arranged from the supplied native DSP preset and SMPKIT audio previews, with level balancing and pitch changes. It is a produced soundtrack rather than one live hardware take. No AI voiceover is included. Editable text, timings, soundtrack, cover and source scripts accompany it.

