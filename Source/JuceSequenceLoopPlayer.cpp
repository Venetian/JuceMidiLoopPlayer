//
//  JuceSequenceLoopPlayer.cpp
//  JuceAbletonMidiFilePlayer
//
//  Created by Andrew Robertson on 31/07/2014.
//
//

#include "JuceSequenceLoopPlayer.h"

//each looper should receive the beats from ableton and the counter
//given the loop points, and the start time (zero?)
//it can figure out where it is in the loop

//buffer midi out events so that note offs are scheduled ahead of time
//straight from the tick counter

//receive beat index, tempo, millisCounter

//millis counter will help us schedule abny note off

//LOOK at the clock
//should be neater
//in comes ticks globally
//related to internal loop clicks
//relate beats to internal beat structure

//SCHEDULE midi off events from the on events
//relative to the global tick counter
//not waiting for them to happen in the loop

//RHYTHM
//look at the way drum machine rhythm can be used to generate patterns
//sparse and busy




//need to schedule note offs for each note on
//looping might mean we exceed length of loop
 
JuceSequenceLoopPlayer::JuceSequenceLoopPlayer(){
    ppq = 480; 
    //set loop
    loopStartTicks = 0;
    loopEndTicks = ppq*16;//4 beat loop of beginning
    
    name = "";
    
    midiPlayIndex = -1;
    midiOutDevice = NULL;
    
    reset();
    
    
    invertScale[0] = 0;
    invertScale[1] = 2;
    invertScale[2] = 3;
    invertScale[3] = 5;
    invertScale[4] = 7;
    invertScale[5] = 9;
    invertScale[6] = 10;
    invertScale[7] = 12;
    invertAnchor = 50;
    
    reversedValue = 0;
}

JuceSequenceLoopPlayer::~JuceSequenceLoopPlayer(){
    stop();
}

void JuceSequenceLoopPlayer::setSequence(const MidiMessageSequence& targetSequence, int tmpppq){
    std::cout << name << " set sequence " << targetSequence.getNumEvents() << " events" << std::endl;
    originalSequence = targetSequence;
    originalSequence.updateMatchedPairs();
    transformedSequence = targetSequence;
    transformedSequence.updateMatchedPairs();
    ppq = tmpppq;
}
 
 
void JuceSequenceLoopPlayer::reset(){
   // millisCounter = 0;
    //beatMillisCounter = 0;//position of millisCounter at last beat
    beatTick = -1;//counter set every time incoming beat happens
    midiPlayIndex = -1;//index in sequence we have played
    lastTick = 0;
    
}

void JuceSequenceLoopPlayer::stop(){
    
    std::cout << name << "STOP" << std::endl;
    reset();
    
    for (int i = 0; i < 100; i++){//notesSentOut.size(); i++){
        MidiMessage m(128, i, 0);//notesSentOut[i], 0);
         MidiMessage m2(129, i, 0);//notesSentOut[i], 0);
       // std::cout << name << " stop note " << notesSentOut[i] << std::endl;
        if (midiOutDevice != NULL){
            midiOutDevice->sendMessageNow(m);
            midiOutDevice->sendMessageNow(m2);
        }
    }
    notesSentOut.clear();
    
   // printSequenceEvents(transformedSequence);
}

//void newBeat(){
//    tickPosition = getTicksFromBeat(beatPosition);
//}

void JuceSequenceLoopPlayer::updatePlaybackToBeat(int& beatIndex){//, int& millisCount){
   // millisCounter = millisCount;
    std::cout << name << " update to beat " << beatIndex << " beatticks " << beatTick << std::endl;
    beatTick = getTicksFromBeat(beatIndex*ppq);
    std::cout << " after beat ticks " << beatTick << std::endl;
}

double JuceSequenceLoopPlayer::getTicksFromBeat(double beatPositionTicks){
    double tmp = beatPositionTicks;
    double loopWidth = loopEndTicks - loopStartTicks;
    while ((int)beatPositionTicks > loopEndTicks){//vital to have =?
        beatPositionTicks -= loopWidth;
    }
    
    std::cout << "beat position " << tmp << " == " << beatPositionTicks << " for loop ";
    std::cout << loopStartTicks << " - " << loopEndTicks << std::endl;

    return beatPositionTicks;
}

void JuceSequenceLoopPlayer::updateTicksSinceLastBeat(double ticksSinceBeatTick){
    //std::cout << "ticks since beat " << ticksSinceBeatTick << std::endl;
    if (beatTick + ticksSinceBeatTick <= loopEndTicks){
        //simple update to new ticks
        updateMidiPlayPositionToTickPosition(lastTick, beatTick + ticksSinceBeatTick);
    } else {
        //update to end
        updateMidiPlayPositionToTickPosition(lastTick, loopEndTicks);
        
        //schedule note offs
 //       TO DO
        //reformat beattick to before loop
        beatTick -= (loopEndTicks - loopStartTicks);//put beattick back before loop started
        
        //actually when we get new beatIndex, we calculate from that so this is temporary
        std::cout << "LOOP GOING ROUND" << std::endl;
        std::cout << "NEW BEAT TICK IS " << beatTick << "ticks since " << ticksSinceBeatTick << std::endl;
        
        midiPlayIndex = 0;
        //lastTick = loopStartTicks;
        //update from beginning of loop to where we are now

        updateMidiPlayPositionToTickPosition(loopStartTicks, beatTick+ticksSinceBeatTick);//update beginning part of loop
        
    }
}

void JuceSequenceLoopPlayer::sendMessageOut(MidiMessage& m){
    if (m.isNoteOn())
        noteOnValue = m.getNoteNumber();
    midiOutDevice->sendMessageNow(m);
}



void JuceSequenceLoopPlayer::updateMidiPlayPositionToTickPosition(double startTick, double tickPosition){
  
    int numEvents = transformedSequence.getNumEvents();
    
    
    MidiMessageSequence::MidiEventHolder* outputEvent;
    
    while (midiPlayIndex < numEvents && transformedSequence.getEventTime(midiPlayIndex) < tickPosition){
        //we have caught up to where we are
        //std::cout << "play index " << midiPlayIndex << " at time " << trackSequence.getEventTime(midiPlayIndex) << std::endl;
        
        if (midiPlayIndex >= 0 && transformedSequence.getEventTime(midiPlayIndex) >= startTick){
            //only output when position at least greater than start
            outputEvent = transformedSequence.getEventPointer(midiPlayIndex);
            
            if (outputEvent->message.isNoteOnOrOff()){
                int pitch = outputEvent->message.getNoteNumber();
                
                if (midiOutDevice != NULL){
                    std::cout << name << ": Note OUT " << pitch << std::endl;
                    midiOutDevice->sendMessageNow(outputEvent->message);//..Out(outputEvent->message);
                }
                if (outputEvent->message.isNoteOn()){
                    notesSentOut.push_back(pitch);
                    noteOnValue = pitch;
                    
                    int tmp = transformedSequence.getIndexOfMatchingKeyUp(midiPlayIndex);
                    std::cout << "SequenceLooper " << name <<" NOTE ON  " << outputEvent->message.getNoteNumber();
                    std::cout << " index " << midiPlayIndex << " has up key " << tmp;//<< std::endl;
                    std::cout << ", tick position " << tickPosition << "beattick " << beatTick << std::endl;
                } else {
                    std::cout << "SequenceLooper: " << name << " NOTE Off " << pitch << " index " << midiPlayIndex << std::endl;
                    
                    for (int i = 0; i < notesSentOut.size(); i++){
                        if (notesSentOut[i] == pitch)
                            notesSentOut.erase(notesSentOut.begin()+i);
                    }
                }
            }
            else if (outputEvent->message.isController()){
                if (midiOutDevice != NULL)
                    midiOutDevice->sendMessageNow(outputEvent->message);
                
                std::cout << name << "event " << midiPlayIndex << "is controller " << outputEvent->message.getControllerNumber();
                std::cout << " value " << outputEvent->message.getControllerValue() << std::endl;
                
            } else if (outputEvent->message.isPitchWheel()){
                if (midiOutDevice != NULL)
                    midiOutDevice->sendMessageNow(outputEvent->message);
                
                std::cout << "SequenceLooper: " << name << " PITCH WHEEL,  " << outputEvent->message.getPitchWheelValue();
                std::cout << "tick position " << tickPosition << std::endl;
                
            } else if (outputEvent->message.isAftertouch()){
                if (midiOutDevice != NULL)
                    midiOutDevice->sendMessageNow(outputEvent->message);
                
                std::cout << "SequenceLooper: " << name << " AFTERTOUCH ,  " << outputEvent->message.getAfterTouchValue();
                std::cout << "tick position " << tickPosition << std::endl;
            } else {
                
                const uint8* data = outputEvent->message.getRawData();
                if ((data[0] & 0xf0) == 0xd0)
                    std::cout << name << " channel aftertouch  " << midiPlayIndex << " is not note on/off, data[0] " << (int)data[0] << std::endl;
                else
                    std::cout << name << " event " << midiPlayIndex << " is not note on/off, data[0] " << (int)data[0] << std::endl;
            }
        }//end if midiplayindex
        
        midiPlayIndex++;
        
        int64 timenow = juce::Time::currentTimeMillis();
       // std::cout << "sequence looper : timer callback " << timenow << std::endl;
        
    }
    
    lastTick = (int)tickPosition;//(int)(millisCounter * playbackSpeed);//relative to 1ms = 1 tick
    
    outputEvent = NULL;
    delete outputEvent;
    
}




void JuceSequenceLoopPlayer::printSequenceEvents(const MidiMessageSequence& sequence){
    
    std::cout << name << " print sequence has " << sequence.getNumEvents() << " events, ppq " << ppq << std::endl;
    
    MidiMessageSequence::MidiEventHolder* event;//pointer to an individual midi event
    
    for (int i = 0; i < sequence.getNumEvents(); i++){
        //can get this time info from the track sequence
        double eventTime = sequence.getEventTime(i);
        std::cout << name << " print seq[" << i << "]: eventtime " << eventTime;// << std::endl;
        
        //or parse through the events themselves
        event = sequence.getEventPointer(i);
        
        double tmp = event->message.getTimeStamp();
        std::cout << ", timestamp " << tmp ;//<< std::endl;
        
        const uint8* data = event->message.getRawData();
        
        std::cout << " (" <<  event->message.getRawDataSize() << ") ";
        
        if (event->message.isNoteOn())
            std::cout << "note on : ";// << std::endl;
        else if (event->message.isNoteOff())
            std::cout << "note off: ";// << std::endl;
        else
            std::cout << "other   : ";//<< std::endl;
        
        std::cout << (int)data[0] << ", " << (int)data[1] << ", " << (int)data[2];
        
        if (event->message.isNoteOn())
            std::cout << " matches " << sequence.getIndexOfMatchingKeyUp(i) << std::endl;
        else
            std::cout << std::endl;
        
        if (data[0] == 255){
            for (int i = 3; i < event->message.getRawDataSize(); i++){
                //           std::cout << "character[" << i-3 << "] :'" << data[i] << "'" << std::endl;
            }
        }
        
    }//end for
    event = NULL;
    delete event;
    
}


void JuceSequenceLoopPlayer::revertToOriginal(){
    transformedSequence = originalSequence;
    reversedValue = 0;
}


void JuceSequenceLoopPlayer::reverseOriginal(){
    reversedValue = 1;
    reverseSequence(transformedSequence, originalSequence, loopStartTicks, loopEndTicks);
}

void JuceSequenceLoopPlayer::reverseSequence(MidiMessageSequence& reversedSequence, const MidiMessageSequence& sequence, int startStamp, int endStamp){
    
    std::cout << name << "REVERSE " << startStamp << " to " << endStamp << std::endl;
    
    //sequence.updateMatchedPairs();//done?
    
    MidiMessageSequence emptyLoop;
    
    MidiMessageSequence::MidiEventHolder* eventHolder;
    MidiMessageSequence::MidiEventHolder* eventOffHolder;
    
    for (int i = 0; i < sequence.getNumEvents() && sequence.getEventTime(i) <= endStamp; i++){
        int tmpTime = sequence.getEventTime(i);
        
        if (tmpTime >= startStamp && sequence.getIndexOfMatchingKeyUp(i) != -1){//i.e. there is a note off
            eventHolder = sequence.getEventPointer(i);
            eventOffHolder = sequence.getEventPointer(sequence.getIndexOfMatchingKeyUp(i));
            
            if (eventHolder->message.isNoteOn()){
                
                //add in if between start and end
                int offTime = sequence.getEventTime(sequence.getIndexOfMatchingKeyUp(i));
                int duration = offTime-tmpTime;
                std::cout << "event index " << i << ", " << "time " << tmpTime;
                std::cout << " has  off index " << sequence.getIndexOfMatchingKeyUp(i) << " time " << offTime << std::endl;
                std::cout << " note on maps to " << endStamp - tmpTime << ", note off to " << endStamp - tmpTime + duration << std::endl;
                
                const uint8* data = eventHolder->message.getRawData();
                //std::cout << "data[0] " << (int)data[0] << " [1]: " << (int)data[1] << " [2]: " <<    (int)data[2] << std::endl;
                MidiMessage newOnMessage((int)data[0], (int)data[1], (int)data[2], (double)(endStamp - tmpTime));
                
                const uint8* dataOff = eventOffHolder->message.getRawData();
                MidiMessage newOffMessage(dataOff[0], dataOff[1], dataOff[2], (double)(endStamp - tmpTime + duration));
                
                //eventHolder->message.setTimeStamp(endStamp - tmpTime);
                //eventOffHolder->message.setTimeStamp(endStamp - tmpTime + duration);
                //would ignores these timestamps anyhow, set above in the message creation
                emptyLoop.addEvent(newOnMessage);
                emptyLoop.addEvent(newOffMessage);
            }//end if note on
            
        }//end if > start
        
        
    }
    
    
    emptyLoop.updateMatchedPairs();
    
    //so you could reverse a sequence and return it to itself
    //overwriting the original after the transformation
    reversedSequence = emptyLoop;
    
}


int JuceSequenceLoopPlayer::invertPitch(int pitch){
    int startPitch = invertAnchor;
    while (startPitch + 12 <= pitch)
        startPitch += 12;
    
    while (startPitch > pitch)
        startPitch -= 12;
    int i = 0;
    //startpitch now in range
    while (startPitch + invertScale[i] != pitch && i < 7){
        i++;
    }
    std::cout << "start p " << startPitch << " i " << i << std::endl;
    
    int newPitch = startPitch - (pitch - startPitch);
    
    if (startPitch+invertScale[i] == pitch){
        std::cout << "return " << (startPitch - 12 + invertScale[7-i]) << std::endl;
        newPitch = (startPitch - 12 + invertScale[7-i]);
    }
    else {
        //stick with simple inversion from anchor, not in scale
        std::cout << "note not in scale " << pitch << " return " << newPitch << std::endl;
        return newPitch;
    }
    return newPitch;
}

void JuceSequenceLoopPlayer::invertOriginal(){
    invertSequence(transformedSequence, originalSequence, loopStartTicks, loopEndTicks);
    transformedSequence.updateMatchedPairs();
}

void JuceSequenceLoopPlayer::invertSequence(MidiMessageSequence& invertedSequence, const MidiMessageSequence& sequence, int startStamp, int endStamp){
    
    std::cout << name << "INVERT " << startStamp << " to " << endStamp << std::endl;
    
    //sequence.updateMatchedPairs();//done?
    
    MidiMessageSequence emptyLoop;
    
    MidiMessageSequence::MidiEventHolder* eventHolder;
    MidiMessageSequence::MidiEventHolder* eventOffHolder;
    
    for (int i = 0; i < sequence.getNumEvents() && sequence.getEventTime(i) <= endStamp; i++){
        int tmpTime = sequence.getEventTime(i);
        
        if (tmpTime >= startStamp && sequence.getIndexOfMatchingKeyUp(i) != -1){//i.e. there is a note off
            eventHolder = sequence.getEventPointer(i);
            eventOffHolder = sequence.getEventPointer(sequence.getIndexOfMatchingKeyUp(i));
            
            if (eventHolder->message.isNoteOn()){
                
                //add in if between start and end
                int offTime = sequence.getEventTime(sequence.getIndexOfMatchingKeyUp(i));
                int pitch = eventHolder->message.getNoteNumber();
                std::cout << "pitch " << pitch << std::endl;
                int newPitch = invertPitch(pitch);
                
                //int duration = offTime-tmpTime;
                std::cout << "event index " << i << ", " << "time " << tmpTime;
                std::cout << " pitch " << pitch << " maps to " << newPitch << std::endl;
                
                
                const uint8* data = eventHolder->message.getRawData();
                //std::cout << "data[0] " << (int)data[0] << " [1]: " << (int)data[1] << " [2]: " <<    (int)data[2] << std::endl;
                MidiMessage newOnMessage((int)data[0], newPitch, (int)data[2], tmpTime);
                
                const uint8* dataOff = eventOffHolder->message.getRawData();
                MidiMessage newOffMessage(dataOff[0], newPitch, dataOff[2], offTime);
                
                emptyLoop.addEvent(newOnMessage);
                emptyLoop.addEvent(newOffMessage);
            }//end if note on
            
        }//end if > start
        
        
    }
    
    
    emptyLoop.updateMatchedPairs();
    
    //so you could reverse a sequence and return it to itself
    //overwriting the original after the transformation
    invertedSequence = emptyLoop;
    
}


void transposeSequence(int notesInScale){
    
    
}
