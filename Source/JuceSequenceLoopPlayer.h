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
    void changeTicksToBeats(MidiMessageSequence& sequence);
    
    void reset();
    
    void startMidiPlayback();
    void stopMidiPlayback();
    
    void reverseSequence(MidiMessageSequence& sequence, int startStamp, int endStamp);
    
    void printSequenceEvents(const MidiMessageSequence& sequence);



    MidiOutput* midiOutDevice;
    MidiInput* midiInputDevice;
    
    void newMidiMessage(const MidiMessage& message);
    
    void updateTicksSinceLastBeat(double ticksSinceBeatTick);
    
    void updatePlaybackToBeat(int& beatIndex);//, int& millisCount);
    double lastTick;
    String name;
    void stop();
    
    void reverseSequence(MidiMessageSequence& transformedSequence, const MidiMessageSequence& sequence, int startStamp, int endStamp);
    
    void invertSequence(MidiMessageSequence& invertedSequence, const MidiMessageSequence& sequence, int startStamp, int endStamp);
    
    void setSequence(const MidiMessageSequence& sequence);
    


    //transforms
    void reverseOriginal();
    void invertOriginal();
    
    MidiMessageSequence transformedSequence;//make private
    
    MidiMessageSequence beatDefinedSequence;
    
    Value noteOnValue;
    void revertToOriginal();
    
    void transposeSequence(int notesInScale);
    Value reversedValue;
    
    //new update routines
    void alternativeUpdateToBeat(const float& beatNow);
    void updateToBeatPosition(const float& beatPosition);
    
    float loopEndBeats;
    float loopStartBeats;
    float loopWidthBeats;//to save time
    double lastBeatPosition;
    
    void setLoopPointsBeats(float startLoop, float endLoop);
    
private:
    MidiMessageSequence originalSequence;
    
    void sendMessageOut(MidiMessage& m);
    
    int invertAnchor;
    int invertScale[8];
    int invertPitch(int pitch);
    
    double globalTickPosition;
    double loopTickPosition;
    
    int ppq;
    int beatMillisCounter;//millis counter when on beat
    int beatTick;
 
    float getLoopPosition(const float& beatPosition);
    float getModulo(float& highValue, float& moduloValue);
    
    std::vector<int> notesSentOut;
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
