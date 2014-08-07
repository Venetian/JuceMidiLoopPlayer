/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

#include <sys/time.h>


#define OSC_RECEIVER_PORT 12178
//==============================================================================
MainContentComponent::MainContentComponent() : finder("abletonSync", OSC_RECEIVER_PORT),
prophetButton("prophet reverse")// deviceManager()
{
    setSize (500, 400);

  //finder.init();
    
//    deviceManager = new AudioDeviceManager();
    deviceManager.initialise(2, 2, 0, true, String::empty, 0);

    finder.halfTime = false;//set this
    
    finder.startThread();
    
    //set up listeners here for new values in our osc class that listens to ableton
    finder.beatVal.setValue(-1);
    beatValue.setValue(finder.beatVal.getValue());
    beatValue.referTo(finder.beatVal);
    beatValue.addListener(this);
    
    
   // sysTimeValue.setValue(-1);
    sysTimeValue.addListener(this);
    sysTimeValue.referTo(finder.sysTimeVal);
    
    tempoValue.setValue(-1);
    tempoValue.referTo(finder.tempoVal);
    tempoValue.addListener(this);
    
    
    //these above not really what we want to start
    finder.tempoVal = 400;//150bpm default
    finder.beatVal = -1;//to set it to the STOP position
    
    
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
    addAndMakeVisible (midiProphetOutputBox);
	midiProphetOutputBox.setTextWhenNoChoicesAvailable ("No MIDI Output Enabled");
	const StringArray midiOutputs (MidiOutput::getDevices());
	midiProphetOutputBox.addItemList (midiOutputs, 1);
	midiProphetOutputBox.addListener (this);
	midiOutputDevice = NULL;
    
   
    
    addAndMakeVisible (midiLooperOutputBox);
	midiProphetOutputBox.setTextWhenNoChoicesAvailable ("No MIDI Looper Output Enabled");
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
    
    midiMoogInputBox.setSelectedId(2);
    midiLooperOutputBox.setSelectedId(2);
    
    
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
    
    midiProphetOutputBox.setSelectedId(1);
    midiProphetInputBox.setSelectedId(1);

    
    lastBeatIndex = -1;//to recognise new beat info
    
    //add message box for looper
    addAndMakeVisible (midiPlayer.looper.messageListBox);
    midiPlayer.looper.messageListBox.setModel (&midiPlayer.looper.midiLogListBoxModel);
    midiPlayer.looper.messageListBox.setColour (ListBox::backgroundColourId, Colour (0x32ffffff));
    midiPlayer.looper.messageListBox.setColour (ListBox::outlineColourId, Colours::black);
    
    addAndMakeVisible (midiPlayer.prophet.messageListBox);
    midiPlayer.prophet.messageListBox.setModel (&midiPlayer.prophet.midiLogListBoxModel);
    midiPlayer.prophet.messageListBox.setColour (ListBox::backgroundColourId, Colour (0x32ffffff));
    midiPlayer.prophet.messageListBox.setColour (ListBox::outlineColourId, Colours::black);

    
    resized();//at bottom of setup
}

MainContentComponent::~MainContentComponent()
{
    //need this stuff??
    midiProphetOutputBox.removeListener(this);
    midiLooperOutputBox.removeListener(this);
    midiProphetInputBox.removeListener(this);
    midiMoogInputBox.removeListener(this);
    
//last input below was
    deviceManager.removeMidiInputCallback (MidiInput::getDevices()[midiMoogInputBox.getSelectedItemIndex()], this);
//    deviceManager = nullptr;
    
    beatValue.removeListener(this);
}





void MainContentComponent::valueChanged(Value& value){

   std::cout << "value changed " << (float)value.getValue() << std::endl;//(int) << std::endl;

    if (value == beatValue){
        std::cout << "MCC: valchange: beat " << (float)value.getValue() << " tempo " << (float)tempoValue.getValue() << std::endl;
        
        beatInfo.setText("beat "+value.toString(), dontSendNotification);//(int)value.getValue());
        
        //have ordered this so that we update tempo, systemtime and then Ableton beat index

        newAbletonBeatReceived((float)beatValue.getValue(), (float)tempoValue.getValue(), (int)sysTimeValue.getValue());
        
    } else if (value == sysTimeValue){
        systemTimeInfo.setText("sysTime "+value.toString(), dontSendNotification);
        std::cout << "sys time changed"<< std::endl;
    } else if (value == tempoValue){
        tempoInfo.setText("tempo "+value.toString(), dontSendNotification);
        std::cout << " tempo changed " << value.toString() << std::endl;
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

}

unsigned long systemTime(){
	struct timeval now;
	gettimeofday( &now, NULL );
	unsigned long timenow = (unsigned long long) now.tv_usec/1000 + (unsigned long long) now.tv_sec*1000;
    //std::cout << "system time now " << timenow << " truncated " << (int)timenow << std::endl;
    return timenow;
}

void MainContentComponent::newAbletonBeatReceived(float beatIndex, float tempo, unsigned long systemTimeAbleton){
    std::cout << "MCC: beat " << beatIndex << " tempo " << tempo << std::endl;
    
    //some latency calculations, in case our osc message is over a network etc
    unsigned long systemTimeHere = systemTime();
    int latency = systemTimeHere - systemTimeAbleton;
    if (abs(latency) > 50){
        std::cout << "weird latency error " << latency << " with system time " << systemTimeHere << " so setting to zero " << std::endl;
        latency = 0;
    } else
        std::cout << "letency from Ableton Live is " << latency << std::endl;
    
  
    if (beatIndex != lastBeatIndex) {
        //var used so we don't double call
        
        std::cout << "mcc new beat " << beatIndex << ", tempo " << tempo << std::endl;
        lastBeatIndex = beatIndex;
        midiPlayer.newBeat(beatIndex, tempo, latency);

    } else {
        std::cout << "repetitive beat " << beatIndex << " tempo " << tempo << std::endl;
    }
}

void MainContentComponent::labelTextChanged(Label* labelThatHasChanged){
    if (labelThatHasChanged == &prophetLabel){
    }
}

void MainContentComponent::comboBoxChanged(ComboBox* box)//override
{
	if (box == &midiProphetOutputBox){
		midiOutputDevice = MidiOutput::openDevice(midiProphetOutputBox.getSelectedItemIndex());
        midiPlayer.prophet.midiOutDevice = midiOutputDevice;
        
    } else if (box == &midiLooperOutputBox){
        midiOutputLooperDevice = MidiOutput::openDevice(midiLooperOutputBox.getSelectedItemIndex());
        midiPlayer.looper.midiOutDevice = midiOutputLooperDevice;
    } else if (box == &midiMoogInputBox){
        /*if (midiMoogInputBox.getSelectedItemIndex() != lastProphetInputIndex){//i.e. the other one
            const StringArray list (MidiInput::getDevices());
            deviceManager.removeMidiInputCallback (list[lastMoogInputIndex], this);
            std::cout << "removing " << list[lastMoogInputIndex] << std::endl;
        }*/
        setMidiInput(midiMoogInputBox.getSelectedItemIndex());
        moogInputName = MidiInput::getDevices()[midiMoogInputBox.getSelectedItemIndex()];
        lastMoogInputIndex = midiProphetInputBox.getSelectedItemIndex();
        std::cout << "moog input is channel " << moogInputName << "index " << lastMoogInputIndex << std::endl;
        
    } else if (box == &midiProphetInputBox){
        /*
        if (midiProphetInputBox.getSelectedItemIndex() != lastMoogInputIndex){//i.e. the other one
            const StringArray list (MidiInput::getDevices());
            deviceManager.removeMidiInputCallback (list[lastProphetInputIndex], this);
            std::cout << "removing " << list[lastProphetInputIndex] << " index " << lastProphetInputIndex << std::endl;
        }*/
        
        setMidiInput(midiProphetInputBox.getSelectedItemIndex());
        prophetInputName = MidiInput::getDevices()[midiProphetInputBox.getSelectedItemIndex()];
        lastProphetInputIndex = midiProphetInputBox.getSelectedItemIndex();
        //, "prophet");
    }
    
    
}


//void MainContentComponent::handleIncomingMidiMessageMoog (MidiInput* source, const MidiMessage& message) {
//    std::cout << "midi in moog" << std::endl;
//}

void MainContentComponent::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) {
    std::cout << "midi in '" << source->getName() << "' " << message.getRawDataSize() << " bytes" << std::endl;
    //std::cout << source.getDevice
    float tmpTimeNow = midiPlayer.beatsNow();
    if (source->getName() == moogInputName)
        midiPlayer.looper.newMidiMessage(message, tmpTimeNow);
    else if (source->getName() == prophetInputName)
        midiPlayer.prophet.newMidiMessage(message, tmpTimeNow);

}


void MainContentComponent::setMidiInput (int index)
{
    const StringArray list (MidiInput::getDevices());
    
//    deviceManager.removeMidiInputCallback (list[lastInputIndex], this);
    
    const String newInput (list[index]);
    
    if (! deviceManager.isMidiInputEnabled (newInput))
        deviceManager.setMidiInputEnabled (newInput, true);
    
    deviceManager.addMidiInputCallback (newInput, this);
    std::cout << "add callback " << newInput << std::endl;
    
//    midiInputMoogdevice =
    //why this ??
    //midiMoogInputBox.setSelectedId (index + 1, dontSendNotification);
    //was midiInputList line above
    
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
    float generalInfoY = 0.8;
    float prophetX = 0.08;
    beatInfo.setBoundsRelative(prophetX, generalInfoY, 0.35, 0.05);
    systemTimeInfo.setBoundsRelative(prophetX, generalInfoY+0.06, 0.35, 0.05);
    tempoInfo.setBoundsRelative(prophetX, generalInfoY+0.12, 0.35, 0.05);

    float prophetY = 0.06;
    float height = 0.06;
    prophetLabel.setBoundsRelative(prophetX, prophetY, 0.4, 0.05);
    midiProphetInputBox.setBoundsRelative(prophetX, prophetY+height, 0.3, 0.05);
    midiProphetOutputBox.setBoundsRelative(prophetX,prophetY+2*height,0.3,0.05);
    
    prophetButton.setBoundsRelative(prophetX, prophetY+0.18, 0.25, 0.05);
    
    
    float moogX = 0.5;
    
    looperLabel.setBoundsRelative(moogX, prophetY, 0.4, 0.05);
    midiMoogInputBox.setBoundsRelative(moogX,prophetY+height,0.3,0.05);
    midiLooperOutputBox.setBoundsRelative(moogX, prophetY+2*height, 0.3, 0.05);

    
    Rectangle<int> area (getLocalBounds());
    midiPlayer.looper.messageListBox.setBoundsRelative(moogX, prophetY+4*height, 0.4, 0.45);
    
    midiPlayer.prophet.messageListBox.setBoundsRelative(prophetX, prophetY+4*height, 0.4, 0.45);
}



/*
 
 JUNK MIDI INPUT STUFF
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

