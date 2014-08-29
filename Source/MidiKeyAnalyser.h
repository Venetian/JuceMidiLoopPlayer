//
//  MidiKeyAnalyser.h
//  JuceAbletonMidiFilePlayer
//
//  Created by Andrew Robertson on 27/08/2014.
//
//

#ifndef __JuceAbletonMidiFilePlayer__MidiKeyAnalyser__
#define __JuceAbletonMidiFilePlayer__MidiKeyAnalyser__

#include <iostream>

#include "../JuceLibraryCode/JuceHeader.h"

class MidiKeyAnalyser{
public:
    
    std::vector<float> bluesWeighting;
    std::vector<float> keyResult;
    
    
    int analyseKey(MidiMessageSequence& sequence){
        
        sequence.updateMatchedPairs();
        
        keyResult.assign(12, 0.0);
        for (int i = 0; i < sequence.getNumEvents(); i++){
            if (sequence.getEventPointer(i)->message.isNoteOn()){
                int pitch = sequence.getEventPointer(i)->message.getNoteNumber();
                for (int b = 0; b < 12; b++){
                    
                    //keyresult[b] += bluesWeight(pitch, b, getDuration(i));
                }
            }
        }
        return 0;
    }
    
    
    int getDuration(MidiMessageSequence& sequence, int index){
        int offindex = sequence.getIndexOfMatchingKeyUp(index);
        if (offindex != -1){
            int ticksOn = sequence.getEventPointer(index)->message.getTimeStamp();
            int ticksOff = sequence.getEventPointer(offindex)->message.getTimeStamp();
            if (ticksOff > ticksOn)
                return ticksOff-ticksOn;
            else
                return 240;
            
        } else
            return 240;
        
    }
};
#endif /* defined(__JuceAbletonMidiFilePlayer__MidiKeyAnalyser__) */
