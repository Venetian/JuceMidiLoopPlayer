/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

#define OSC_RECEIVER_PORT 12178
//==============================================================================
MainContentComponent::MainContentComponent() : finder("abletonSync", OSC_RECEIVER_PORT)
{
    setSize (500, 400);

  //finder.init();
    
    finder.startThread();
    
    //set up listeners here for new values in our osc class that listens to ableton
    beatValue.setValue(-1);
    beatValue.addListener(this);
    beatValue.referTo(finder.beatVal);
    
    sysTimeValue.setValue(-1);
    sysTimeValue.addListener(this);
    sysTimeValue.referTo(finder.sysTimeVal);
    
    tempoValue.setValue(-1);
    tempoValue.addListener(this);
    tempoValue.referTo(finder.tempoVal);
    
    
    
    //add text for ableton sync
    beatInfo.setText("info", sendNotification);
    beatInfo.addListener(this);
    addAndMakeVisible(&beatInfo);
    
    systemTimeInfo.setText("time info", sendNotification);
    systemTimeInfo.addListener(this);
    addAndMakeVisible(&systemTimeInfo);
    
    tempoInfo.setText("tempoInfo", sendNotification);
    tempoInfo.addListener(this);
    addAndMakeVisible(&tempoInfo);
    
    //add midi menu
    addAndMakeVisible (midiOutputBox);
	midiOutputBox.setTextWhenNoChoicesAvailable ("No MIDI Output Enabled");
	const StringArray midiOutputs (MidiOutput::getDevices());
	midiOutputBox.addItemList (midiOutputs, 1);
	midiOutputBox.addListener (this);
	midiOutputDevice = NULL;
    
    addAndMakeVisible (midiLooperOutputBox);
	midiOutputBox.setTextWhenNoChoicesAvailable ("No MIDI Looper Output Enabled");
	const StringArray midiLooperOutputs (MidiOutput::getDevices());
	midiLooperOutputBox.addItemList (midiLooperOutputs, 1);
	midiLooperOutputBox.addListener (this);
	midiOutputLooperDevice = NULL;
    
    resized();
    

}

MainContentComponent::~MainContentComponent()
{
    
    midiOutputBox.removeListener(this);

    midiLooperOutputBox.removeListener(this);
}

void MainContentComponent::valueChanged(Value& value){

    std::cout << "value changed " << std::endl;//(int)value.getValue() << std::endl;

    if (value == beatValue){
        beatInfo.setText("beat "+value.toString()+"  "+String(midiPlayer.looper.lastTick), sendNotification);//(int)value.getValue());
    } else if (value == sysTimeValue){
        systemTimeInfo.setText("sysTime "+value.toString(), sendNotification);
    } else if (value == tempoValue){
        tempoInfo.setText("tempo "+value.toString(), sendNotification);
    }
    
    newAbletonBeatReceived((int)beatValue.getValue(), (float)tempoValue.getValue());

}

void MainContentComponent::newAbletonBeatReceived(int beatIndex, float tempo){
    std::cout << "beat " << beatIndex << " tempo " << tempo << std::endl;
    midiPlayer.newBeat(beatIndex, tempo);
}

void MainContentComponent::labelTextChanged(Label* labelThatHasChanged){
    
}

void MainContentComponent::comboBoxChanged(ComboBox* box)//override
{
	if (box == &midiOutputBox){
		midiOutputDevice = MidiOutput::openDevice(midiOutputBox.getSelectedItemIndex());
		//midiOutputDevice->sendMessageNow(MidiMessage(144, 60, 20));
        
        midiPlayer.midiDevice = midiOutputDevice;//always point to our chosen midi output
        
        //for now same as the other
        midiOutputLooperDevice = MidiOutput::openDevice(midiOutputBox.getSelectedItemIndex());
        midiPlayer.looper.midiOutDevice = midiOutputLooperDevice;
        
    } else if (box == &midiLooperOutputBox){
		midiOutputLooperDevice = MidiOutput::openDevice(midiOutputBox.getSelectedItemIndex());
        
        //for now same as the other
        midiOutputLooperDevice = MidiOutput::openDevice(midiLooperOutputBox.getSelectedItemIndex());
        midiPlayer.looper.midiOutDevice = midiOutputLooperDevice;
    
    }
    
}


void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour (0xffeeddff));

    g.setFont (Font (16.0f));
    g.setColour (Colours::black);
    g.drawText ("listening on "+String(OSC_RECEIVER_PORT), getLocalBounds(), Justification::top, true);
    //g.drawText (beatInfo.getText(), getLocalBounds(), Justification::topLeft, true);
}

void MainContentComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
    beatInfo.setBounds(20,20,100,40);
    systemTimeInfo.setBounds(20,40,200,40);
    tempoInfo.setBounds(20,60,100,40);
    
    midiOutputBox.setBoundsRelative(0.05,0.25,0.3,0.05);
    midiLooperOutputBox.setBoundsRelative(0.05, 0.35, 0.3, 0.05);
}
