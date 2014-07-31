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
                                public Label::Listener, public ComboBox::Listener// to listen to menu
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
    
    
    
private:
    OSCAbletonFinder finder;
    
    void newAbletonBeatReceived(int beatIndex, float tempo);
    //menu for midi devices
    ComboBox midiOutputBox;
    ComboBox midiLooperOutputBox;
    ScopedPointer<MidiOutput> midiOutputDevice;
    ScopedPointer<MidiOutput> midiOutputLooperDevice;
    
    JuceMidiFilePlayer midiPlayer;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
