//
//  OscAbletonFinder.h
//  JuceOscSyncTest
//
//  Created by Andrew Robertson on 30/07/2014.
//
//

#ifndef __JuceOscSyncTest__OscAbletonFinder__
#define __JuceOscSyncTest__OscAbletonFinder__


#include "OSCfinder.h"

class OSCAbletonFinder : public OSCfinder{
public:
    
    using OSCfinder::OSCfinder;
    
//    int* beat; 
    float beat;
    int systemTime;
    float tempoMillis;
    void init();
   // Value myVal;
    Value beatVal;
    Value sysTimeVal;
    Value tempoVal;
//    void valueChanged(Value* tmpValue);
    
    bool halfTime;
protected:
    //this is our main processing function
    //overwrite to to do other things with incoming osc messages
    void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& /*remoteEndpoint*/);
};


#endif





