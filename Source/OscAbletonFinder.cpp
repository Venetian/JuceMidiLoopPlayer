//
//  OscAbletonFinder.cpp
//  JuceOscSyncTest
//
//  Created by Andrew Robertson on 30/07/2014.
//
//

#include "OscAbletonFinder.h"



void OSCAbletonFinder::init(){

}

//void OSCAbletonFinder::valueChanged(Value* tmpValue){}

void OSCAbletonFinder::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& /*remoteEndpoint*/)
{
    
    try
    {
        String msgPattern = m.AddressPattern();
        const int numArgs = m.ArgumentCount();
        /*
        if (msgPattern.equalsIgnoreCase(OSCPrefix))
            std::cout << "ableton osc message matches address: ";
        else
            std::cout << "ableton received something else: ";
        std::cout << msgPattern;
        
        std::cout << " and " << numArgs << " argument(s)" << std::endl;
        
        */
        
        osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
        
      
        if (msgPattern.equalsIgnoreCase(OSCPrefix)){
            arg = m.ArgumentsBegin();
            if (arg->IsInt32()){
                beat = arg->AsInt32();
                //std::cout << "beat is " << beat << std::endl;
            } //else
                //std::cout << "not int" << std::endl;
            arg++;
            if (arg->IsInt32()){
                systemTime = arg->AsInt32();
               // std::cout << "system time " << systemTime << std::endl;
            }
            arg++;
            if (arg->IsFloat() || arg->IsInt32()){
                tempoMillis = arg->AsFloatUnchecked();
                //std::cout << "tempo " <<  tempoMillis << std::endl;
            }

            
            if (!halfTime){
                tempoVal.setValue(tempoMillis);//first so we know tempo
                beatVal.setValue(beat);
                sysTimeVal.setValue(systemTime);
                
            } else if (beat % 2 == 1){
                tempoVal.setValue(tempoMillis* 2);
                beatVal.setValue((beat+1)/2) ;
                sysTimeVal.setValue(systemTime);
                
            }
            //newBeatReceived - add a listener?
        }
    }
    catch (osc::Exception& e)
    {
        DBG("error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n");
    }
}

