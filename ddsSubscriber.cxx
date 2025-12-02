#include <iostream>
#include <thread>
#include "dds/dds.hpp"
#include "build/message_schema.hpp"
using namespace dds::domain;
using namespace org::eclipse::cyclonedds;



int32_t main(){
    // Creaiting domain participent entity
    dds::domain::DomainParticipant entity_sub(domain::default_id());
    try{
        dds::topic::Topic<sensorData::msg> topic(entity_sub, "sensorData");
        dds::sub::Subscriber subscriber(entity_sub);
        std::cout<<"\t\t[SUBSCRIBER] Succefull created a Publishing Entity"<<std::endl;
        try{
            dds::sub::DataReader<sensorData::msg> writer(subscriber, topic);
        }catch (const dds::core::Exception& er){
            std::cerr << "\t\t\t[SUBSCRIBER] Exception : " << er.what() <<std::endl;
            return EXIT_FAILURE;
        }
    }catch (const dds::core::Exception& ce){
        std::cerr << "\t\t\t[SUBSCRIBER] Exception : " << ce.what() <<std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}