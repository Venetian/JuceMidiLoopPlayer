//
//  JuceSequenceLoopPlayer.h
//  JuceAbletonMidiFilePlayer
//
//  Created by Andrew Robertson on 31/07/2014.
//
//

#ifndef __JuceAbletonMidiFilePlayer__JuceSequenceLoopPlayer__
#define __JuceAbletonMidiFilePlayer__JuceSequenceLoopPlayer__

#include <iostream>

#include "../JuceLibraryCode/JuceHeader.h"

class JuceSequenceLoopPlayer {
public:
    JuceSequenceLoopPlayer();
    ~JuceSequenceLoopPlayer();
    
    void setSequence(const MidiMessageSequence&targetSequence, int tmpppq);
    
    void reset();
    
    void startMidiPlayback();
    void stopMidiPlayback();
    
    void reverseSequence(MidiMessageSequence& sequence, int startStamp, int endStamp);
    
    void printSequenceEvents();



    MidiOutput* midiOutDevice;
    
    void updateTicksSinceLastBeat(double ticksSinceBeatTick);
    
    void updatePlaybackToBeat(int& beatIndex);//, int& millisCount);
    double lastTick;
private:
    MidiMessageSequence sequence;
    
    double globalTickPosition;
    double loopTickPosition;
    
    int ppq;
    int beatMillisCounter;//millis counter when on beat
    int beatTick;
 
    
//    int millisCounter;
    int midiPlayIndex;
    
    int loopStartTicks;//in beats Ticks;
    int loopEndTicks;//in beats Ticks;
    
    double getTicksFromBeat(double beatPositionTicks);//returns ticks within limits
    
//    void updatePlaybackToBeat(int beatIndex);
 //   void updateMidiPlayPositionToTickPosition(double position);//for both sequences
 //   void updateMidiPlayPosition();
    void updateMidiPlayPositionToTickPosition(double startTick, double tickPosition);
    
    MidiMessageSequence trackSequence;//pointer to track we load in
    MidiMessageSequence::MidiEventHolder* playEvent;//pointer to an individual midi event
    
};
#endif /* defined(__JuceAbletonMidiFilePlayer__JuceSequenceLoopPlayer__) */
