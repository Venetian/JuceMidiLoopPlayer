/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "OSCAbletonFinder.h"
#include "JuceMidiFilePlayer.h"
//this receives ableton beats via osc


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public Component, public Value::Listener,
                                public Label::Listener, public ComboBox::Listener,// to listen to menu
                                public Button::Listener, private MidiInputCallback
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&);
    void resized();


    Value beatValue;
    Value sysTimeValue;
    Value tempoValue;
    void valueChanged(Value& value);
    
    Label beatInfo;
    Label systemTimeInfo;
    Label tempoInfo;
    void labelTextChanged(Label* labelThatHasChanged);
    
    void comboBoxChanged(ComboBox* box) override;
    
    //notes out for our two MIDI hardware devices
    Label prophetLabel;
    Label looperLabel;
    
    Value prophetNoteValue;
    
    Value midiViewValue;
    
    void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) ;
 //   void handleIncomingMidiMessageMoog (MidiInput* source, const MidiMessage& message) ;
    
private:
    OSCAbletonFinder finder;
    
    void newAbletonBeatReceived(float beatIndex, float tempo, unsigned long systemTimeAbleton);
    //menu for midi devices
    ComboBox midiProphetOutputBox;
    ComboBox midiLooperOutputBox;
    
    ComboBox midiProphetInputBox;
    ComboBox midiMoogInputBox;
    
    TextButton prophetButton;
    Value prophetReversedValue;
    void buttonClicked(Button* button);
    
    ScopedPointer<MidiOutput> midiOutputDevice;//prophet
    ScopedPointer<MidiOutput> midiOutputLooperDevice;//moog
    
    JuceMidiFilePlayer midiPlayer;
    int lastBeatIndex;
 
    //for MIDI in and Audio In
    AudioDeviceManager deviceManager;
   // void handleIncomingMidiMessageInt (MidiInput*, const MidiMessage&);
    void setMidiInput (int index);
    int lastMoogInputIndex;
    int lastProphetInputIndex;
    
    ScopedPointer<MidiInput> midiProphetInputDevice;
    ScopedPointer<MidiInput> midiMoogInputDevice;
    
    String moogInputName;//to help parse midi messages by channel
    String prophetInputName;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
