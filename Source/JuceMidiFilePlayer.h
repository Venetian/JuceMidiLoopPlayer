//
//  JuceMidiFilePlayer.h
//  JuceAbletonMidiFilePlayer
//
//  Created by Andrew Robertson on 31/07/2014.
//
//

#ifndef __JuceAbletonMidiFilePlayer__JuceMidiFilePlayer__
#define __JuceAbletonMidiFilePlayer__JuceMidiFilePlayer__

#include <iostream>
#include "../JuceLibraryCode/JuceHeader.h"

#include "JuceSequenceLoopPlayer.h"

class JuceMidiFilePlayer : private HighResolutionTimer {
public:
    JuceMidiFilePlayer();
    ~JuceMidiFilePlayer();
    
    MidiFile midiFile;
    
    void loadMidiFile(String fileLocation);
    void startMidiPlayback();
    void stopMidiPlayback();

    void newBeat(int beatIndex, float tempoMillis);
    
    void reverseSequence(MidiMessageSequence& sequence, int startStamp, int endStamp);
    JuceSequenceLoopPlayer looper;
    
    ScopedPointer<MidiOutput> midiDevice;
    

    
private:
    
    float playbackSpeed;
    //timer to play file - called every millisecond
    void hiResTimerCallback() override{
        updateMidiPlayPosition();
    }

    int ppq;
    int beatMillisCounter;//millis counter when on beat
    int beatTick;
    
    int millisCounter;
    int midiPlayIndex;
    void reset();
    
    void setTempo(float tempoMillis);
    
    void updatePlaybackToBeat(int beatIndex);
    void updateMidiPlayPositionToTickPosition(double position);//for both sequences
    void updateMidiPlayPosition();
    void updateMidiPlayPositionToTickPosition(MidiMessageSequence& sequence, float tickPosition);

    MidiMessageSequence trackSequence;//pointer to track we load in
    MidiMessageSequence::MidiEventHolder* playEvent;//pointer to an individual midi event
    
    MidiMessageSequence loopSequence, mutatedSequence;//loop what we play, mutate for the transformed version

    void printSequenceEvents(const MidiMessageSequence& sequence);//to see what is in sequences
    
    void bufferTest();//not needed

};
#endif /* defined(__JuceAbletonMidiFilePlayer__JuceMidiFilePlayer__) */
