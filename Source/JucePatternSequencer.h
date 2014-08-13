//
//  JucePatternSequencer.h
//  JuceAbletonMidiFilePlayer
//
//  Created by Andrew Robertson on 12/08/2014.
//
//

#ifndef __JuceAbletonMidiFilePlayer__JucePatternSequencer__
#define __JuceAbletonMidiFilePlayer__JucePatternSequencer__

#include <iostream>

#include "../JuceLibraryCode/JuceHeader.h"


class MidiMessageArraySorter
{
public:
    static int compareElements(MidiMessage first, MidiMessage second){
        if (first.getNoteNumber() < second.getNoteNumber())
            return -1;
        else if (first.getNoteNumber() == second.getNoteNumber())
            return 0;
        else
            return 1;
    }
};

class JucePatternSequencer{
public:
    JucePatternSequencer();
    ~JucePatternSequencer();
    
    MidiMessageSequence rhythmPattern;
    //need to record the midi notes by storing the index in the pitchSet
    //as a usual midi pattern
    
    
    Array<MidiMessage> pitchSet;//set of notes to play our pattern with
    int getIndex(int& pitch);
    
    
    void loadSequence(MidiMessageSequence& sequence);
    
    MidiMessageSequence outputSequence;
    
    void printPitchSet();
    
    void newMidiMessage(MidiMessage& message);
    
    int* milliscounter;
    
    float* loopMin;
    float* loopMax;
    void setLoopPoints(float& loopPointMin, float& loopPointMax);
    
    bool noteIsInPitchSet(int& pitch);
    
    void generateOutputSequence(MidiMessageSequence& sequence);
    void reorderPitchSetAndRhythm();
    
//    MidiMessageArraySorter sorter;
    
};

#endif /* defined(__JuceAbletonMidiFilePlayer__JucePatternSequencer__) */
