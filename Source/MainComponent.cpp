/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

#define OSC_RECEIVER_PORT 12178
//==============================================================================
MainContentComponent::MainContentComponent() : finder("abletonSync", OSC_RECEIVER_PORT),
prophetButton("prophet reverse")//, deviceManager()
{
    setSize (500, 400);

  //finder.init();
    
//    deviceManager = new AudioDeviceManager();
    deviceManager.initialise(2, 2, 0, true, String::empty, 0);

    finder.halfTime = false;//set this
    
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
    
    prophetNoteValue.setValue(-1);
    prophetNoteValue.addListener(this);
    prophetNoteValue.referTo(midiPlayer.prophet.noteOnValue);
    
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
    
    prophetLabel.setText("prophet info", sendNotification);
    prophetLabel.addListener(this);
    addAndMakeVisible(&prophetLabel);
    
    looperLabel.setText("looper info", sendNotification);
    looperLabel.addListener(this);
    addAndMakeVisible(&looperLabel);
    
    prophetButton.addListener(this);
    addAndMakeVisible(&prophetButton);
    prophetReversedValue.addListener(this);
    prophetReversedValue = 0;
    prophetReversedValue.referTo(midiPlayer.prophet.reversedValue);
    
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
    
    
    //midi in
    
    addAndMakeVisible (midiMoogInputBox);
	midiMoogInputBox.setTextWhenNoChoicesAvailable ("No MIDI Input");
	const StringArray midiInputs (MidiInput::getDevices());
	midiMoogInputBox.addItemList (midiInputs, 1);
	midiMoogInputBox.addListener (this);
	midiMoogInputDevice = NULL;
    
    
    // find the first enabled device and use that bu default
    for (int i = 0; i < midiInputs.size(); ++i)
    {
        if (deviceManager.isMidiInputEnabled (midiInputs[i]))
        {
            setMidiInput (i);
            break;
        }
    }
    
    addAndMakeVisible(midiProphetInputBox);
    midiProphetInputBox.setTextWhenNoChoicesAvailable ("No MIDI Input");
    midiProphetInputBox.addItemList (midiInputs, 1);
	midiProphetInputBox.addListener (this);
	midiProphetInputDevice = NULL;
    
    resized();
    
    lastBeatIndex = -1;//to recognise new beat info

}

MainContentComponent::~MainContentComponent()
{
    //need this stuff??
    midiOutputBox.removeListener(this);
    midiLooperOutputBox.removeListener(this);
    midiProphetInputBox.removeListener(this);
    midiMoogInputBox.removeListener(this);
    
//last input below was
    deviceManager.removeMidiInputCallback (MidiInput::getDevices()[midiMoogInputBox.getSelectedItemIndex()], this);
//    deviceManager = nullptr;
    
    beatValue.removeListener(this);
}



void MainContentComponent::valueChanged(Value& value){

   // std::cout << "value changed " << std::endl;//(int)value.getValue() << std::endl;

    if (value == beatValue){
        beatInfo.setText("beat "+value.toString()+"  "+String(midiPlayer.looper.lastTick), dontSendNotification);//(int)value.getValue());
    } else if (value == sysTimeValue){
        systemTimeInfo.setText("sysTime "+value.toString(), dontSendNotification);
    } else if (value == tempoValue){
        tempoInfo.setText("tempo "+value.toString(), dontSendNotification);
    } else if (value == prophetNoteValue){
        prophetLabel.setText(value.toString(), dontSendNotification);
    } else if (value == prophetReversedValue){
        if ((int)prophetReversedValue.getValue() == 1)
            prophetButton.setButtonText("reversed");
        else if ((int)prophetReversedValue.getValue() == 0)
            prophetButton.setButtonText("original");
        else if ((int)prophetReversedValue.getValue() == 2)
            prophetButton.setButtonText("inverted");
    }
   
    
    newAbletonBeatReceived((int)beatValue.getValue(), (float)tempoValue.getValue());
    
    
    
    

}

void MainContentComponent::newAbletonBeatReceived(int beatIndex, float tempo){
    //std::cout << "MCC: beat " << beatIndex << " tempo " << tempo << std::endl;
    if (beatIndex != lastBeatIndex){
        lastBeatIndex = beatIndex;
        midiPlayer.newBeat(beatIndex, tempo);
    }
}

void MainContentComponent::labelTextChanged(Label* labelThatHasChanged){
    if (labelThatHasChanged == &prophetLabel){
    }
}

void MainContentComponent::comboBoxChanged(ComboBox* box)//override
{
	if (box == &midiOutputBox){
		midiOutputDevice = MidiOutput::openDevice(midiOutputBox.getSelectedItemIndex());
        midiPlayer.prophet.midiOutDevice = midiOutputDevice;
        
    } else if (box == &midiLooperOutputBox){
        midiOutputLooperDevice = MidiOutput::openDevice(midiLooperOutputBox.getSelectedItemIndex());
        midiPlayer.looper.midiOutDevice = midiOutputLooperDevice;
    } else if (box == &midiMoogInputBox){
        
        setMidiInput(midiMoogInputBox.getSelectedItemIndex());
        /*
        if (midiMoogInputDevice != nullptr)
            midiMoogInputDevice->stop();
        
        midiMoogInputDevice = MidiInput::openDevice(midiMoogInputBox.getSelectedItemIndex(), this);
        midiMoogInputDevice->start();
        */
        
        /*
        int index = 0;
        const StringArray list (MidiInput::getDevices());
        
        std::cout << list.size() << std::endl;
        
        for (int i = 0; i < list.size(); i++)
            std::cout << list[i].toStdString() << std::endl;
        
        
        deviceManager.removeMidiInputCallback (list[lastInputIndex], this);
        
        const String newInput (list[index]);
        
        if (! deviceManager.isMidiInputEnabled (newInput))
            deviceManager.setMidiInputEnabled (newInput, true);
        
        deviceManager.addMidiInputCallback (list[index], this);//MidiInput::getDevices()[midiMoogInputBox.getSelectedItemIndex()], this);
        
        //std::cout << "add midi call in " << index << " " << list[index] << std::endl;
        */
        
  
        //midiPlayer.looper.midiInputDevice = midiMoogInputDevice;

        //replace above with this:
        //setMidiInput(midiMoogInputBox.getSelectedItemIndex());
       // std::cout << "set midi in " << midiMoogInputBox.getSelectedItemIndex() << " " << list[midiMoogInputBox.getSelectedItemIndex()] << std::endl;
    }
    
}


//void MainContentComponent::handleIncomingMidiMessageMoog (MidiInput* source, const MidiMessage& message) {
//    std::cout << "midi in moog" << std::endl;
//}

void MainContentComponent::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) {
    std::cout << "midi in '" << source->getName() << "' " << message.getRawDataSize() << " bytes" << std::endl;
    //std::cout << source.getDevice
    
}


void MainContentComponent::setMidiInput (int index)
{
    const StringArray list (MidiInput::getDevices());
    
    deviceManager.removeMidiInputCallback (list[lastInputIndex], this);
    
    const String newInput (list[index]);
    
    if (! deviceManager.isMidiInputEnabled (newInput))
        deviceManager.setMidiInputEnabled (newInput, true);
    
    deviceManager.addMidiInputCallback (newInput, this);
    std::cout << "add callback " << newInput << std::endl;
    
//    midiInputMoogdevice =
    //why this ??
    //midiMoogInputBox.setSelectedId (index + 1, dontSendNotification);
    //was midiInputList line above
    
    lastInputIndex = index;
}

void newInput(){

}



void MainContentComponent::buttonClicked(Button* button){
    if (button == &prophetButton){
        prophetReversedValue.setValue( (int)prophetReversedValue.getValue()+1);
        int newVal = (int)prophetReversedValue.getValue();
        if (newVal == 3){
            prophetReversedValue.setValue(0);
            newVal = 0;
        }
        
        switch (newVal){
            case 1:
                midiPlayer.prophet.reverseOriginal();
                break;
            case 2:
                midiPlayer.prophet.invertOriginal();
                break;
            case 0:
                midiPlayer.prophet.revertToOriginal();
                break;
        }
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
    
    prophetLabel.setBoundsRelative(0.65, 0.25, 0.4, 0.05);
    looperLabel.setBoundsRelative(0.35, 0.35, 0.4, 0.05);
    
    midiOutputBox.setBoundsRelative(0.05,0.25,0.3,0.05);
    midiLooperOutputBox.setBoundsRelative(0.05, 0.35, 0.3, 0.05);
    
    midiMoogInputBox.setBoundsRelative(0.05,0.65,0.3,0.05);
    midiProphetInputBox.setBoundsRelative(0.05, 0.75, 0.3, 0.05);
    
    prophetButton.setBoundsRelative(0.35, 0.25, 0.25, 0.05);
}
