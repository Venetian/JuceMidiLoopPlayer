//
//  JuceMidiFilePlayer.cpp
//  JuceAbletonMidiFilePlayer
//
//  Created by Andrew Robertson on 31/07/2014.
//
//

#include "JuceMidiFilePlayer.h"

#define COPY_TRACK_CHANNEL 1

#include <sys/time.h>
/*
TO DO:
bug on playing - check how first bar gets played

scheduling with MidiBuffer

transforms
 
 //storing  data about last beat times
 //easier than accessing array every beat calculation
 */

JuceMidiFilePlayer::JuceMidiFilePlayer(){
    playbackSpeed = 1.0;
    
    String location;
    int fileToLoad = 4;
    switch (fileToLoad){
        case 0:
           location = "../../../../exampleMidiFiles/midiScale.mid";
            break;
        case 1:
            location = "../../../../exampleMidiFiles/Fugue1.mid";//midiScale.mid";
            break;
        case 2:
            location = "../../../../exampleMidiFiles/Arabesque I.mid";//Fugue1.mid";//midiScale.mid";
            break;
        case 3:
            location = "../../../../exampleMidiFiles/tauras_Bassline.mid";
            break;
        case 4:
            location = "../../../../exampleMidiFiles/KingKongBassline.mid";
            break;
    }
    prophet.name = "PROPHET";
    looper.name = "MOOG";
    midiDevice = NULL;
    
    MidiMessageSequence loadedSequence = loadMidiFile(location, true);//true for merging all
   
    prophet.setSequence(loadedSequence, ppq);
    
    looper.setSequence(loadedSequence, ppq);
    looper.printSequenceEvents(looper.transformedSequence);
    std::cout << "LOADED" << std::endl;
    
//    looper.reverseOriginal();
    looper.invertOriginal();
    looper.printSequenceEvents(looper.transformedSequence);
    
    //important so we start clock and midi
    //is there better way for this?
    lastAltBeatIndex = 0;
    
}

JuceMidiFilePlayer::~JuceMidiFilePlayer(){
    stopTimer();
    

    
    for (int i = 32; i < 96; i++){
        MidiMessage offmsg = MidiMessage::noteOff(COPY_TRACK_CHANNEL, i);
        if (midiDevice != NULL)
            midiDevice->sendMessageNow(offmsg);
    }
}

void JuceMidiFilePlayer::startMidiPlayback(){
   
    reset();
    startTimer(1);
    std::cout << "midi play start" << std::endl;
}

void JuceMidiFilePlayer::reset(){
    millisCounter = 0;
    beatMillisCounter = 0;//position of millisCounter at last beat
    beatTick = 0;//counter set every time incoming beat happens
    midiPlayIndex = -1;//index in sequence we have played
    
    //important that at start we reset all clocks
    //the beat times are stored by these three variables
    lastAltBeatTimeMillis = 0;
    lastAltBeatTimeTicks = 0;
    lastAltBeatIndex = 0;
    
    beatsReceived.clear();
    beatPeriod = 500;//in case
}

void JuceMidiFilePlayer::stopMidiPlayback(){
    std::cout << "stop" << std::endl;
    
    stopTimer();
    //this is our trigger used to start again
    lastAltBeatIndex = -1;
    
    looper.stop();
    prophet.stop();
    
    for (int i = 0; i < beatsReceived.size(); i++){
        std::cout << beatsReceived[i].index << " tempo " << beatsReceived[i].tempo << ", systime " << beatsReceived[i].systemTime << std::endl;
    }
    beatsReceived.clear();
    
    
}

//this is called from Ableton via oscAbletonFinder in Juce
//we then need to do our own clocking, knowing the Ableton tempo
//to work out what MIDI notes need scheduling when
void JuceMidiFilePlayer::newBeat(float beatIndex, float tempoMillis, int latency){
    if (beatIndex == -1){
        stopMidiPlayback();
        return;
    }

    std::cout << "lastaltbeat " << lastAltBeatIndex << ", beat index " << beatIndex << std::endl;
    if (lastAltBeatIndex == -1){

        startMidiPlayback();
        setTempo(tempoMillis);
    } else {
        std::cout << "lastalt " << lastAltBeatIndex << std::endl;
    }
    
    alternativeBeatCall(beatIndex, tempoMillis, latency);//the new zero indexed beats
    
    
    /*
    beatIndex--;//because we count from zero!
    if (beatIndex == 0){
        setTempo(tempoMillis);
        startMidiPlayback();//alternative methods here too
        updatePlaybackToBeat(0);//
    } else {
        updatePlaybackToBeat(beatIndex);
        setTempo(tempoMillis);
    }
    */
}



void JuceMidiFilePlayer::updatePlaybackToBeat(int beatIndex){
    //THIS CLASS
    //updateMidiPlayPositionToTickPosition(beatIndex*ppq);
    
   
   // looper.getTicksFromBeat(beatIndex*ppq);defunkt
    looper.updatePlaybackToBeat(beatIndex);//, millisCounter);
    
    prophet.updatePlaybackToBeat(beatIndex);
    
    beatMillisCounter = millisCounter;
    beatTick = beatIndex*ppq;
    

}

unsigned long JuceMidiFilePlayer::systemTime(){
	struct timeval now;
	gettimeofday( &now, NULL );
	unsigned long timenow = (unsigned long long) now.tv_usec/1000 + (unsigned long long) now.tv_sec*1000;
    //std::cout << "system time now " << timenow << " truncated " << (int)timenow << std::endl;
    return timenow;
}

void JuceMidiFilePlayer::alternativeBeatCall(float& beatIndex, float& tempoMillis, int& latency){
    unsigned long timenow = systemTime();
    timenow -= latency;
    
    //we can check the time here - if there was latency - eg over network - we would eliminate it
    //by scheduling accordingly
    //for osc between programs on the same computer, zero latency
    
    float beatEstimate = millisToBeats(millisCounter);
    
    AbletonBeat newBeat;
    newBeat.index = beatIndex;
    newBeat.tempo = tempoMillis;
    newBeat.systemTime = timenow;

    newBeat.millis = millisCounter - latency;//in case it takes some time to arrive
    
    std::cout << "beat period " << beatPeriod << std::endl;
    
    newBeat.ticks = millisToTicks(millisCounter-latency);
    //update our recent beat variables so clock count and prediction from
    //beats received is accurate
    lastAltBeatIndex = beatIndex;
    lastAltBeatTimeMillis = newBeat.millis;//put after ticks to millis
    lastAltBeatTimeTicks = newBeat.ticks;//again after the other setting and before pushing this beat back
    beatPeriod = tempoMillis;//after calc?
    beatsReceived.push_back(newBeat);
    
    std::cout << "alt beat " << beatIndex << " tempo " << tempoMillis << " sys time " << timenow << " millis counter " << newBeat.millis << " ticks " << newBeat.ticks << ", beat estimate " << beatEstimate << std::endl;
    
    //not calling the looper yet
//    looper.alternativeUpdateToBeat()
    
}

float JuceMidiFilePlayer::millisToTicks(int millis){
    //get last beat and ticks there
    float millisElapsed = millis - lastAltBeatTimeMillis;
    //at current tempo in terms of ticks, this is
    float ticksElapsed = (ppq*millisElapsed) /beatPeriod;
    
    return (ticksElapsed + lastAltBeatTimeTicks);

}


float JuceMidiFilePlayer::millisToBeats(const float& millis){
    //get last beat and ticks there
    //float millisElapsed = (millis - lastAltBeatTimeMillis);
    //at current tempo in terms of ticks, this is
  //  float beatsElapsed = (millis - lastAltBeatTimeMillis) /beatPeriod;
    //return (beatsElapsed + lastAltBeatIndex);
    
    //i.e.
    return ((millis - lastAltBeatTimeMillis)/beatPeriod)+lastAltBeatIndex;
}





void JuceMidiFilePlayer::setTempo(float tempoMillis){
    //millis counter called once per millisecond
    //ticks is ppq per beat
    //beat period here
    if (tempoMillis > 0)
        playbackSpeed = ppq/tempoMillis;
    else
        playbackSpeed = 1.0;//
}


void JuceMidiFilePlayer::updateMidiPlayPosition(){
    //main fn called by clock
    
    //called every millis by the clock
    millisCounter++;
    
    updateMidiPlayPositionToTickPosition(beatTick + ((millisCounter-beatMillisCounter)*playbackSpeed));
 
    //std::cout << "counter " << millisCounter << std::endl;
    
    //alternative methd with beats
    //turn our clock here into a beat time
    //millis to beats does that using the recent ableton info via osc
    //starting at zero!
    looper.alternativeUpdateToBeat(millisToBeats(millisCounter));
    
   // dont think this one ever would be right function - delete
    //looper.alternativeUpdateToBeat(millisToBeats(millisCounter));
}


void JuceMidiFilePlayer::updateMidiPlayPositionToTickPosition(double position){
    //THIS CLASS
    //updateMidiPlayPositionToTickPosition(loopSequence, position);
    
    //time in ticks since beattick is position - beattick
 //for looper
    position -= beatTick;
    looper.updateTicksSinceLastBeat(position);//i.e. position-beatTick , time since beat happened
    prophet.updateTicksSinceLastBeat(position);
}

/*
void JuceMidiFilePlayer::updateMidiPlayPositionToTickPosition(MidiMessageSequence& sequence, float tickPosition){
    
     //this not needed - we already have index from the last time
     //could be good check though?
    // int tmpCounter = (int)(millisCounter * playbackSpeed);
    // while (midiPlayIndex < numEvents && trackSequence.getEventTime(midiPlayIndex) < tmpCounter){
     //we have caught up to where we are, maybe we should be optimising this though from last time it was called?
    // midiPlayIndex++;
    // }
     
    
    int numEvents = sequence.getNumEvents();
    int useCount = (int)tickPosition;//(int)(millisCounter * playbackSpeed);//relative to 1ms = 1 tick
    
    MidiMessageSequence::MidiEventHolder* outputEvent;
    
    while (midiPlayIndex < numEvents && sequence.getEventTime(midiPlayIndex) < useCount){
        //we have caught up to where we are
        // index++;
        //std::cout << "play index " << midiPlayIndex << " at time " << trackSequence.getEventTime(midiPlayIndex) << std::endl;
        
        if (midiPlayIndex >= 0){
            
            outputEvent = sequence.getEventPointer(midiPlayIndex);
            
            if (outputEvent->message.isNoteOnOrOff()){
                if (midiDevice != NULL)
                    midiDevice->sendMessageNow(outputEvent->message);
                
                if (outputEvent->message.isNoteOn()){
                    int tmp = sequence.getIndexOfMatchingKeyUp(midiPlayIndex);
                    std::cout << "MidiFilePlayer: NOTE ON,  index " << midiPlayIndex << " has up key " << tmp << std::endl;
                    std::cout << "tick position " << useCount << std::endl;
                } else {
                    std::cout << "NOTE Off, index " << midiPlayIndex << std::endl;
                }
            }
            else
                std::cout << "event " << midiPlayIndex << " is not note on/off" << std::endl;
            
        }//end if midiplayindex
        
        midiPlayIndex++;
        
        int64 timenow = juce::Time::currentTimeMillis();
        std::cout << "timer callback " << timenow << std::endl;
        
    }
    
    outputEvent = NULL;
    delete outputEvent;
    
}
*/

//update a buffer to play
//store an interator
//when our iterator hits the end or the next event is beyond

/*
void JuceMidiFilePlayer::updateMidiPlayPositionToTickPositionWithLooping(MidiMessageSequence& sequence, float tickPosition){
    
    //have a loop buffer
    //and an iterator
    //position in the buffer
    
    
    int numEvents = sequence.getNumEvents();
    int useCount = (int)tickPosition;//(int)(millisCounter * playbackSpeed);//relative to 1ms = 1 tick
    
    MidiMessageSequence::MidiEventHolder* outputEvent;
    
    while (midiPlayIndex < numEvents && sequence.getEventTime(midiPlayIndex) < useCount){
        //we have caught up to where we are
        // index++;
        //std::cout << "play index " << midiPlayIndex << " at time " << trackSequence.getEventTime(midiPlayIndex) << std::endl;
        
        if (midiPlayIndex >= 0){
            
            outputEvent = sequence.getEventPointer(midiPlayIndex);
            
            if (outputEvent->message.isNoteOnOrOff()){
                if (midiDevice != NULL)
                    midiDevice->sendMessageNow(outputEvent->message);
                
                if (outputEvent->message.isNoteOn()){
                    int tmp = sequence.getIndexOfMatchingKeyUp(midiPlayIndex);
                    std::cout << "NOTE ON,  index " << midiPlayIndex << " has up key " << tmp << std::endl;
                    std::cout << "tick position " << useCount << std::endl;
                } else {
                    std::cout << "NOTE Off, index " << midiPlayIndex << std::endl;
                }
            }
            else
                std::cout << "event " << midiPlayIndex << " is not note on/off" << std::endl;
            
        }//end if midiplayindex
        
        midiPlayIndex++;
        
        int64 timenow = juce::Time::currentTimeMillis();
        std::cout << "timer callback " << timenow << std::endl;
        
    }
    
    outputEvent = NULL;
    delete outputEvent;
    
}

*/

MidiMessageSequence JuceMidiFilePlayer::loadMidiFile(String fileLocation, bool mergeOn){
    
    //either add exmaplemidiFiles to your project or change this location
    File file(fileLocation);
    
    //TO DO: add some kind of check whether file exists!
    
    MidiMessageSequence loadedSequence;
    
    if (!file.isDirectory())
    {
        FileInputStream fileInput(file);
        
        
        midiFile.readFrom(fileInput);
        
       // loopSequence.clear();

        //num tracks returns an int
        std::cout << "num tracks " << midiFile.getNumTracks() << std::endl;
        
        ppq = midiFile.getTimeFormat();
        std::cout << "time format " << ppq << std::endl;
        
        
        
        
        std::cout << "Last Timestamp " << midiFile.getLastTimestamp() << std::endl;
        
        for (int trackIndex = 0; trackIndex < midiFile.getNumTracks(); trackIndex++){
            
            MidiMessageSequence emptySequence;
            
            if (trackIndex < midiFile.getNumTracks()){
                
                //midi message sequence is pointer to a midi track, as loaded from a file
                trackSequence = *midiFile.getTrack(trackIndex);//replaces empty holder with track 0
                
                /*
                if (trackIndex == COPY_TRACK_CHANNEL || loopSequence.getNumEvents() == 0)
                    loopSequence = MidiMessageSequence(trackSequence);
                else {
                    //doesnt work for now - we only copy channel 0
                    
                    //loopSequence.addSequence(trackSequence, 0, 0, midiFile.getLastTimestamp());
                   // trackSequence.extractMidiChannelMessages(trackIndex, loopSequence, false);
                    //loopSequence.sort();
                    //loopSequence.updateMatchedPairs();
                }
                 */
                
                std::cout << "LOADER: print midi track sequence as loaded, trackindex " << trackIndex << std::endl;
                //printSequenceEvents(trackSequence);
            
                //trackSequence.deleteSysExMessages();
            
                std::cout << "\ntrack 0 has " << trackSequence.getNumEvents() << " events" << std::endl;
                
                MidiMessageSequence::MidiEventHolder* event;//pointer to an individual midi event
                
                for (int i = 0; i < trackSequence.getNumEvents(); i++){
                    //can get this time info from the track sequence
                    double eventTime = trackSequence.getEventTime(i);
                    std::cout << "Loading: event " << i << ": time " << eventTime;//<< std::endl;
                    
                    //or parse through the events themselves
                    event = trackSequence.getEventPointer(i);
                    
                    double tmp = event->message.getTimeStamp();
                    std::cout << " t_stamp " << tmp << " ";//std::endl;
                    
                    const uint8* data = event->message.getRawData();
                    
                    std::cout << "size " <<  event->message.getRawDataSize() << ", ";// << std::endl;
                    
                    std::cout << "chnl " << event->message.getChannel() << ", ";
                    
                    if (event->message.isNoteOn())
                        std::cout << "note on  ";//<< std::endl;
                    else if (event->message.isNoteOff())
                        std::cout << "note off ";// << std::endl;
                    else
                        std::cout << "other    ";// << std::endl;
                    
                    if (event->message.isNoteOnOrOff()){
                        MidiMessage tmpMsg(data[0], data[1], data[2], tmp);
                       
                       
                        //event->message.setChannel(1);
                        emptySequence.addEvent(tmpMsg);//event->message);
                    }
                    
                    std::cout << "data " << (int)data[0] << ", ";// std::endl;
                    std::cout << (int)data[1] << ", ";// std::endl;
                    std::cout << (int)data[2] << std::endl;
                    
                    if (data[0] == 255){
                        for (int i = 3; i < event->message.getRawDataSize(); i++){
                            std::cout << "char[" << i-3 << "] :'" << data[i] << "'" << std::endl;
                        }
                    }
                    
                }//end for i
                
                trackSequence.updateMatchedPairs();
                emptySequence.updateMatchedPairs();
                
                loadedSequence.addSequence(emptySequence, 0, 0, midiFile.getLastTimestamp());
                
                //loopSequence.addSequence(trackSequence, 0, 0, 100000000);
               // for (int channel = 0; channel < 16; channel++)
               //     trackSequence.extractMidiChannelMessages(channel, loopSequence, false);
            
                event = NULL;
                delete event;
            }//end if
           
           // loopSequence.updateMatchedPairs();
           // loopSequence.sort();
            
        }//end for track index

      
        return loadedSequence;
    /*
        std::cout << "LOOP seq " << std::endl;
        printSequenceEvents(loopSequence);
        
        
        std::cout << "\nREVERSING LOOP\n" << std::endl;
        reverseSequence(loopSequence, 0, 4*ppq);
      */
        
    }//end main if not dir
    
}

void JuceMidiFilePlayer::printSequenceEvents(const MidiMessageSequence& sequence){
    
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


void JuceMidiFilePlayer::reverseSequence(MidiMessageSequence& sequence, int startStamp, int endStamp){
    std::cout << "reverse " << startStamp << " to " << endStamp << std::endl;
    sequence.updateMatchedPairs();
    
    MidiMessageSequence emptyLoop;
    
    MidiMessageSequence::MidiEventHolder* eventHolder;
    MidiMessageSequence::MidiEventHolder* eventOffHolder;
    
    for (int i = 0; i < sequence.getNumEvents() && sequence.getEventTime(i) <= endStamp; i++){
        eventHolder = sequence.getEventPointer(i);
        eventOffHolder = sequence.getEventPointer(sequence.getIndexOfMatchingKeyUp(i));
        
        if (eventHolder->message.isNoteOn()){
            
            //add in if between start and end
            
            int tmpTime = sequence.getEventTime(i);
            int offTime = sequence.getEventTime(sequence.getIndexOfMatchingKeyUp(i));
            int duration = offTime-tmpTime;
            std::cout << "event index " << i << ", " << "time " << tmpTime;
            std::cout << " has  off index " << sequence.getIndexOfMatchingKeyUp(i) << " time " << offTime << std::endl;
            
            std::cout << " note on maps to " << endStamp - tmpTime << ", note off to " << endStamp - tmpTime + duration << std::endl;
            
            const uint8* data = eventHolder->message.getRawData();
            std::cout << "data[0] " << (int)data[0] << " [1]: " << (int)data[1] << " [2]: " <<    (int)data[2] << std::endl;
            MidiMessage newOnMessage((int)data[0], (int)data[1], (int)data[2], (double)(endStamp - tmpTime));
            
            const uint8* dataOff = eventOffHolder->message.getRawData();
            MidiMessage newOffMessage(dataOff[0], dataOff[1], dataOff[2], (double)(endStamp - tmpTime + duration));
             
           
            //eventHolder->message.setTimeStamp(endStamp - tmpTime);
            //eventOffHolder->message.setTimeStamp(endStamp - tmpTime + duration);
            //ignores these anyhow, set in the message creation
            emptyLoop.addEvent(newOnMessage);
            emptyLoop.addEvent(newOffMessage);
               
            
        }
        
    }
  //  sequence.updateMatchedPairs();
   // sequence.sort();


    
    std::cout << " end reverse " << std::endl;
    printSequenceEvents(sequence);

   
    emptyLoop.updateMatchedPairs();
    std::cout << " REVERSED EMPTY LOOP" << std::endl;
    printSequenceEvents(emptyLoop);
    
//    loopSequence = emptyLoop;
   // mutatedSequence = emptyLoop;

   //reversed
    looper.setSequence(emptyLoop, ppq);
    std::cout << "LOOPER" << std::endl;
    looper.reverseOriginal();
    
    looper.printSequenceEvents(looper.transformedSequence);
    
    prophet.setSequence(sequence, ppq);
    std::cout << "PROPHET" << std::endl;
//prophet.printSequenceEvents(prophet.transformedSequence);
    
}

void JuceMidiFilePlayer::bufferTest(){
    
    MidiMessage newMessage(144,60,100,(double)250);
    MidiMessage offMessage(128,60,0,(double)350);MidiBuffer buffer(newMessage);
    
    MidiMessage nextMessage(144,60,100,250);
    buffer.addEvent(nextMessage, 350);
    
    
    
    buffer.addEvent(offMessage, 460);
    
    MidiBuffer::Iterator iter(buffer);
    MidiMessage m(0xf0);
    int sample;
    
    
    std::cout << "BUFFER" << std::endl;
    
    

    while (iter.getNextEvent(m, sample)){
        if (m.isNoteOn()){
            std::cout << "buffer note on " << m.getNoteNumber() << " VEL " <<  (int) m.getVelocity();
            std::cout << " time " << m.getTimeStamp() << std::endl;
            const uint8* data = m.getRawData();
            std::cout << "data[0] " << (int)data[0] << " [1]: " << (int)data[1] << " [2]: " <<    (int)data[2] << std::endl;
            
            } else if (m.isNoteOff()){
                std::cout << "buffer note off " << m.getNoteNumber() << " VEL " <<  (int) m.getVelocity();
                std::cout << " time " << m.getTimeStamp() << std::endl;
                const uint8* data = m.getRawData();
                std::cout << "data[0] " << (int)data[0] << " [1]: " << (int)data[1] << " [2]: " <<    (int)data[2] << std::endl;
                
            }
        }

}

/*
loopTest{
    //emptyLoop.clear();
    MidiMessage newMessage(144,60,100,250);
    MidiMessage offMessage(128,60,0,550);
    
    MidiMessageSequence emptyLoop;//trackSequence);
    emptyLoop.addEvent(newMessage);
    emptyLoop.addEvent(offMessage);
    emptyLoop.updateMatchedPairs();
}
*/