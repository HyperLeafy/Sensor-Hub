#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

#include "dds/dds.hpp"
#include "build/message_schema.hpp"

using namespace org::eclipse::cyclonedds;

std::atomic<bool> ctrl_switch{false};

int32_t main(){
    try{
        dds::domain::DomainParticipant sub_participent_entity(domain::default_id());
        
        // Subscribing to topics
        dds::topic::Topic<sensorData::msg> tempTopic(sub_participent_entity, "TEMP-TOPIC");
        dds::topic::Topic<sensorData::msg> presTopic(sub_participent_entity, "PRESSURE-TOPIC");
        dds::topic::Topic<sensorData::msg> flowTopic(sub_participent_entity, "FLOW-TOPIC");

        // Creating Subscriber
        dds::sub::Subscriber subscriber_entity(sub_participent_entity);
        std::cout<<"===[SUBSCRIBER] Succefull created a Subscriber Entity"<<std::endl;

        // Creating readers entites
        dds::sub::DataReader<sensorData::msg> tempReaderObj(subscriber_entity, tempTopic);
        dds::sub::DataReader<sensorData::msg> presReaderObj(subscriber_entity, presTopic);
        dds::sub::DataReader<sensorData::msg> flowReaderObj(subscriber_entity, flowTopic);

        // 
        while(!ctrl_switch){
            

            // Temp sensor
            auto tempData = tempReaderObj.take();
            for(auto& s: tempData){
                if(s.info().valid()){
                    std::cout<< s.data().sensor_id() << " "<< s.data().value() << " " << s.data().timeStamp() << "\n";
                }
            }

            // Pressure sensors
            auto presData = presReaderObj.take();
            for(auto& s: presData){
                if(s.info().valid()){
                    std::cout<< s.data().sensor_id() << " "<< s.data().value() << " " << s.data().timeStamp() << "\n";
                }
            }

            // Flow sensoors
            auto flowData = flowReaderObj.take();
            for(auto& s: flowData){
                if(s.info().valid()){
                    std::cout<< s.data().sensor_id() << " "<< s.data().value() << " " << s.data().timeStamp() << "\n";
                }
            }
        }
    }catch(const dds::core::Exception& ce) {
        std::cerr << "DDS Error: " << ce.what() << std::endl;
        return 1;
    }
    

}