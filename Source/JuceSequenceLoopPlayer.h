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

//#include "JuceMidiFilePlayer.h"

static String getMidiMessageDescription (const MidiMessage& m)
{
    if (m.isNoteOn())           return "Note on "  + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
    if (m.isNoteOff())          return "Note off " + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
    if (m.isProgramChange())    return "Program change " + String (m.getProgramChangeNumber());
    if (m.isPitchWheel())       return "Pitch wheel " + String (m.getPitchWheelValue());
    if (m.isAftertouch())       return "After touch " + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) +  ": " + String (m.getAfterTouchValue());
    if (m.isChannelPressure())  return "Channel pressure " + String (m.getChannelPressureValue());
    if (m.isAllNotesOff())      return "All notes off";
    if (m.isAllSoundOff())      return "All sound off";
    if (m.isMetaEvent())        return "Meta event";
    
    if (m.isController())
    {
        String name (MidiMessage::getControllerName (m.getControllerNumber()));
        
        if (name.isEmpty())
            name = "[" + String (m.getControllerNumber()) + "]";
        
        return "Controler " + name + ": " + String (m.getControllerValue());
    }
    
    return String::toHexString (m.getRawData(), m.getRawDataSize());
}

class MidiLogListBoxModel   : public ListBoxModel
{
public:
    MidiLogListBoxModel (const Array<MidiMessage>& list)
    : midiMessageList (list)
    {
    }
    
    int getNumRows() override    { return midiMessageList.size(); }
    
    void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll (Colours::blue.withAlpha (0.2f));
        
        if (isPositiveAndBelow (row, midiMessageList.size()))
        {
            g.setColour (Colours::black);
            
            const MidiMessage& message = midiMessageList.getReference (row);
            double time = message.getTimeStamp();
            
            g.drawText (String::formatted ("%02d:%02d:%02d",
                                           ((int) (time / 3600.0)) % 24,
                                           ((int) (time / 60.0)) % 60,
                                           ((int) time) % 60)
                        + "  -  " + getMidiMessageDescription (message),
                        Rectangle<int> (width, height).reduced (4, 0),
                        Justification::centredLeft, true);
        }
    }
    
private:
    const Array<MidiMessage>& midiMessageList;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiLogListBoxModel)
};


class JuceSequenceLoopPlayer: private AsyncUpdater {
    //asyncupdater is triggered when new midi message added to our list box above (grabbed from juce demo)
    //so we see the midi messages input
public:
    JuceSequenceLoopPlayer();
    ~JuceSequenceLoopPlayer();
    
    void setSequence(const MidiMessageSequence&targetSequence, int tmpppq);
    void changeTicksToBeats(MidiMessageSequence& sequence);
    
    void reset();
    
    void startMidiPlayback();
    void stopMidiPlayback();
    
   // void reverseSequence(MidiMessageSequence& sequence, float startStamp, float endStamp);
    
    void printSequenceEvents(const MidiMessageSequence& sequence);



    MidiOutput* midiOutDevice;
    MidiInput* midiInputDevice;
    
    void newMidiMessage(const MidiMessage& message, float& beatTime);
    
    void updateTicksSinceLastBeat(double ticksSinceBeatTick);
    
    void updatePlaybackToBeat(int& beatIndex);//, int& millisCount);
    double lastTick;
    String name;
    void stop();
    
    void reverseSequence(MidiMessageSequence& transformedSequence, const MidiMessageSequence& sequence, float startStamp, float endStamp);
    
    void invertSequence(MidiMessageSequence& invertedSequence, const MidiMessageSequence& sequence, int startStamp, int endStamp);
    
    void setSequence(const MidiMessageSequence& sequence);
    

    float lastMidiMessageInTime();
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
    float lastBeatPosition;
    
    void setLoopPointsBeats(float startLoop, float endLoop);
    
    ListBox messageListBox;
    MidiLogListBoxModel midiLogListBoxModel;
    
    
    //not particularly keen on this one but
    int* milliscounter;
    float* tempoMillis;

    
private:
    Array<MidiMessage> midiMessageList;
    
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
    int outputCheckIndex;
    void checkOutput(float& lastBeatTime, const float& beatTime);
    
    double getTicksFromBeat(double beatPositionTicks);//returns ticks within limits
    
//    void updatePlaybackToBeat(int beatIndex);
 //   void updateMidiPlayPositionToTickPosition(double position);//for both sequences
 //   void updateMidiPlayPosition();
    void updateMidiPlayPositionToTickPosition(double startTick, double tickPosition);
    
    MidiMessageSequence trackSequence;//pointer to track we load in
    MidiMessageSequence::MidiEventHolder* playEvent;//pointer to an individual midi event
  
    float currentGlobalBeatPosition;
    
    MidiMessageSequence scheduledEvents;
//    MidiBuffer midiBuffer;

    int beatsToMillis(float& beats);
   // float millisToBeats(float& millis);
    
    void addNoteOff(MidiMessage& message, int millisTime);
    void checkNoteOffs();
    bool checkLock;
    void handleAsyncUpdate()
    {
        messageListBox.updateContent();
        messageListBox.scrollToEnsureRowIsOnscreen (midiMessageList.size() - 1);
        messageListBox.repaint();
    }
    
};
#endif /* defined(__JuceAbletonMidiFilePlayer__JuceSequenceLoopPlayer__) */
