//
//  JuceMidiKeyPattern.cpp
//  JuceAbletonMidiFilePlayer
//
//  Created by Andrew Robertson on 28/08/2014.
//
//

#include "JuceMidiKeyPattern.h"

//BUG WHERE SOME PATTERNS ARE > BAR END

JuceMidiKeyPattern::JuceMidiKeyPattern(){
    scaleSize = 7;
    bluesScale.assign(scaleSize, 0);
    bluesScale[1] = 2;
    bluesScale[2] = 3;
    //bluesScale[3] = 4;
    bluesScale[3] = 5;
    bluesScale[4] = 7;
    bluesScale[5] = 9;
    bluesScale[6] = 10;

    
}


JuceMidiKeyPattern::~JuceMidiKeyPattern(){

}

void JuceMidiKeyPattern::clearSequence(){
    soloPattern.clear();
}

void JuceMidiKeyPattern::analyseSequence(MidiMessageSequence& sequence, int key){
    
    
    int barIndex = 1;
    int barTime = 0;
    RhythmicPattern pattern;
    sequence.updateMatchedPairs();
    
    for (int i = 0; i < sequence.getNumEvents(); i++){
        if (sequence.getEventPointer(i)->message.isNoteOn()){
            int notePitch = sequence.getEventPointer(i)->message.getNoteNumber();
            std::cout << "note " << i << ", pitch " << sequence.getEventPointer(i)->message.getNoteNumber() << std::endl;
            int p = findPitch(sequence.getEventPointer(i)->message.getNoteNumber(), key);
            int q = findOctave(sequence.getEventPointer(i)->message.getNoteNumber(), key);
            float beatTime = sequence.getEventTime(i);
            
            std::cout << "pitch: octave " << q << ", pitch [" << p << "], time " << beatTime << std::endl;
            int transpose = 0;
            int newPitch = getPitch(q, p+transpose, key);
            float duration = 0;
            int velocity = sequence.getEventPointer(i)->message.getVelocity();
            sequence.getEventPointer(i)->message.setNoteNumber(newPitch);
            if (sequence.getIndexOfMatchingKeyUp(i) != -1){
                sequence.getEventPointer(sequence.getIndexOfMatchingKeyUp(i))->message.setNoteNumber(newPitch);
                duration = sequence.getEventTime(sequence.getIndexOfMatchingKeyUp(i)) - beatTime;
                std::cout << "note " << notePitch << " duration " << duration << std::endl;
            }
            RhythmicNote r;
            //setPitchAndOctave()
            r.pitch = p;
            r.octave = q;
            r.beatPosition = beatTime - barTime;
            r.sharp = 0;
            r.duration = duration;
            r.velocity = velocity;
            //for now!
            
            if (beatTime >= barIndex*4){
                //push back
                soloPattern.push_back(pattern);
                //new bar object
                pattern.clear();
                barTime = barIndex*4;//assume 4/4 for now
                r.beatPosition = beatTime - barTime;//re-do
                pattern.push_back(r);//start new one
                
                barIndex++;
                std::cout << "PUSH_BACK PATTERN" << std::endl;
            } else {
                pattern.push_back(r);
            }
            
        }
    
    }
    
    
    for (int j = 0; j < soloPattern.size(); j++){
        std::cout << "BAR " << j << " has " << soloPattern[j].size() << " elements" << std::endl;
        for (int i = 0; i < soloPattern[j].size(); i++){
            std::cout << "NOTE [" << soloPattern[j][i].octave << "][" << soloPattern[j][i].pitch << "]: " << soloPattern[j][i].beatPosition << " dur " << soloPattern[j][i].duration << std::endl;
        }
    }
}


void JuceMidiKeyPattern::addToMidiSequence(MidiMessageSequence& sequence, float startTime, int patternIndex){
    for (int i = 0; i < soloPattern[patternIndex].size(); i++){
        //int p = soloPattern[patternIndex][i].pitch;
        //int q = soloPattern[patternIndex][i].octave;
        float t = soloPattern[patternIndex][i].beatPosition;
        float endTime = t + soloPattern[patternIndex][i].duration;
        //add note on
        int pitch = getPitchFromScaleNote(soloPattern[patternIndex][i].pitch, soloPattern[patternIndex][i].octave, soloPattern[patternIndex][i].sharp);
        MidiMessage m(144, pitch, soloPattern[patternIndex][i].velocity);
        m.setTimeStamp(startTime + t);
        sequence.addEvent(m);
        
        //add note off
        MidiMessage m_off(128, pitch, 0);
        m_off.setTimeStamp(startTime + endTime);
        sequence.addEvent(m_off);
    }
    sequence.updateMatchedPairs();
}

int JuceMidiKeyPattern::getPitchFromScaleNote(int index, int octave, int sharp){
    return bluesScale[index] + octave*12 + sharp;
}

int JuceMidiKeyPattern::getPitch(int octave, int noteIndex, int key){
    if (noteIndex < 0){
        noteIndex += scaleSize;
        octave--;
    }
    
    if (noteIndex > scaleSize){
        noteIndex -= scaleSize;
        octave++;
    }
    return (octave*12) + key + bluesScale[noteIndex];
}

int JuceMidiKeyPattern::findPitch(int pitch, int key){
    int p = scale(pitch, key);
    std::cout << "pitch " << pitch << ", key " << key << ", scale " << p << std::endl;
    int i = 0;
    while (bluesScale[i] < p)
        i++;
    if (bluesScale[i] != p)
        std::cout << "augmented " << bluesScale[i] << " compared to " << p << std::endl;
    return i;
}

int JuceMidiKeyPattern::findOctave(int pitch, int key){
    pitch -= scale(pitch, key);
    return pitch/12;
}

int JuceMidiKeyPattern::scale(int p, int q){
    
    while (p > 12)
        p -= 12;
    while (p < q)
        p+=12;
    return p-q;
}
