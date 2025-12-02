#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <atomic>

#include "dds/dds.hpp"
#include "build/message_schema.hpp"

using namespace org::eclipse::cyclonedds;

std::atomic<bool> ctrl_switch_temp{false};
std::atomic<bool> ctrl_switch_pressure{false};
std::atomic<bool> ctrl_switch_flow{false};

// Funciton to generate Temprature sensor data
void temp_sensor_data(dds::pub::DataWriter<sensorData::msg>& tempWriter ,double_t min_temp, double_t max_temp){
    static std::random_device RD_T;
    std::uniform_real_distribution<double_t> dis_generator(min_temp, max_temp);

    // The flag controll from outside to stop the temp data flow
    while(!ctrl_switch_temp){
        sensorData::msg temp_meassge;
        // sesnorData temp_meassge;
        // This what generates the actuall value
        temp_meassge.sensor_id("Temp-Sensor");
        temp_meassge.value(dis_generator(RD_T));
        temp_meassge.timeStamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        // Publishing the message
        // tempWriter.write(mesured_reading_temp);
        tempWriter.write(temp_meassge);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

// Funciton to generate Pressure sensor data
void press_sensor_data(dds::pub::DataWriter<sensorData::msg>& pressWriter ,double_t min_press, double_t max_press){
    static std::random_device RD_T;
    std::uniform_real_distribution<double_t> dis_generator(min_press, max_press);

    // The flag controll from outside to stop the temp data flow
    while(!ctrl_switch_pressure){
        sensorData::msg pressure_meassge;
        // This what generates the actuall value
        pressure_meassge.sensor_id("Press-Sensor");
        pressure_meassge.value(dis_generator(RD_T)); 
        pressure_meassge.timeStamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        // Publishing the pressure message
        pressWriter.write(pressure_meassge);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));       
    }
}
// Funciton to generate flowsensor data
void flow_sensor_data(dds::pub::DataWriter<sensorData::msg>& flowWriter, double_t min_rate, double_t max_rate){
    static std::random_device RD_P;
    std::uniform_real_distribution<double_t> dis_generator(min_rate, max_rate);

    while(!ctrl_switch_flow){
        sensorData::msg measured_reading_flow;
        // This what generates the actuall value
        measured_reading_flow.sensor_id("flow-Sensor");
        measured_reading_flow.value(dis_generator(RD_P));
        measured_reading_flow.timeStamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        // Publishing the flow message
        flowWriter.write(measured_reading_flow);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
    }
}


// --------------------MAIN------------------------

int32_t main() {
    try{
        dds::domain::DomainParticipant pub_participent_entity(domain::default_id());

        // Topic for each sensor
        dds::topic::Topic<sensorData::msg> tempTopic(pub_participent_entity, "TEMP-TOPIC");
        dds::topic::Topic<sensorData::msg> presTopic(pub_participent_entity, "PRESSURE-TOPIC");
        dds::topic::Topic<sensorData::msg> flowTopic(pub_participent_entity, "FLOW-TOPIC");

        // Creating Publisher 
        dds::pub::Publisher publisher_entity(pub_participent_entity);
        std::cout<<"===[PUBLISHER] Succefull created a Publisher Entity"<<std::endl;

        // Creating the writer entites
        dds::pub::DataWriter<sensorData::msg> tempWriterObj(publisher_entity, tempTopic);
        dds::pub::DataWriter<sensorData::msg> presWriterObj(publisher_entity, presTopic);
        dds::pub::DataWriter<sensorData::msg> flowWriterObj(publisher_entity, flowTopic);
        std::cout<<"===[PUBLISHER] Writer is created" << std::endl;
        std::cout<<"===[PUBLISHER] STARTED"<<std::endl;

        // Starting the sensor threads
        std::thread temp_thread(temp_sensor_data, std::ref(tempWriterObj), 20.0, 100.0);
        std::thread pres_thread(press_sensor_data, std::ref(tempWriterObj), 220.0, 350.0);
        std::thread flow_thread(flow_sensor_data, std::ref(tempWriterObj), 500.0, 1000.0);

        // shutdown logic for now sensor thread run untill main thread is asleep for 20sec
        std::this_thread::sleep_for(std::chrono::seconds(20));

        // Stop all threads
        ctrl_switch_temp = true;
        ctrl_switch_pressure = true;
        ctrl_switch_flow = true;

        //
        std::cout<<"===[PUBLISHER] STOPPED"<<std::endl;
    }catch (const dds::core::Exception& ce){
        std::cerr << "===[PUBLISHER] Exception : " << ce.what() <<std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}