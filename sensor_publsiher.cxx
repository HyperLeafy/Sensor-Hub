#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <atomic>
#include "utilites/safe_queue.h"
#include "dds/dds.hpp"
#include "build/message_schema.hpp"

using namespace org::eclipse::cyclonedds;

std::atomic<bool> ctrl_switch_temp{false};
std::atomic<bool> ctrl_switch_pressure{false};
std::atomic<bool> ctrl_switch_flow{false};
std::atomic<bool> ctrl_switch_aggregator{false};

// Funciton to generate Temprature sensor data
//dds::pub::DataWriter<sensorData::msg>& tempWriter, 
void temp_sensor_data(safeQueue<sensorData::msg>& squeue, double_t min_temp, double_t max_temp){
    //arg: 1. writer obj , 2. queue refrence, 3. min value, 4. max value
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
        // tempWriter.write(temp_meassge);
        // Puts message in temp queue/channel
        squeue.push_in_queue(temp_meassge);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Funciton to generate Pressure sensor data
void press_sensor_data(safeQueue<sensorData::msg>& squeue, double_t min_press, double_t max_press){
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
        // pressWriter.write(pressure_meassge);
        // Puts message in pressure queue/channel
        squeue.push_in_queue(pressure_meassge);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));       
    }
}
// Funciton to generate flowsensor data
void flow_sensor_data(safeQueue<sensorData::msg>& squeue, double_t min_rate, double_t max_rate){
    static std::random_device RD_P;
    std::uniform_real_distribution<double_t> dis_generator(min_rate, max_rate);

    while(!ctrl_switch_flow){
        sensorData::msg flow_message;
        // This what generates the actuall value
        flow_message.sensor_id("flow-Sensor");
        flow_message.value(dis_generator(RD_P));
        flow_message.timeStamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        // Publishing the flow message
        // flowWriter.write(flow_message);
        // Puts message in flow queue/channel
        squeue.push_in_queue(flow_message);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    }
}


// ------------------Aggregator--------------------
void aggregrator(safeQueue<sensorData::msg>& temp, safeQueue<sensorData::msg>& pressure, safeQueue<sensorData::msg>& flow, dds::pub::DataWriter<sensorData::msg>& sensorWriter){
    // safeQueue<sensorData::msg> temparory_catch ;
    std::vector<sensorData::msg> temporary_container;
    bool haveTemp = false, havePress = false, haveFlow = false;
    const int8_t TOLLARANCE_IN_MS = 100;
    sensorData::msg data;

    while (!ctrl_switch_aggregator){
        // Non blocking check for 
        if(temp.try_pop(data)) temporary_container.push_back(data); 
        if(pressure.try_pop(data)) temporary_container.push_back(data); 
        if(flow.try_pop(data)) temporary_container.push_back(data); 

        // Sorting based on time stamps
        while(!temporary_container.empty()){
            auto ref_timestamp = temporary_container.front().timeStamp();
            std::vector<sensorData::msg> to_publish;
            
            // For batching out the messages
            for (auto it = temporary_container.begin(); it!=temporary_container.end();){
                if ((std::abs(it->timeStamp()) - ref_timestamp) <=TOLLARANCE_IN_MS){
                    to_publish.push_back(*it);
                    it = temporary_container.erase(it);
                }
                else ++it;
            }

            // Publsihing all batched messages
            for(auto msg: to_publish){
                sensorWriter.write(msg);
                std::cout << "[AGG] " << msg.sensor_id() << " " << msg.value() << " " << msg.timeStamp() << "\n";
            }
        }
    }

    // TO delay this thread a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}





// --------------------MAIN------------------------

int32_t main() {
    safeQueue<sensorData::msg> temp_sensor_data_queue;
    safeQueue<sensorData::msg> pres_sensor_data_queue;
    safeQueue<sensorData::msg> flow_sensor_data_queue;


    try{
        dds::domain::DomainParticipant pub_participent_entity(domain::default_id());

        // Topic for each sensor
        dds::topic::Topic<sensorData::msg> tempTopic(pub_participent_entity, "TEMP-TOPIC");
        dds::topic::Topic<sensorData::msg> presTopic(pub_participent_entity, "PRESSURE-TOPIC");
        dds::topic::Topic<sensorData::msg> flowTopic(pub_participent_entity, "FLOW-TOPIC");

        // Topic for sigle channle
        dds::topic::Topic<sensorData::msg> sensorTelemetyTopic(pub_participent_entity, "SENSOR-TELEMETRY");

        // Creating Publisher 
        dds::pub::Publisher publisher_entity(pub_participent_entity);
        std::cout<<"===[PUBLISHER] Succefull created a Publisher Entity"<<std::endl;

        // Creating the writer entites
        // dds::pub::DataWriter<sensorData::msg> tempWriterObj(publisher_entity, tempTopic);
        // dds::pub::DataWriter<sensorData::msg> presWriterObj(publisher_entity, presTopic);
        // dds::pub::DataWriter<sensorData::msg> flowWriterObj(publisher_entity, flowTopic);
        dds::pub::DataWriter<sensorData::msg> sensorWriterObj(publisher_entity, sensorTelemetyTopic);
        std::cout<<"===[PUBLISHER] Writer is created" << std::endl;
        std::cout<<"===[PUBLISHER] STARTED"<<std::endl;

        // Starting the sensor threads
        std::thread temp_thread(temp_sensor_data, std::ref(temp_sensor_data_queue), 20.0, 100.0);
        std::thread pres_thread(press_sensor_data, std::ref(pres_sensor_data_queue), 220.0, 350.0);
        std::thread flow_thread(flow_sensor_data, std::ref(flow_sensor_data_queue), 500.0, 1000.0);
        
        // Aggregato Thread
        std::thread sensor_thread( aggregrator, std::ref(temp_sensor_data_queue), std::ref(pres_sensor_data_queue), std::ref(flow_sensor_data_queue), std::ref(sensorWriterObj));

        // shutdown logic for now sensor thread run untill main thread is asleep for 20sec
        std::this_thread::sleep_for(std::chrono::seconds(20));

        // Stop all threads
        ctrl_switch_temp = true;
        ctrl_switch_pressure = true;
        ctrl_switch_flow = true;
        ctrl_switch_aggregator = true;

        //
        std::cout<<"===[PUBLISHER] STOPPED"<<std::endl;
    }catch (const dds::core::Exception& ce){
        std::cerr << "===[PUBLISHER] Exception : " << ce.what() <<std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}