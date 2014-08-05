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
        //const int numArgs = m.ArgumentCount();
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
            if (arg->IsFloat()){
                beat = arg->AsFloat();
                std::cout << "beat is " << beat << std::endl;
            } else if (arg->IsInt32()){
                beat = arg->AsInt32();
            }
                //std::cout << "not int" << std::endl;
            arg++;
            if (arg->IsInt32()){
                systemTime = arg->AsInt32();
                //std::cout << "ableton system time " << systemTime << std::endl;
            }
            arg++;
            if (arg->IsFloat() || arg->IsInt32()){
                tempoMillis = 60000.0/arg->AsFloatUnchecked();
                std::cout << "tempo " <<  tempoMillis << std::endl;
            }

            //ordering is important below
            //if (!halfTime){
                tempoVal.setValue(tempoMillis);//first so we know tempo
                sysTimeVal.setValue(systemTime);
                beatVal.setValue(beat);
            //} else if (beat % 2 == 1){
              //  tempoVal.setValue(tempoMillis* 2);
              //  sysTimeVal.setValue(systemTime);
              //  beatVal.setValue((beat+1)/2) ;
            // }
            //newBeatReceived - add a listener?
            
            //forgettung the half time stuff for the moment
            //we change the values of the associated variables
            //listener on these in main component will do our fn call
            
            std::cout << "beatset " << (float)beatVal.getValue() << std::endl;
        }
    }
    catch (osc::Exception& e)
    {
        DBG("error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n");
    }
}

