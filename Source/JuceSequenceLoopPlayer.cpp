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



 
JuceSequenceLoopPlayer::JuceSequenceLoopPlayer(){
    loopStartTicks = 0;
    
    beatTick = 0;
    ppq = 480;
    loopEndTicks = ppq*4;//4 beat loop of beginning
    lastTick = 0;
    midiPlayIndex = -1;
    midiOutDevice = NULL;
}

JuceSequenceLoopPlayer::~JuceSequenceLoopPlayer(){

}

void JuceSequenceLoopPlayer::setSequence(const MidiMessageSequence& targetSequence, int tmpppq){
    sequence = targetSequence;
    sequence.updateMatchedPairs();
    ppq = tmpppq;
}
 
 
void JuceSequenceLoopPlayer::reset(){
   // millisCounter = 0;
    //beatMillisCounter = 0;//position of millisCounter at last beat
    beatTick = 0;//counter set every time incoming beat happens
    midiPlayIndex = -1;//index in sequence we have played
    lastTick = 0;
    
}

//void newBeat(){
//    tickPosition = getTicksFromBeat(beatPosition);
//}

void JuceSequenceLoopPlayer::updatePlaybackToBeat(int& beatIndex){//, int& millisCount){
   // millisCounter = millisCount;
    
    beatTick = getTicksFromBeat(beatIndex*ppq);
}

double JuceSequenceLoopPlayer::getTicksFromBeat(double beatPositionTicks){
    double tmp = beatPositionTicks;
    double loopWidth = loopEndTicks - loopStartTicks;
    while (beatPositionTicks > loopEndTicks){
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
        std::cout << "NEW BEAT TICK IS " << beatTick << std::endl;
        
        midiPlayIndex = 0;
        //update from beginning of loop to where we are now
        updateMidiPlayPositionToTickPosition(loopStartTicks, beatTick+ticksSinceBeatTick);//update beginning part of loop
        
    }
}





void JuceSequenceLoopPlayer::updateMidiPlayPositionToTickPosition(double startTick, double tickPosition){
  
    int numEvents = sequence.getNumEvents();
    
    
    MidiMessageSequence::MidiEventHolder* outputEvent;
    
    while (midiPlayIndex < numEvents && sequence.getEventTime(midiPlayIndex) < tickPosition){
        //we have caught up to where we are
        //std::cout << "play index " << midiPlayIndex << " at time " << trackSequence.getEventTime(midiPlayIndex) << std::endl;
        
        if (midiPlayIndex >= 0 && sequence.getEventTime(midiPlayIndex) >= startTick){
            //only output when position at least greater than start
            outputEvent = sequence.getEventPointer(midiPlayIndex);
            
            if (outputEvent->message.isNoteOnOrOff()){
                if (midiOutDevice != NULL)
                    midiOutDevice->sendMessageNow(outputEvent->message);
                
                if (outputEvent->message.isNoteOn()){
                    int tmp = sequence.getIndexOfMatchingKeyUp(midiPlayIndex);
                    std::cout << "SequenceLooper: NOTE ON,  " << outputEvent->message.getNoteNumber() << " index " << midiPlayIndex << " has up key " << tmp << std::endl;
                    std::cout << "tick position " << tickPosition << std::endl;
                } else {
                    std::cout << "NOTE Off, index " << midiPlayIndex << std::endl;
                }
            }
            else
                std::cout << "event " << midiPlayIndex << " is not note on/off" << std::endl;
            
        }//end if midiplayindex
        
        midiPlayIndex++;
        
        int64 timenow = juce::Time::currentTimeMillis();
        std::cout << "sequence looper : timer callback " << timenow << std::endl;
        
    }
    
    lastTick = (int)tickPosition;//(int)(millisCounter * playbackSpeed);//relative to 1ms = 1 tick
    
    outputEvent = NULL;
    delete outputEvent;
    
}




void JuceSequenceLoopPlayer::printSequenceEvents(){
    
    std::cout << "print sequence has " << sequence.getNumEvents() << " events, ppq " << ppq << std::endl;
    
    MidiMessageSequence::MidiEventHolder* event;//pointer to an individual midi event
    
    for (int i = 0; i < sequence.getNumEvents(); i++){
        //can get this time info from the track sequence
        double eventTime = sequence.getEventTime(i);
        std::cout << "print seq[" << i << "]: eventtime " << eventTime;// << std::endl;
        
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
