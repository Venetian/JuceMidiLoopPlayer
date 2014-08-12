//
//  MidiSequenceViewer.cpp
//  JuceAbletonMidiFilePlayer
//
//  Created by Andrew Robertson on 11/08/2014.
//
//

#include "MidiSequenceViewer.h"

MidiSequenceViewer::MidiSequenceViewer() : relativeViewerBox(0.1,0.3,0.4,0.2){
    viewerGraphics = nullptr;
    viewerBox.setBounds(10.0, 60.0, 20.0, 40.0);
    
    
    loopMax = nullptr;
    loopMin = nullptr;
    
    pitchMin = 40;
    pitchMin = 52;
    
}

MidiSequenceViewer::~MidiSequenceViewer(){
    
}

void MidiSequenceViewer::setBounds(float x, float y, float w, float h){
    if (x > 0 && y > 0 && w > 0 && h > 0){
        viewerBox.setBounds(x,y,w,h);
    }
}

void MidiSequenceViewer::draw(Graphics& g){
    //viewerBox = relativeViewerBox.resolve();
    g.setColour(Colours::white);
    g.drawRect(viewerBox);
    g.fillRect(viewerBox);
    
    if (loopMax != nullptr && loopMin != nullptr)
        redrawMidiSequence(g);
    
   
}

void MidiSequenceViewer::resized(){
    //redrawMidiSequence();
    std::cout << "midi viewer resized " << std::endl;
   
    
//    viewerBox = relativeViewerBox.resolve();

    
}

void MidiSequenceViewer::redrawMidiSequence(Graphics& g){
    std::cout << "midi viewer redraw " << std::endl;
    
    setPitchLimits();
    
    
    float binHeight = viewerBox.getHeight()/(pitchMax - pitchMin);
    float oneNoteWidth = viewerBox.getWidth()/(*loopMax - *loopMin);
   // std::cout << "bin height " << binHeight << " 1 note " << oneNoteWidth << std::endl;
    
    
    for (int j = pitchMin; j <= pitchMax; j++){
        int curPitch = j%12;
        switch (curPitch){
            case 0: case 2: case 4: case 5: case 7: case 9: case 11:
               g.setColour(Colours::white);
                break;
            default:
                g.setColour(Colours::grey);
                break;
        }
        
        g.setOpacity(0.12);
        Rectangle<float> tmpRect (viewerBox.getX(), viewerBox.getY()+viewerBox.getHeight()-(j-pitchMin)*binHeight, viewerBox.getWidth(), binHeight);
        //g.drawRect(tmpRect);
        g.fillRect(tmpRect);
    }
    
    g.setOpacity(1.0);
    
    
    for (int i = 0; i < (*loopMax); i ++){
        if (i % 4 == 0)
            g.setColour(Colours::steelblue);
        else
            g.setColour(Colours::lightsteelblue);
        g.drawLine(viewerBox.getX()+oneNoteWidth*i, viewerBox.getY(), viewerBox.getX()+oneNoteWidth*i, viewerBox.getY()+viewerBox.getHeight());
    }
    g.setColour(Colours::red);
    for (int i = 0; i < sequencePtr->getNumEvents(); i++){
        
        float tmpTime = sequencePtr->getEventTime(i);
        if (sequencePtr->getEventPointer(i)->message.isNoteOn() && tmpTime >= *loopMin && tmpTime < *loopMax){
           std::cout << " [" << i << "] : " << sequencePtr->getEventPointer(i)->message.getNoteNumber() << " at " << sequencePtr->getEventTime(i) << std::endl;
            
            float height = getHeight(sequencePtr->getEventPointer(i)->message.getNoteNumber());
        //    std::cout << " get height " << sequencePtr->getEventPointer(i)->message.getNoteNumber() << " is "<< height;
            float x = viewerBox.getX()+viewerBox.getWidth()*(tmpTime-*loopMin)/(*loopMax - *loopMin);
        //    std::cout << " x " << tmpTime << " is " << x << std::endl;
            float duration = sequencePtr->getEventTime(sequencePtr->getIndexOfMatchingKeyUp(i)) - tmpTime;
            //std::cout << " x is " << x << "height is " << height << std::endl;
            Rectangle<float> tmp(x, height, duration*oneNoteWidth, binHeight);
            g.drawRect(tmp);
            g.fillRect(tmp);
        }
        
    }
    
    
    
   
}
        
float MidiSequenceViewer::getHeight(float pitch){
    //std::cout << " pitch is " << pitch << "height is " << viewerBox.getY() + viewerBox.getHeight()*(pitchMax - pitch)/(float)(pitchMax - pitchMin) << std::endl;
    return viewerBox.getY() + viewerBox.getHeight()*(pitchMax - pitch)/(float)(pitchMax - pitchMin);
}

void MidiSequenceViewer::setPitchLimits(){
    int tmpMin = getMinPitch();
    int tmpMax =  getMaxPitch();
    pitchMin = tmpMin - tmpMin%12;
    pitchMax = tmpMax + 12 - tmpMax%12;
    
}

int MidiSequenceViewer::getMinPitch(){
    pitchMin = 100;
    for (int i = 0; i < sequencePtr->getNumEvents(); i++){
        if (sequencePtr->getEventPointer(i)->message.isNoteOn() && sequencePtr->getEventPointer(i)->message.getNoteNumber() < pitchMin)
            pitchMin = sequencePtr->getEventPointer(i)->message.getNoteNumber();
    }
    return pitchMin;
}

int MidiSequenceViewer::getMaxPitch(){
    pitchMax = 32;
    for (int i = 0; i < sequencePtr->getNumEvents(); i++){
        if (sequencePtr->getEventPointer(i)->message.isNoteOn() && sequencePtr->getEventPointer(i)->message.getNoteNumber() >pitchMax)
            pitchMax = sequencePtr->getEventPointer(i)->message.getNoteNumber();
    }
    return pitchMax;
}



void MidiSequenceViewer::setSequence(MidiMessageSequence& sequence){
    sequencePtr = &sequence;
}