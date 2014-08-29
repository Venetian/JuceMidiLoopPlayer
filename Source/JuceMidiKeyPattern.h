//
//  JuceMidiKeyPattern.h
//  JuceAbletonMidiFilePlayer
//
//  Created by Andrew Robertson on 28/08/2014.
//
//

#ifndef __JuceAbletonMidiFilePlayer__JuceMidiKeyPattern__
#define __JuceAbletonMidiFilePlayer__JuceMidiKeyPattern__

#include <iostream>

#include "../JuceLibraryCode/JuceHeader.h"



//analyses a midi sequence relative to a given key
//classifying which pitches are in the scale

struct RhythmicNote{
    int pitch;
    int octave;
    bool sharp;
    float beatPosition;
    float duration;
    int velocity;
};

class JuceMidiKeyPattern{
public:
    JuceMidiKeyPattern();
    ~JuceMidiKeyPattern();
    
    void clearSequence();

    void analyseSequence(MidiMessageSequence& sequence, int key);
    void addToMidiSequence(MidiMessageSequence& sequence, float startTime, int patternIndex);
    //get p in terms of notes greater than start note of q mod 12
    int scale(int p, int q);
    int findPitch(int pitch, int key);
    int findOctave(int pitch, int key);
    int getPitchFromScaleNote(int index, int octave, int sharp);
    int getPitch(int octave, int noteIndex, int key);
    std::vector<int> bluesScale;
    int scaleSize;
    
    typedef std::vector<RhythmicNote> RhythmicPattern;
    std::vector<RhythmicPattern> soloPattern;
};
#endif /* defined(__JuceAbletonMidiFilePlayer__JuceMidiKeyPattern__) */
