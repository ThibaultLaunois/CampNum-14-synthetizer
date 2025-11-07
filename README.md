# Piano Synthesizer

This is a simple piano synthesizer application built using openFrameworks. It allows users to play piano notes using the keyboard and visualize the waveform, the frequencies and keys being pressed.

## 0. Background

### a. Sampling rate

Computers cannot store continuous sound waves directly. Instead, they sample the sound wave at discrete intervals to create a digital representation. The sampling rate determines how many samples are taken per second.

According to the Nyquist-Shannon sampling theorem, to accurately reproduce a sound wave, it must be sampled at least twice the frequency of the highest frequency present in the sound. For example, to reproduce a sound wave with a maximum frequency of 20 kHz, a sampling rate of at least 40 kHz is required.

The human hearing range typically spans from 20 Hz to 20 kHz. Therefore, to ensure that all audible frequencies can be accurately reproduced, a sampling rate of at least 40 kHz is necessary. In this application, we use a standard sampling rate of 44.1 kHz, which is commonly used in audio CDs.

44.1 kHz = 44,100 samples per second. It means we get 1 sample every 1/44100 seconds ≈ 0.0000227 seconds = 22.7 microseconds, which is very fast. In fact, this is so fast that the human ear cannot distinguish individual samples, resulting in a smooth and continuous sound wave.

### b.Sound synthesis

A sound is a wave that propagates through air. This wave can be represented mathematically as a sine wave. The sine wave is defined by its frequency, amplitude, and phase. The frequency determines the pitch of the sound, the amplitude determines the loudness, and the phase determines the starting point of the wave.

In order to generate sound digitally, we need to create at high frequency (44.1 kHz) a series of samples that represent the sound wave. This is done by calculating the value of the sine wave at each sample point based on the desired frequency and amplitude.

**Example:** Let's say we want to generate a sound wave with a frequency of 440 Hz (the musical note A4, also known as La). How do we generate a sine wave at 440 Hz when the sampling rate is 44.1 kHz?

The answer is we use a **phase** variable that advances through the sine wave cycle:

- The phase represents where we are in the sine wave (from 0 to 2π radians)
- At each sample, we calculate `sin(phase)` to get the audio value
- We increment the phase by a **phase adder** at each sample

The phase adder is calculated as:

```
phaseAdder = (frequency / sampleRate) × 2π
           = (440 / 44100) × 2π
           ≈ 0.0627 radians per sample
```

This means:

- After 44,100 samples (1 second), the phase will have advanced by 440 complete cycles (440 × 2π radians)
- This produces exactly 440 Hz!

### c. Polyphonic synthesis

**Monophonic** synthesis means playing one note at a time. **Polyphonic** synthesis means playing multiple notes simultaneously.

Our synthesizer uses **additive synthesis** for polyphony:

1. We create 10 independent sound generators called "voices"
2. Each voice generates its own sine wave at its own frequency
3. At each sample, we **add together** all active voices
4. The final sum is sent to the sound card

This is like having 10 instruments playing at the same time - the final sound is the combination of all sounds.

### d. Musical frequencies

Musical notes correspond to specific frequencies. The standard reference pitch is A4 (the A above middle C), which is set at 440 Hz. Other notes are defined relative to this pitch using the formula:

$$
frequency = 440 × 2^{(n/12)}
$$

Where `n` is the **number of semitones away from A4**. For example:

- C4 (9 semitones below A4): $440 × 2^{(-9/12)} ≈ 261.63 Hz$
- G#4/Ab4 (1 semitone below A4): $440 × 2^{(-1/12)} ≈ 415.30 Hz$
- A4 (reference): $440 Hz$
- A#4/Bb4 (1 semitone above A4): $440 × 2^{(1/12)} ≈ 466.16 Hz$
- B4 (2 semitones above A4): $440 × 2^{(2/12)} ≈ 493.88 Hz$
- C5 (3 semitones above A4): $440 × 2^{(3/12)} ≈ 523.25 Hz$
- And so on...

## 1. Architecture

### Key Components

**Voice Structure:**

```cpp
struct Voice {
    bool active;        // Is this voice currently playing?
    float phase;        // Current position in the sine wave (0 to 2π)
    float phaseAdder;   // How much to advance phase per sample
    int keyIndex;       // Which piano key is this voice playing?
};
```

We have an array of 10 voices (because humans have 10 fingers max).

**Piano Keys:**
Each piano key is represented by a `PianoKey` structure containing:

- Rectangle position for drawing
- Frequency (calculated using: $440 × 2^{(semitones\_from\_A4 / 12)}$)
- Whether it's a black or white key
- Associated keyboard character
- Note name (e.g., "C4", "A#5")

## 2. How It Works

### a. When you press a key

1. `keyPressed()` is called with the keyboard character
2. Find the corresponding piano key and its frequency
3. Search for a free voice (`active == false`)
4. Activate that voice:
   - Set `active = true`
   - Set `phase = 0` (start at beginning of sine wave)
   - Calculate `phaseAdder = (frequency / 44100) × 2π`
   - Store which piano key this voice is playing

### b. In the audio thread (`audioOut()`)

Called ~86 times per second with 512 samples each time:

```
For each sample (512 iterations):
    sample = 0

    For each voice (0 to 9):
        If voice is active:
            voice.phase += voice.phaseAdder    // Advance through sine wave
            sample += sin(voice.phase)          // Add this voice's contribution

    sample /= sqrt(number_of_active_voices)    // Normalize
    sample *= volume

    Send sample to left and right audio channels
```

### c. When you release a key

1. `keyReleased()` is called
2. Find the voice with matching `keyIndex`
3. Set `active = false`
4. Audio thread stops processing this voice

### d. Visualization

- **Waveform display:** Shows the combined audio output in real-time
- **Piano keys:** White keys (major notes) drawn first, black keys (sharps/flats) drawn on top
- **Key highlighting:** Pressed keys turn blue

## 3. Piano Layout

The synthesizer displays 3 octaves (36 notes from C3 to B5):

- **White keys:** C, D, E, F, G, A, B (natural notes)
- **Black keys:** C#, D#, F#, G#, A# (sharps)

Keyboard mapping uses QWERTY keys: `awsedftgyhujkolp;'[]\zsxcfvgbnjmk,.`

## 4. Controls

- **Keyboard keys:** Play piano notes (see on-screen mapping)
- **+/=:** Increase volume
- **-/\_:** Decrease volume

## 5. Code Structure

**ofApp.h:**

- Declares the Voice structure
- Declares piano key data structures
- Declares audio and visual variables

**ofApp.cpp:**

- `setup()`: Initialize audio system, create piano keys
- `setupPianoKeys()`: Calculate frequencies and positions for all 36 keys
- `calculateFrequency()`: Convert semitone offset to Hz using musical formula
- `draw()`: Render waveform and piano keyboard
- `keyPressed()/keyReleased()`: Handle keyboard input
- `audioOut()`: Generate audio samples (runs on audio thread)
