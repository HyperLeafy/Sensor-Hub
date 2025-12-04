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
        dds::topic::Topic<sensorData::msg> sensorTopic(sub_participent_entity, "SENSOR-TELEMETRY");

        // Creating Subscriber
        dds::sub::Subscriber subscriber_entity(sub_participent_entity);
        std::cout<<"===[SUBSCRIBER] Succefull created a Subscriber Entity"<<std::endl;

        // Creating readers entites
        // dds::sub::DataReader<sensorData::msg> tempReaderObj(subscriber_entity, tempTopic);
        // dds::sub::DataReader<sensorData::msg> presReaderObj(subscriber_entity, presTopic);
        // dds::sub::DataReader<sensorData::msg> flowReaderObj(subscriber_entity, flowTopic);
        dds::sub::DataReader<sensorData::msg> sensorReaderObj(subscriber_entity, sensorTopic);

        // 
        while(!ctrl_switch){
            
            // Sensor telemetery reading
            auto temporary_sensor_data = sensorReaderObj.take();
            for(auto& it: temporary_sensor_data){
                if(it.info().valid()){
                    std::cout << it.data().sensor_id() << " "<< it.data().value() << " " << it.data().timeStamp() << "\n";
                }
            }
        }
    }catch(const dds::core::Exception& ce) {
        std::cerr << "DDS Error: " << ce.what() << std::endl;
        return 1;
    }
    

}