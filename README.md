# TRoss 606

606-style drum machine with a 16-step sequencer (AU / VST3 / LV2 / Standalone) by Ticco Ross, 2026.

## Using it

| Control | What it does |
|---|---|
| 8 drum channels (silver panel) | Tune / Decay / Level, plus Attack (BD), Snappy (SD), Noise (CP), Tone (CY). **M** mutes, **S** solos, **OWN OUT** sends the drum to its own output. Hover or drag a knob to see its value; double-click resets it |
| Master | Accent amount for accented steps and hits above velocity 100; Volume. The display shows the playing pattern (and the queued one) |
| Step pads | 9 rows (ACC + 8 drums) × 16 steps, coloured in groups of four. Click to toggle, or drag along a row to paint. The LEDs above follow the playing step |
| RUN | Sequencer on/off |
| LENGTH | Pattern length, 1–16 steps: drag the red read-out up/down (steps past the length are dimmed) |
| CLEAR | Clears the pattern that's playing |
| REC | While the host is playing, keyboard hits are written into the nearest step. Velocity above 100 also sets the accent. On by default |
| PATTERN 1–12 | 12 pattern slots. While playing, a new slot waits until the current pattern reaches step 1. While stopped, it switches immediately. The LED shows which slot is playing; a blinking LED shows the slot that's queued |

**Keyboard map (Ableton note names, C1 = MIDI 36):** BD C1/B0 · SD D1/E1 · LT A1/F1/G1 · HT D2/B1/C2 ·
CP D#1 · CH F#1/G#1 · OH A#1 · CY C#2/A2/D#2/E2/G2.

Velocity: velocities up to 100 scale the level, and anything above 100
also adds the Accent.

All 12 patterns are saved with your Live set, along with every knob. The window can be resized
from its corner.

## Outputs

| Output | Carries |
|---|---|
| Main (stereo) | Every drum whose OWN OUT is off, after Volume (the same mono mix on both channels) |
| BD, SD, LT, HT, CP, CH, OH, CY (mono) | That drum alone while its OWN OUT is on: after its Level, Mute/Solo and accents, **not** affected by Volume |

All drums start on the main output. Turn on a drum's **OWN OUT** and it leaves the main mix and plays on
its own output instead. If the host has that output switched off, the drum stays in the main mix.

**Ableton Live:** make an audio track for each separate drum, set **Audio From** to the TRoss 606 track,
pick the output (for example `BD`) in the second menu, and set Monitor to **In**.

The standalone app has only the main output, so it doesn't show the OWN OUT switches.

The **standalone app** has no host transport, so it shows its own **PLAY** button and BPM read-out.

Defaults: Bass Drum Decay 27.2 %, Bass Drum Attack 21.3 %.
The build's version number is shown at the top right of the panel.

## Build on your Mac (macOS 12+)

1. **Compiler:** run `xcode-select --install` in Terminal.
2. **CMake 3.22 or newer:** download it from https://cmake.org/download/, then run
   `sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install`
3. **Build** from inside this folder:

   ```sh
   cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
   cmake --build build --config Release -j8
   ```

   The plug-ins are copied to `~/Library/Audio/Plug-Ins/Components/TRoss 606.component` and
   `~/Library/Audio/Plug-Ins/VST3/TRoss 606.vst3`.
4. **Validate the AU (optional):** `auval -v aumu T606 Tcro`
5. **Ableton Live:** go to Settings → Plug-Ins, turn on "Use Audio Units v2", and rescan.
   The plug-in is under Plug-Ins → Audio Units → Ticco Ross → TRoss 606.

To build the installer, run `./tools/make_pkg.sh` after the build. It creates `dist/TRoss606-<version>.pkg`.
Signing and notarizing work the same way as for Polytwelve (`DEV_ID_APP`, `DEV_ID_INSTALLER`, `NOTARY_PROFILE`).

## Automatic builds (GitHub Actions)

Each push to `main` builds everything on GitHub's macOS, Windows and Linux machines and runs the
tests on Linux. The files appear under the run's **Artifacts** on the **Actions** tab:

| Platform | Files |
|---|---|
| macOS | `TRoss606-<v>-macOS.pkg` (AU + VST3 + app), plus AU and VST3 zips |
| Windows | `TRoss606-<v>-Windows-Setup.exe` (VST3 + app), plus a VST3 zip |
| Linux | `TRoss606-<v>-Linux-x86_64.tar.gz` (VST3 + LV2 + app, with `install.sh`) |

**Publishing a release:** raise the version in `CMakeLists.txt`, commit, then
`git tag v0.2.0 && git push --tags`. The installers get attached to a new GitHub release.

The installers aren't signed. The first time you open one, macOS shows a Gatekeeper warning
(Privacy & Security → Open Anyway) and Windows shows SmartScreen (More info → Run anyway).

## Tests

```sh
g++ -O2 -std=c++17 -ISource tools/render_test.cpp -o render_test && ./render_test   # engine only, no JUCE
cmake -B build -DT606_BUILD_TESTS=ON && cmake --build build --target TRoss606Test     # full plug-in
```

The plug-in test covers host sync, pattern queueing, LEN, keyboard recording, and state save/restore. It also takes a snapshot of the editor.
