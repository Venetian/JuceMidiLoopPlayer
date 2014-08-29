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
    
    //set up pointers in looper
    //not particularly keen on this system but will work fine as long as the midi player setup is called here
    //it watches out for the timing here - tempo and millis counter - to schedule note off events
    setUp(looper);
    setUp(prophet);
    
    
    playbackSpeed = 1.0;
    
    String location;
    String moogLocation = "";
    
    int key = 0;
    int soloKey = 0;
    
    
    int fileToLoad = 7;
    
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
            moogLocation = "../../../../exampleMidiFiles/KingKongBassline.mid";
            break;
        case 5:
            location = "../../../../exampleMidiFiles/TaurusBasslineHalftime.mid";
            moogLocation = location;
            key = 2;//?
            break;
        case 6:
            moogLocation = "/Users/andrewrobertson/Music/HigamosSynchotron/elkamonia research/elka bass notes midi.mid";
            key = 0;
            break;
        case 7:
            moogLocation = "/Users/andrewrobertson/Music/HigamosSynchotron/my trav 90 bpm research/MyTravelBassline.mid";
            key = 4;
            break;
        case 8:
            //location = "/Users/andrewrobertson/Music/HigamosSynchotron/comm_solo.mid";
            moogLocation = "../../../../exampleMidiFiles/KingKongBassline.mid";
            key = 2;
            break;
        case 9:
            location = "/Users/andrewrobertson/Music/HigamosSynchotron/ZeppelinMidi/led_zeppelin-candy_store_rock.mid";
            break;
        case 10:
            moogLocation = "/Users/andrewrobertson/Music/HigamosSynchotron/ZeppelinMidi/led_zeppelin-black_dog.mid";
            key = 9;
            break;
        
            
            
    }
    //zep communication breakdown
    int zepSolo = 3;
    switch (zepSolo){
    case 0:
        location = "/Users/andrewrobertson/Music/HigamosSynchotron/comm_solo.mid";
        soloKey = 4;//e minor
        break;
    case 1:
        location = "/Users/andrewrobertson/Music/Logic/ZepSolos/Darlene_solo.mid";
        soloKey = 2;
        break;
    case 2:
        location = "/Users/andrewrobertson/Music/Logic/ZepSolos/CelebrationDay3_solo.mid";
        soloKey = 9;
        break;
    case 3:
        location = "/Users/andrewrobertson/Music/Logic/ZepSolos/StairwayToHeaven_solo.mid";
        soloKey = 9;
        break;
    case 4:
        location = "/Users/andrewrobertson/Music/Logic/ZepSolos/RoyalOrleans_solo.mid";
        soloKey = 9;
        break;
    case 5:
        location = "/Users/andrewrobertson/Music/Logic/ZepSolos/NobodysFaultButMine_solo.mid";
        soloKey = 4;//or 9??
        break;
    case 6:
        location = "/Users/andrewrobertson/Music/Logic/ZepSolos/richie_hallelujah_solo.mid";
        soloKey = 9;//??
        break;
    case 7:
        location = "/Users/andrewrobertson/Music/Logic/ZepSolos/InMyTimeOfDying_solo.mid";
        soloKey = 9;//a minor
        break;
    case 8:
        location = "/Users/andrewrobertson/Music/Logic/ZepSolos/RambleOn_solo.mid";
        soloKey = 4;
        break;
    default:
        moogLocation = location;
        soloKey = key;
        break;
    }
   
    
    prophet.name = "PROPHET";
    looper.name = "MOOG";
    midiDevice = NULL;
    
    std::cout << "LOAD PROPHET SEQ:" << std::endl;
    MidiMessageSequence loadedSequence = loadMidiFile(location, true);//true for merging all
    
    MidiMessageSequence moogSequence;
    std::cout << "LOAD MOOG SEQ:" << std::endl;
    if (moogLocation != "")
        moogSequence = loadMidiFile(moogLocation, true);
    else
        moogSequence = loadedSequence;
    
    looper.setLoopPointsBeats(0, 16);
    prophet.setLoopPointsBeats(0, 64 );
    
    
    prophet.setSequence(loadedSequence, ppq);
    prophet.transposeSequence((key + 12 - soloKey)%12);
    
    looper.setSequence(moogSequence, ppq);
    
   // looper.printSequenceEvents(looper.transformedSequence);
    std::cout << "LOADED" << std::endl;
    
    //looper.reverseOriginal();
    //looper.invertOriginal();
    looper.printSequenceEvents(looper.beatDefinedSequence, looper.loopEndBeats);
    
    //important so we start clock and midi
    //is there better way for this?
    lastAltBeatIndex = -1;
    beatPeriod = 500;
    lastAltBeatTimeMillis = 0;
    
    
    
    prophet.midiViewer.setBounds(60, 350, 320, 100);
    looper.midiViewer.setBounds(400, 350, 320, 100);

    //to view??
    looper.viewerValue = &midiViewerValue;
    prophet.viewerValue = &midiViewerValue;
    
    patternSequencer.setLoopPoints(looper.loopStartBeats, looper.loopEndBeats);
    patternSequencer.loadSequence(looper.beatDefinedSequence);
    
    std::cout << "KEY ANALYSIS, key " << soloKey << std::endl;
    /*
    keyPatternAnalyser.clearSequence();//call before analysis;
    keyPatternAnalyser.analyseSequence(prophet.beatDefinedSequence, soloKey);
    
    MidiMessageSequence testSequence;
    int numPatterns = keyPatternAnalyser.soloPattern.size();
    Random rnd;
    
    for (int bar = 0; bar < 8; bar++){
        int pattern = std::abs(rnd.nextInt())%numPatterns;
        keyPatternAnalyser.addToMidiSequence(testSequence, bar*4.0, pattern);
        std::cout << "TEST_SEQ, bar " << bar << " pattern " << pattern << std::endl;
    }
    printSequenceEvents(testSequence);
    prophet.beatDefinedSequence = testSequence;
    prophet.beatDefinedSequence.updateMatchedPairs();
    prophet.setLoopPointsBeats(0, 32);
    */
    
    
    doZeppelinPatterns();
    
    /*
    std::cout << "swap elements " << std::endl;
    MidiMessage m = patternSequencer.pitchSet[0];
    std::cout << "before " << m.getNoteNumber() << " pitch 0 is " << m.getNoteNumber() << " pitch 1 is " << patternSequencer.pitchSet[1].getNoteNumber() <<  std::endl;
    if (patternSequencer.pitchSet[0].isNoteOn()){
        patternSequencer.pitchSet[0].setNoteNumber(52);//patternSequencer.pitchSet[1].getNoteNumber());
       // m.setNoteNumber(patternSequencer.pitchSet[0].getNoteNumber());
        patternSequencer.pitchSet[1].setNoteNumber(m.getNoteNumber());
        std::cout << "pitch changed " << m.getNoteNumber() << std::endl;
    }
    //patternSequencer.pitchSet[1].setNoteNumber(m.getNoteNumber());
    std::cout << "after pitch 0 is " << patternSequencer.pitchSet[0].getNoteNumber() << " pitch 1 is " << patternSequencer.pitchSet[1].getNoteNumber() <<  std::endl;
     */
    
    if (1 == 2){
        patternSequencer.pitchSet.swap(0,1);
        patternSequencer.printPitchSet();
        patternSequencer.reorderPitchSetAndRhythm();
    }
    patternSequencer.generateOutputSequence(looper.beatDefinedSequence);
    std::cout << "PATTERN GENERATED " << std::endl;
    printSequenceEvents(looper.beatDefinedSequence);
   // startTimer(1);
    
}


void JuceMidiFilePlayer::doZeppelinPatterns(){
    int soloKey;
    String location;
    keyPatternAnalyser.clearSequence();//call before analysis;
    MidiMessageSequence soloSequence;
    
    location = "/Users/andrewrobertson/Music/HigamosSynchotron/comm_solo.mid";
    soloKey = 4;//e minor
    soloSequence = loadMidiFile(location, true);
    changeTicksToBeats(soloSequence);
    keyPatternAnalyser.analyseSequence(soloSequence, soloKey);
    
    location = "/Users/andrewrobertson/Music/Logic/ZepSolos/Darlene_solo.mid";
    soloKey = 2;
    soloSequence = loadMidiFile(location, true);
    changeTicksToBeats(soloSequence);
    keyPatternAnalyser.analyseSequence(soloSequence, soloKey);
    
    location = "/Users/andrewrobertson/Music/Logic/ZepSolos/CelebrationDay3_solo.mid";
    soloKey = 9;
    soloSequence = loadMidiFile(location, true);
    changeTicksToBeats(soloSequence);
    keyPatternAnalyser.analyseSequence(soloSequence, soloKey);
    
    location = "/Users/andrewrobertson/Music/Logic/ZepSolos/StairwayToHeaven_solo.mid";
    soloKey = 9;
    soloSequence = loadMidiFile(location, true);
    changeTicksToBeats(soloSequence);
    keyPatternAnalyser.analyseSequence(soloSequence, soloKey);
    /*
     location = "/Users/andrewrobertson/Music/Logic/ZepSolos/RoyalOrleans_solo.mid";
     soloKey = 9;
     
     location = "/Users/andrewrobertson/Music/Logic/ZepSolos/NobodysFaultButMine_solo.mid";
     soloKey = 4;//or 9??
     
     location = "/Users/andrewrobertson/Music/Logic/ZepSolos/richie_hallelujah_solo.mid";
     soloKey = 9;//??
     
     location = "/Users/andrewrobertson/Music/Logic/ZepSolos/InMyTimeOfDying_solo.mid";
     soloKey = 9;//a minor
     
     location = "/Users/andrewrobertson/Music/Logic/ZepSolos/RambleOn_solo.mid";
     soloKey = 4;
     */
    
    
    //generate new one
    MidiMessageSequence testSequence;
    int numPatterns = keyPatternAnalyser.soloPattern.size();
    Random rnd;
    
    for (int bar = 0; bar < 16; bar++){
        int pattern = std::abs(rnd.nextInt())%numPatterns;
        keyPatternAnalyser.addToMidiSequence(testSequence, bar*4.0, pattern);
        std::cout << "TEST_SEQ, bar " << bar << " pattern " << pattern << std::endl;
    }
    printSequenceEvents(testSequence);
    prophet.beatDefinedSequence = testSequence;
    prophet.beatDefinedSequence.updateMatchedPairs();
    
    
}

void JuceMidiFilePlayer::resized(){
    looper.resized();
    prophet.resized();
}

void JuceMidiFilePlayer::setUp(JuceSequenceLoopPlayer& player){
    player.milliscounter = &millisCounter;
    player.tempoMillis = &beatPeriod;
    player.setupCorrect = true;
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

    std::cout << "JMFP: midi play start" << std::endl;
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
    std::cout << "JMFP: stop" << std::endl;
    
    //for some odd reason - if we call this
    //we never get past it
    stopTimer();
    
    
    //this is our trigger used to start again
    lastAltBeatIndex = -1;
    
    looper.stop();
    prophet.stop();
    
    std::cout << "JMFP: beats received " << beatsReceived.size() << std::endl;
    for (int i = 0; i < beatsReceived.size(); i++){
        std::cout << beatsReceived[i].index << " tempo " << beatsReceived[i].tempo << ", systime " << beatsReceived[i].systemTime << std::endl;
    }
    beatsReceived.clear();
    
    std::cout << "PROPHET BEAT DEFINED SEQ" << std::endl;
    prophet.printSequenceEvents(prophet.beatDefinedSequence);
    
}

//this is called from Ableton via oscAbletonFinder in Juce
//we then need to do our own clocking, knowing the Ableton tempo
//to work out what MIDI notes need scheduling when
void JuceMidiFilePlayer::newBeat(float beatIndex, float tempoMillis, int latency){
    if (beatIndex == -1){
        std::cout << "JMFP: stop called" << std::endl;
        stopMidiPlayback();//also sets lastAltBeatIndex to -1, ready below to trigger start again
        return;
    } else {

        std::cout << "JMFP: lastaltbeat " << lastAltBeatIndex << ", new beat index " << beatIndex << std::endl;
        if (lastAltBeatIndex == -1){
            std::cout << "JMFP: start midi playback" << std::endl;
            startMidiPlayback();
            setTempo(tempoMillis);
        } else {
            std::cout << "JMFP: lastalt " << lastAltBeatIndex << std::endl;
        }
        
        
        alternativeBeatCall(beatIndex, tempoMillis, latency);//the new zero indexed beats
    
    }
    
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


/*
void JuceMidiFilePlayer::updatePlaybackToBeat(int beatIndex){
   
  //  looper.updatePlaybackToBeat(beatIndex);//, millisCounter);
    
  //  prophet.updatePlaybackToBeat(beatIndex);
    
//    beatMillisCounter = millisCounter;
  //  beatTick = beatIndex*ppq;
    

}
 */

unsigned long long JuceMidiFilePlayer::systemTime(){
	struct timeval now;
	gettimeofday( &now, NULL );
	unsigned long long timenow = (unsigned long long) now.tv_usec/1000 + (unsigned long long) now.tv_sec*1000;
    //std::cout << "system time now " << timenow << " truncated " << (int)timenow << std::endl;
    return timenow;
}

void JuceMidiFilePlayer::alternativeBeatCall(float& beatIndex, float& tempoMillis, int& latency){
    unsigned long long timenow = systemTime();
    timenow -= latency;
    
    //we can check the time here - if there was latency - eg over network - we would eliminate it
    //by scheduling accordingly
    //for osc between programs on the same computer, zero latency
    std::cout << "JMFP: alt beat call millis counter " << millisCounter << std::endl;
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
    //could call a signle update routine?
    //like updateMidiPlayPosition();
  //just does this:
    looper.alternativeUpdateToBeat(beatIndex);
    prophet.alternativeUpdateToBeat(beatIndex);
    
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

float JuceMidiFilePlayer::beatsNow(){
    return millisToBeats(millisCounter);
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
    //main fn called by clock every 1 msec
    
    //called every millis by the clock
    millisCounter++;
    
    //below was the routine called to send out ticks
    //updateMidiPlayPositionToTickPosition(beatTick + ((millisCounter-beatMillisCounter)*playbackSpeed));
 
    //std::cout << "counter " << millisCounter << std::endl;
    
    //alternative methd with beats
    //turn our clock here into a beat time
    //millis to beats does that using the recent ableton info via osc
    //starting at zero!
    
    looper.alternativeUpdateToBeat(millisToBeats(millisCounter));
    prophet.alternativeUpdateToBeat(millisToBeats(millisCounter));
    
}



MidiMessageSequence JuceMidiFilePlayer::loadMidiFile(String fileLocation, bool mergeOn){
    
    //either add exmaplemidiFiles to your project or change this location
    File file(fileLocation);
    
    //TO DO: add some kind of check whether file exists!
    std::cout << "Load '" << fileLocation << "'" << std::endl;
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
        
        
        bool printingInfo = false;
        
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
                std::cout << "track " << trackIndex << " has " << trackSequence.getNumEvents() << " events" << std::endl;
                
                //printSequenceEvents(trackSequence);
            
                //trackSequence.deleteSysExMessages();
                
                MidiMessageSequence::MidiEventHolder* event;//pointer to an individual midi event
                
                for (int i = 0; i < trackSequence.getNumEvents(); i++){
                    //can get this time info from the track sequence
                    double eventTime = trackSequence.getEventTime(i);
                    
                    
                    //or parse through the events themselves
                    event = trackSequence.getEventPointer(i);
                    
                    double tmp = event->message.getTimeStamp();
                    const uint8* data = event->message.getRawData();
                    
                    if (event->message.isNoteOnOrOff()){
                        MidiMessage tmpMsg(data[0], data[1], data[2], tmp);
                        
                        //event->message.setChannel(1);
                        emptySequence.addEvent(tmpMsg);//event->message);
                        
                    }
                    
                    if (printingInfo){
                        std::cout << "Loading: event " << i << ": time " << eventTime;//<< std::endl;
                        std::cout << " t_stamp " << tmp << " ";//std::endl;
                        std::cout << "size " <<  event->message.getRawDataSize() << ", ";// << std::endl;
                        std::cout << "chnl " << event->message.getChannel() << ", ";
                        
                        if (event->message.isNoteOn())
                            std::cout << "note on  ";//<< std::endl;
                        else if (event->message.isNoteOff())
                            std::cout << "note off ";// << std::endl;
                        else
                            std::cout << "other    ";// << std::endl;
                        
                        
                        
                        
                    }
                    if (data[0] == 255){
                        std::cout << "data track " << trackIndex << ", event " << i << ": " << (int)data[0] << ", ";// std::endl;
                        std::cout << (int)data[1] << ", ";// std::endl;
                        std::cout << (int)data[2] << std::endl;
                    
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

      
        filterNotesOfZeroDuration(loadedSequence);
        
        return loadedSequence;
    /*
        std::cout << "LOOP seq " << std::endl;
        printSequenceEvents(loopSequence);
        
        
        std::cout << "\nREVERSING LOOP\n" << std::endl;
        reverseSequence(loopSequence, 0, 4*ppq);
      */
        
    }//end main if not dir
    return loadedSequence;//but it's empty
    
}


void JuceMidiFilePlayer::filterNotesOfZeroDuration(MidiMessageSequence& sequence){
    
    sequence.updateMatchedPairs();
    //printSequenceEvents(sequence);
    
    int i = sequence.getNumEvents();
    i--;
    while (i >= 0){
        if (sequence.getEventPointer(i)->message.isNoteOn()){
            if (sequence.getEventTime(i) == sequence.getEventTime(sequence.getIndexOfMatchingKeyUp(i)) ){
                sequence.deleteEvent(i, true);
                //std::cout << "REMOVE " << i << std::endl;
            }
        }
        i--;
    }
    
    //std::cout << "AFTER REMOVAL " << std::endl;
    //printSequenceEvents(sequence);
}


void JuceMidiFilePlayer::changeTicksToBeats(MidiMessageSequence& sequence){
    
    std::cout <<"\nCHANGE TICKS TO BEATS" << std::endl;
    //    printSequenceEvents(sequence);
    
    //might need to think if there were structural changes - eg to 5/8 or something?
    for (int i = 0 ; i < sequence.getNumEvents(); i++){
        double tmpTime = sequence.getEventTime(i);
        //      std::cout << tmpTime << "-> " << tmpTime/ppq << std::endl;
        sequence.getEventPointer(i)->message.setTimeStamp(tmpTime/ppq);
    }
   
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
    std::cout << "LOOPER BEAT DEFINED SEQUENCE" << std::endl;
    looper.printSequenceEvents(looper.beatDefinedSequence);
    
    prophet.setSequence(sequence, ppq);
    std::cout << "PROPHET" << std::endl;
    //prophet.reverseOriginal();
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



/*
 
 void JuceMidiFilePlayer::updateMidiPlayPositionToTickPosition(double position){
 //THIS CLASS
 //updateMidiPlayPositionToTickPosition(loopSequence, position);
 
 //time in ticks since beattick is position - beattick
 //for looper
 position -= beatTick;
 
 //defunnkt
 looper.updateTicksSinceLastBeat(position);//i.e. position-beatTick , time since beat happened
 prophet.updateTicksSinceLastBeat(position);
 }
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
