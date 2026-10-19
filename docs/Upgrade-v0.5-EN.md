# RED MASS VAMPIRE OS v0.5.1-alpha — Upgrade controls

M-VAVE FM1 · v0.5.1-alpha · FM-1_906

This build adds all twelve selected tools and expands the existing song chain into a 32-row arranger. The instrument has six audio tracks and eight shared voices. The curated browser contains 256 ROM presets; the separate editable user bank still has 32 slots.

Developer & contributor: Karsei. This revision also replaces the original factory sample kit with NIGHT TRAP, adds RAGE TRAP at KIT index 4, preserves USER KIT indices 1–3, and unifies every firmware font at weight 700. The Windows application contains the complete editor and installer and runs without Python using installed Edge or Chrome. Physical FM1 and Windows runtime qualification are pending.

## Open the tools

Hold HOME to open MENU. Turn ALGORITHM to SYSTEM, turn PRESETS to TOOLBOX, then press OCT+. Turn KNOB 1 to select a tool and press OCT+ to open it. ALGORITHM selects the track on an ordinary tool page.

An action is usually selected by turning its labelled knob to the right, then performed with OCT+. A value knob edits immediately. OCT- cancels an action or returns. Most destructive pattern tools require stopped playback and disarmed REC. The screen explains a blocked action.

| Tool | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
| --- | --- | --- | --- | --- |
| COPY | TRACK / PAGE scope | Select COPY action | Select PASTE action | Page 1–4 |
| CTRL ALL | Common parameter | Amount −63…+63 | CAP: capture snapshot | RELOAD snapshot |
| LOCKS | Step | Parameter | Value | PUT / DELETE, then OCT+ |
| MACROS | Macro 1 | Macro 2 | Macro 3 | Macro 4 |
| MACRO SET | Macro 1–4 | Target 1–4 | Parameter | Signed depth |
| MELODY | Density % | Octave | Range 1–3 octaves | Seed; OCT+ generates |
| MUTATE | Amount 0–64 | Seed | APPLY action | UNDO action |
| BANK 256 | Category | Sound | Favorite on/off | Favorites-only on/off |
| MUTE GROUP | Group 1–8 | Six-track mask | CAP: capture | RECALL action |
| DRUM CHANCE | Pad 1–16 | Its chance % | All-pad chance % | APPLY all pads |
| MICRO | Step | Shift −7…+7 /32 | ZERO action | HUMAN amount; OCT+ applies |
| HISTORY | UNDO action | REDO action | Current history depth | CLEAR action |
| SONG ROW | Row 1–32 | Row length | Row tempo | Six-track mute mask |
| SONG SET | KEEP / PROJ sounds | Start row | LOOP: loop-from row | STOP / LOOP ending |
| SCENES | Capture A | Capture B | Morph 0–100 | Return to A |

MACRO SET, SONG ROW and SONG SET are supporting pages, not extra promises beyond the twelve requested features. SCENES is the existing morph tool with persistent captures.

## 1. Copy a track or a page

Select the source track, open COPY and choose scope. TRACK copies all 64 stored steps and LEN/DIV/SWING/GATE. PAGE copies the selected sixteen-step page. Turn KNOB 2 right and press OCT+.

Select the destination track. For PAGE, choose the destination page with KNOB 4. Turn KNOB 3 right and press OCT+. A page beyond the current length extends LEN to the end of that page. Notes, drum hits, chance, conditions, ratchets, microtiming and automation locks move together. Sound parameters stay on the destination track. Hold SAVE to swap the pattern paste back/forward.

The shared automation limit is 64 events. Paste refuses before changing the destination if it would exceed that limit. Engine-specific locks require the same engine. A sixteen-pad grid with upper-pad or ghost data requires track 6.

## 2. CTRL ALL and RELOAD

Choose a common parameter with KNOB 1: distortion, chorus, delay, reverb, attack, decay, sustain, release, pan or level. KNOB 2 offsets that parameter on all six tracks relative to their captured values. The first amount edit automatically captures a snapshot; CAP with KNOB 3/OCT+ explicitly captures a new one.

KNOB 4/OCT+ restores the touched common parameters exactly. This works during ordinary pattern playback with REC disarmed. Stop Song Mode and disable active track automation before changing the amount. The temporary CTRL ALL snapshot is session state; it is not a saved performance scene.

## 3. Manual step locks

On LOCKS, select step, parameter and value. Select PUT with KNOB 4 and press OCT+. The lock is stored in the existing motion pool and enabled for that track. DELETE removes only that step/parameter pair. Other locks remain.

There are 64 automation events shared by the six tracks. This is not a second independent lock bank. A project or complete backup preserves these events.

## 4. Four assignable macros per track

On MACRO SET, choose a macro and one of its four targets. Choose an eligible continuous parameter and a signed depth. Zero depth disables that target. Configuration captures the current parameter as its base and resets macro positions to zero. Disarm REC and stop playback while configuring.

On MACROS, the four knobs control positions 0–127. Positive and negative targets can work together, and multiple macros can affect the same parameter. Values clamp to the parameter's valid range. Returning positions to zero restores their captured bases. Loading a sound rebases assignments to that sound.

Macro movement is available during ordinary playback when REC is disarmed and active motion automation on that track is off. Song Mode blocks macro movement. Assignments, bases and positions are saved in full projects. Macro performance is not recorded as a separate macro automation lane.

## 5. Scale-safe melody generator

Set ROOT and SCALE on the selected synth track first. On MELODY set density, starting octave, range and seed, then press OCT+. The generator replaces the pattern up to the current LEN with notes from that scale and rests, and clears the replaced track's automation. The same settings and seed generate the same melody. Drum tracks are rejected.

Hold SAVE to swap the generated pattern back/forward. The seed and generation settings are temporary working controls; the resulting sequence is saved.

## 6. Timbre mutation

On MUTATE choose an amount and seed, select APPLY and press OCT+. Continuous timbre, envelope, modulation and effect values vary within their valid ranges. Track level, pan, routing, scale, sequence settings, transposition and tuning are protected; sound pitch controls and slot selectors are excluded.

UNDO with KNOB 4/OCT+ or held SAVE swaps the sound mutation back/forward. Mutation is relative to the current sound: the same seed yields the same result when applied to the same starting parameter values. It is not a sample generator or a replacement FM6 operator editor.

## 7. 256 categorized sounds and favorites

BANK 256 opens the curated browser. VAMP256 contains existing presets 001–128 and new presets 129–256. OLD128 and NEW128 isolate the two halves; other filters include PLUCK, PAD, LEAD, PIANO, BASS, KEYS, BELL, CHOIR, STRING, ORGAN, PERC and FX. ALL also exposes retained stock/legacy sounds for project compatibility.

KNOB 2 loads a sound without replacing the pattern. KNOB 3 marks it as a favorite; KNOB 4 filters favorites. Favorite references retain their original factory indices. VAMP256 is the default on a normal device boot; category selection is a session control.

The full computer library is bank/RED-MASS-VAMPIRE-256.json. Its optional A–H files each contain 32 sounds for the device's 32 editable slots. These are alternative sound-bank subsets, not eight simultaneous onboard banks. The A–H file letters are separate from Song Mode's project sources A–D. Built-in ROM presets do not need to be imported.

## 8. Eight mute groups

Choose GROUP 1–8. CAP with KNOB 3/OCT+ records the current six track mutes. MUTES with KNOB 2 edits the saved mask directly; M means muted and '-' means audible, from track 1 to 6. RECALL with KNOB 4/OCT+ applies it, including during playback. All eight masks are stored in projects.

Song rows also have their own mute masks. A row change applies its mask again, so a live mute-group recall can be temporary during an arrangement.

## 9. Per-pad drum probability

Select track 6 and SMPKIT. On DRUM CHANCE choose PAD with KNOB 1 and chance with KNOB 2. The setting applies to that pad throughout the pattern. It combines with each step's existing whole-step chance and condition. A step makes one per-pad decision; its ratchets repeat that decision, rather than re-rolling every hit.

Zero means never play that pad; 100 means always pass the pad decision. To set all sixteen pads, choose ALL with KNOB 3 and press OCT+ on APPLY. Full projects preserve pad chances. These are per-pad settings, not a separate probability value for every pad on every step.

## 10. Microtiming

On MICRO select a step and turn SHIFT. One unit is 1/32 of that step's duration, including swing. The range is −7…+7. Negative values play earlier; positive values later. The first early note at PLAY is clipped to time zero; later loop events can lead the nominal boundary.

ZERO/OCT+ puts the selected step on the grid. HUMAN with KNOB 4 chooses a maximum offset and OCT+ humanizes the pattern deterministically with the current seed. Conditions, pad hits and locks stay with their event. Ratchets compress into the available space when the following step is early, so all selected hits fit.

Single stopped step shifts use the sixteen-edit history; whole-pattern humanization uses pattern-load SAVE undo. Microtiming is saved in full projects and complete backups. The current browser step editor does not expose this new panel control.

## 11. Persistent A/B scene captures

On SCENES capture A, edit a sound within the same engine, then capture B. KNOB 3 morphs eligible continuous sound parameters and KNOB 4/OCT+ returns to A. Both captures must match the current engine. Disarm REC and disable active motion automation before morphing.

In v0.5 full projects and complete backups retain both captures, validity and morph position for each track. Saving just a user sound is not a substitute for saving the whole project. Switching between differently structured FM6 patches is not implemented as an operator-by-operator scene morph.

## 12. Sixteen-edit undo and explicit redo

Stopped note/drum step edits, velocity/chance/condition/ratchet edits, step clearing and single-step microtiming enter a sixteen-transaction history. REC must be disarmed. HISTORY shows the current depth. Select UNDO with KNOB 1 or REDO with KNOB 2, then press OCT+. A new edit after undo discards the redo branch. CLEAR/OCT+ drops the history.

Held SAVE undoes a step edit when this history contains entries. Sound/pattern loads, generators, mutation and paste clear step history and use the earlier one-level load swap instead; held SAVE again swaps that load forward. Project loads clear both histories. Manual lock edits, macro configuration and mute-group changes are not included in this step-only history. History itself is temporary and is not saved in projects.

## Expanded Song Mode

The previous OS already had a limited chain of four project patterns. This release extends it; it does not create 32 independent saved project slots.

1. Save section patterns as projects A, B, C and D. Each source contains all six tracks. They are the four reusable pattern/sound sources.
2. Hold SEQ for SONG, or cycle the SEQ pages to SONG. Set ROW with KNOB 1, source SLOT with KNOB 2 and REPS 1–64 with KNOB 3. Add up to 32 rows.
3. Tap SEQ to SONG ROW. Choose LEN: 0/PAT uses source track 1's natural cycle; 1–64 specifies track 1 clock steps per repeat. BPM 0/KEEP retains the current tempo; 20–300 sets it on entering the row. MUTES selects the row mask.
4. Tap SEQ to SONG SET. SOUNDS KEEP uses current instruments and source sequences; PROJECT recalls the source project sounds, FM6 patches, effect settings and pad chances. START selects the first row. LOOP selects the row to return to. END chooses STOP or LOOP. Out-of-range start/loop rows fall back to row 1.
5. Press PLAY. All tracks start together. REC is suspended while the arrangement runs. Edits and pattern tools that could invalidate a cached source are blocked. At STOP, the editable starting patterns, sounds, timing, mutes, tempo and record arms return.
6. Save the arrangement in a full project. Avoid overwriting a source A–D while you are still building the song unless the changed source is intended. A project may contain both a source pattern and the song row list.

Row mutes combine with saved-source mutes in PROJECT mode or starting mutes in KEEP mode. Unmute a source/current track if you want a row to make it audible. Row length follows track 1's division and swing; the other tracks can retain different divisions and lengths. Full Song Mode configuration is also available in the updated browser editor's Projects tab.

## The eight grand piano variations

These eight patches use the existing CC0 recorded grand piano multisamples. Their differences are native tone, saturation, envelopes and effects; they are not eight separately sampled piano libraries. All are polyphonic within the shared eight-voice budget.

| Number | Name | Character |
| --- | --- | --- |
| 249 | DREADGRND | Dark concert attack with a restrained room tail |
| 250 | BLACKIVRY | Muted, intimate attack and a compact decay |
| 251 | IRONHAMMR | Harder, driven hammer strike for aggressive parts |
| 252 | CRYPTROOM | Spacious piano with a long atmospheric room tail |
| 253 | BLOODGLAS | Brighter, wider piano with rhythmic echo |
| 254 | VELVETKEY | Softer, rounded envelope and width |
| 255 | FANGSTACC | Short staccato strikes for rapid rhythmic playing |
| 256 | GRAVECONC | Longer sustain and a broad cinematic tail |

Listen at preview/index.html. Each preview is a processed native C DSP render, encoded to MP3 for listening. Lossless piano examples are in audio/pianos. There is no additional post-render EQ, normalization or external reverb.

## Saving and compatibility

The new FUNB project core is linked to an A/B feature extension by checksum. A feature save commits first, then the project core; interrupted page-write tests retain a complete old or new project. A missing/damaged feature extension is rejected with PROJECT FEATURES LOST before replacing the current music.

Use the updated complete-backup tool to preserve current music, A–D projects, macro/scene/mute/chance/microtiming/arranger features, user sounds, user samples and settings together. Extensions restore before their project cores. Older 11-, 12- and 13-object backups remain readable. A new 18-object archive requires v0.5 feature support.

v0.5 reads older FUN1–FUNA projects and supplies default new features. Older firmware does not understand the FUNB extension. Keep a pre-upgrade backup for rollback. Do not copy only a project core from a new complete archive.

The final validation report states which checks passed. Physical FM1 boot, flash, USB and performance qualification remains pending; this is an alpha build, not a claimed hardware-tested release.
