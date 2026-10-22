ODE TO GSX — 0.2.1

An experimental standalone and VST3/AU C++ granular synthesizer using JUCE for desktop
hosting and DaisySP for oscillators. Inspired by Barry Truax’s GSX and the
WebAudio implementation by ijc:
https://ijc8.me/2020/09/06/emulating-the-gsx/

Features
- Up to 64 simultaneous sine, band-limited square and saw voices.
- Uniform random grain frequency/duration, inter-grain delay, and a linear
  25% attack / 50% sustain / 25% release envelope.
- Value/step controls: each RAMP interval adds step times INC.
- Reset and randomization of values, steps, or both.
- Downward ramps wrap at their lower bound, preserving overshoot.
  RAMP itself clamps to 1 ms. Upward ramps cap at their upper bound.
- Audio output selection, master level, and Start/Stop fades.

Voice allocation: square first, then saw, remaining voices sine.
Fractional voice-count accumulators are truncated to whole sounding voices.
Audio is mono duplicated to stereo. Phase continues between grains.
Randomized durations clamp to 1 ms; this is not the historical 8 ms limit.
Restarting repeats the seeded random sequence. DaisySP waveforms and the
sample-clock scheduler differ from browser rendering; this is not bit exact.

Reset restores the original demo defaults, clears steps, sets INC=1 and
output=20%, preserving transport and audio device. Randomize preserves output.
VALUES includes INC; STEP changes only the right-hand increments.

Build
Requires CMake 3.22+, a C++17 compiler, and separately obtained JUCE and DaisySP.
Tested locally with JUCE 8.0.12 on macOS. Dependencies are never downloaded.

cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DJUCE_PATH=/path/to/JUCE -DDAISYSP_PATH=/path/to/DaisySP
cmake --build build-release --parallel 4
ctest --test-dir build-release --output-on-failure

macOS app: build-release/OdeToGSX_artefacts/Release/Standalone/ODE TO GSX.app

Validation covers envelopes, extreme ranges, 64 voices at 44.1/48/96 kHz,
Stop, zero voices, invalid inputs, ramp timing, and lower-bound wraparound.
Daisy hardware performance has not been measured. This is a desktop app,
not an embedded firmware release. FM, sampling, harmonic pitch selection,
tendency masks and bounce ramps are not implemented in this baseline.

Attribution and rights
This is a functional recreation, not original GSX software or PDP-11/DMX
hardware emulation. The WebAudio source above informed the implementation.
No third-party firmware, samples, research PDFs or dependency sources are
included. JUCE and DaisySP retain their respective licenses. Project code is licensed under AGPLv3 (see LICENSE). This does not grant
rights to any original GSX software, recordings or other third-party material.

Downloads and build workflow
Precompiled preview releases: https://github.com/SnazzyBlanket/Ode_2_GSX/releases
See DISTRIBUTION.txt for supported platforms, installation and limitations.
Plugin.cpp and Plugin.h implement the current shared standalone/plugin host.
Main.cpp preserves the earlier standalone implementation for reference only.
Release workflows pin dependencies listed in DEPENDENCIES.txt, test both
platforms, and publish binaries plus corresponding source with dependencies.
Windows uses --config Release when building and -C Release when running ctest.
Configure macOS with -DCMAKE_OSX_ARCHITECTURES=arm64
and -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 for the Apple Silicon preview.
