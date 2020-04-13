//
//  MusicGeneration.h
//  MusicGB
//
//  Created by Erwin on 04/04/2020.
//  Copyright © 2020 Erwin. All rights reserved.
//

#ifndef MusicGeneration_h
#define MusicGeneration_h

#include <stdint.h>
#include "../../config/config.h"

namespace Gamebuino_Meta {

//--------------------------------------------------------------------------------------------------
// Tune Generation

constexpr uint16_t SAMPLERATE = 22050;

// Should be updated when changing SAMPLERATE as follows:
// 44100 -> 0, 22050 -> 1, 11025 -> 2
constexpr uint8_t SAMPLERATE_SHIFT = 1;

// Number of samples when sample rate is 11025 Hz
constexpr uint8_t SAMPLES_PER_TICK = 90;

typedef int16_t Sample;

constexpr int numNotes = 12;
enum class Note {
    A, As, B, C, Cs, D, Ds, E, F, Fs, G, Gs
};

enum class WaveForm : int8_t {
    TRIANGLE,
    TILTED_SAW,
    SAW,
    SQUARE,
    PULSE,
    ORGAN,
    NOISE,
    PHASER,
    NONE
};

enum class Effect : int8_t {
    NONE,
    SLIDE,
    VIBRATO,
    DROP,
    FADE_IN,
    FADE_OUT,
    ARPEGGIO,
    ARPEGGIO_FAST
};

struct NoteSpec {
    Note note;
    uint8_t oct; // [2..6]
    uint8_t vol; // [0..8]
    WaveForm wav;
    Effect fx;
};

const NoteSpec SILENCE = NoteSpec {
    .note = Note::A, .oct = 4, .vol = 0, .wav = WaveForm::NONE, .fx = Effect::NONE
};

struct TuneSpec {
    uint8_t noteDuration;        // in "ticks". [1..64]
    uint8_t loopStart, numNotes;
    const NoteSpec* notes;
};

struct WaveTable {
    const uint16_t numSamples;
    const int8_t* samples;
};

class TuneGenerator {
    const TuneSpec* _tuneSpec;
    int16_t _samplesPerNote;

// Current note
    const NoteSpec* _note;
    const WaveTable* _waveTable;
    const NoteSpec* _arpeggioNote;
    int32_t _waveIndex, _maxWaveIndex;
    int32_t _indexDelta, _indexDeltaDelta;
    int32_t _vibratoDelta, _vibratoDeltaDelta;
    int16_t _sampleIndex, _endMainIndex;
    int16_t _volume, _volumeDelta;
    int16_t _blendSample, _blendDelta;
    uint8_t _noiseShift;

    void inline setSamplesPerNote() {
        _samplesPerNote = (_tuneSpec->noteDuration * SAMPLES_PER_TICK) << (2 - SAMPLERATE_SHIFT);
    }

    int inline arpeggioFactor(const NoteSpec* note) const {
        // Assumes effect is ARPEGGIO or ARPEGGIO_FAST
        return (note->fx == Effect::ARPEGGIO_FAST ? 3 : 2) - (_tuneSpec->noteDuration <= 8 ? 1 : 0);
    }
    bool isFirstArpeggioNote() const;
    bool isLastArpeggioNote() const;

    void startNote();

    const NoteSpec* peekPrevNote() const;
    const NoteSpec* peekNextNote() const;
    void moveToNextNote();

    void addMainSamples(Sample* &curP, Sample* endP);
    void addMainSamplesSilence(Sample* &curP, Sample* endP);
    void addMainSamplesVibrato(Sample* &curP, Sample* endP);
    void addBlendSamples(Sample* &curP, Sample* endP);

public:
    void setTuneSpec(const TuneSpec* tuneSpec);
    bool isDone() { return _note == nullptr; }

    // Adds samples for the tune to the given buffer. Note, it does not overwrite existing values
    // in the buffer, but adds to the existing value so that multiple generators can contribute to
    // the same buffer. This relies on an overarching orchestrator to clear the buffer values at
    // right moment.
    //
    // Returns the number of samples added. It can less than the maximum when the tune ends.
    int addSamples(Sample* buf, int maxSamples);
};

//--------------------------------------------------------------------------------------------------
// Pattern Generation

struct PatternSpec {
    uint8_t numTunes;
    const TuneSpec** tunes;
};

constexpr int MAX_TUNES_IN_PATTERN = 4;

class PatternGenerator {
    const PatternSpec* _patternSpec;
    TuneGenerator _tuneGens[MAX_TUNES_IN_PATTERN];

public:
    void setPatternSpec(const PatternSpec* patternSpec);

    // Adds samples for the pattern to the given buffer. Note, it does not overwrite existing values
    // in the buffer, but adds to the existing value so that multiple generators can contribute to
    // the same buffer. This relies on an overarching orchestrator to clear the buffer values at
    // right moment.
    //
    // Returns the number of samples added. It can less than the maximum when the pattern ends.
    int addSamples(Sample* buf, int maxSamples);
};

//--------------------------------------------------------------------------------------------------
// Song Generation

struct SongSpec {
    uint8_t loopStart, numPatterns;
    const PatternSpec** patterns;
};

class SongGenerator {
    const SongSpec* _songSpec;
    PatternGenerator _patternGenerator;
    const PatternSpec** _pattern;
    bool _loop;

    void startPattern();
    void moveToNextPattern();

public:
    void setSongSpec(const SongSpec* songSpec, bool loop);
    bool isDone() { return _pattern == nullptr; }

    // Adds samples for the tune to the given buffer. Note, it does not overwrite existing values
    // in the buffer, but adds to the existing value so that multiple generators can contribute to
    // the same buffer. This relies on an overarching orchestrator to clear the buffer values at
    // right moment.
    //
    // Returns the number of samples added. It can less than the maximum when the song ends.
    int addSamples(Sample* buf, int maxSamples);
};

//--------------------------------------------------------------------------------------------------
// Music Handler

class MusicHandler {
    int16_t _buffer[SOUND_MUSIC_BUFFERSIZE];
    int16_t* _readP;
    int16_t* _headP;
    int16_t* _endP;
    int16_t* _zeroP;

    TuneGenerator _tuneGenerator;

public:
    MusicHandler();

    void play(const TuneSpec* tuneSpec);

    void update();

    uint16_t nextSample() {
        if (_readP == _endP) { _readP = _buffer; }
        return *_readP++;
    }
};

} // Namespace

#endif /* TuneGenerator_h */
