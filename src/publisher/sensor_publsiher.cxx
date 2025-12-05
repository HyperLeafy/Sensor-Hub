#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <atomic>
#include <mutex>
#include <fstream>
#include <iomanip>
#include <map>
// #include "utilites/safe_queue.h"
#include "utilities/safe_queue.h"
#include "dds/dds.hpp"
#include "message_schema.hpp"
#include "Sensor_wrapper.hpp"
#include "Serializer/sensor.pb.h"

using namespace org::eclipse::cyclonedds;

std::atomic<bool> ctrl_switch_temp{false};
std::atomic<bool> ctrl_switch_pressure{false};
std::atomic<bool> ctrl_switch_flow{false};
std::atomic<bool> ctrl_switch_aggregator{false};
std::atomic<uint32_t> seq_counter{0};
std::atomic<uint32_t> temp_seq_counter{0};
std::atomic<uint32_t> pres_seq_counter{0};
std::atomic<uint32_t> flow_seq_counter{0};

std::mutex log_mutex;
std::mutex dashboard_mutex;

// Dashboard state
std::map<std::string, double> latest_value;
std::map<std::string, uint64_t> latest_timestamp;
std::map<std::string, uint32_t> latest_seq;
std::map<std::string, uint32_t> published_count;

void temp_sensor_data(safeQueue<sensorData::msg>& squeue, double_t min_temp, double_t max_temp){
    static std::random_device RD_T;
    std::uniform_real_distribution<double_t> dis_generator(min_temp, max_temp);

    while(!ctrl_switch_temp){
        sensorData::msg temp_meassge;
        temp_meassge.sensor_id("Temp-Sensor");
        temp_meassge.value(dis_generator(RD_T));
        temp_meassge.timeStamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        temp_meassge.sequence_num(temp_seq_counter++);
        squeue.push_in_queue(temp_meassge);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void press_sensor_data(safeQueue<sensorData::msg>& squeue, double_t min_press, double_t max_press){
    static std::random_device RD_T;
    std::uniform_real_distribution<double_t> dis_generator(min_press, max_press);

    while(!ctrl_switch_pressure){
        sensorData::msg pressure_meassge;
        pressure_meassge.sensor_id("Press-Sensor");
        pressure_meassge.value(dis_generator(RD_T)); 
        pressure_meassge.timeStamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        pressure_meassge.sequence_num(pres_seq_counter++);
        squeue.push_in_queue(pressure_meassge);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));       
    }
}

void flow_sensor_data(safeQueue<sensorData::msg>& squeue, double_t min_rate, double_t max_rate){
    static std::random_device RD_P;
    std::uniform_real_distribution<double_t> dis_generator(min_rate, max_rate);

    while(!ctrl_switch_flow){
        sensorData::msg flow_message;
        flow_message.sensor_id("flow-Sensor");
        flow_message.value(dis_generator(RD_P));
        flow_message.timeStamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        flow_message.sequence_num(flow_seq_counter++);
        squeue.push_in_queue(flow_message);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    }
}

void log_message(const sensorData::msg& msg){
    std::ofstream logFile("Publisher-Log.csv", std::ios::app);
    std::lock_guard<std::mutex> lock(log_mutex);    
    logFile<< msg.sensor_id() << " " << msg.value() << " " << msg.timeStamp() << " " << msg.sequence_num() << "\n";
}

void clear_terminal() {
    std::cout << "\033[2J\033[1;1H";
}

void printPublisherDashboard() {
    std::lock_guard<std::mutex> lock(dashboard_mutex);
    clear_terminal();
    
    std::cout << "\n==================== PUBLISHER DASHBOARD ====================\n\n";
    std::cout << std::left 
              << std::setw(15) << "Sensor"
              << std::setw(12) << "Value"
              << std::setw(18) << "Timestamp"
              << std::setw(8) << "Seq"
              << std::setw(12) << "Published" << "\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (const auto& [sensor, value] : latest_value) {
        std::cout << std::left 
                  << std::setw(15) << sensor
                  << std::setw(12) << std::fixed << std::setprecision(2) << value
                  << std::setw(18) << latest_timestamp[sensor]
                  << std::setw(8) << latest_seq[sensor]
                  << std::setw(12) << published_count[sensor] << "\n";
    }
    
    uint32_t total_published = 0;
    for (const auto& [sensor, count] : published_count) {
        total_published += count;
    }
    
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "TOTAL PUBLISHED: " << total_published << " messages\n";
    std::cout << std::string(70, '=') << "\n";
}

void aggregrator(safeQueue<sensorData::msg>& temp, safeQueue<sensorData::msg>& pressure, safeQueue<sensorData::msg>& flow, dds::pub::DataWriter<SensorData::RawSensorData>& sensorWriter){
    std::vector<sensorData::msg> temporary_container;
    const int16_t TOLLARANCE_IN_MS = 1000;
    sensor_proto::proto_serial_data proto_msg_data;
    SensorData::RawSensorData buffer_to_dds;
    sensorData::msg data;
    int msg_count = 0;

    while (!ctrl_switch_aggregator){
        if(temp.try_pop(data)) temporary_container.push_back(data); 
        if(pressure.try_pop(data)) temporary_container.push_back(data); 
        if(flow.try_pop(data)) temporary_container.push_back(data); 

        while(!temporary_container.empty()){
            auto ref_timestamp = temporary_container.front().timeStamp();
            std::vector<sensorData::msg> to_publish;
            
            for (auto it = temporary_container.begin(); it!=temporary_container.end();){
                if (std::abs(it->timeStamp() - ref_timestamp) <= TOLLARANCE_IN_MS){
                    to_publish.push_back(*it);
                    it = temporary_container.erase(it);
                }
                else ++it;
            }

            for(auto msg: to_publish){
                // sensorWriter.write(msg);
                seq_counter++;
                log_message(msg);

                // PROTOBUF CONVERSION
                proto_msg_data.set_sensor_id(msg.sensor_id());
                proto_msg_data.set_value(msg.value());
                proto_msg_data.set_timestamp(msg.timeStamp());
                proto_msg_data.set_sequence_num(msg.sequence_num());
                // SERIALZED BUFFER CREATED 
                std::string buffer;
                proto_msg_data.SerializeToString(&buffer);
                // sensorWriter.write(buffer);
                buffer_to_dds.data().assign(buffer.begin(), buffer.end());
                sensorWriter.write(buffer_to_dds);                

                // Update dashboard state
                {
                    std::lock_guard<std::mutex> lock(dashboard_mutex);
                    latest_value[msg.sensor_id()] = msg.value();
                    latest_timestamp[msg.sensor_id()] = msg.timeStamp();
                    latest_seq[msg.sensor_id()] = msg.sequence_num();
                    published_count[msg.sensor_id()]++;
                }
                
                msg_count++;
                
                // Refresh dashboard every 5 messages
                if (msg_count % 5 == 0) {
                    printPublisherDashboard();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

int32_t main() {
    safeQueue<sensorData::msg> temp_sensor_data_queue;
    safeQueue<sensorData::msg> pres_sensor_data_queue;
    safeQueue<sensorData::msg> flow_sensor_data_queue;

    try{
        dds::domain::DomainParticipant pub_participent_entity(domain::default_id());
        dds::topic::Topic<SensorData::RawSensorData> sensorTelemetyTopic(pub_participent_entity, "SENSOR-TELEMETRY");
        dds::pub::Publisher publisher_entity(pub_participent_entity);
        
        // Create QoS for best-effort streaming data
        // dds::pub::qos::DataWriterQos besteffort_sensor_qos;
        // besteffort_sensor_qos << dds::core::policy::Reliability::BestEffort() << dds::core::policy::History::KeepLast(1) << dds::core::policy::Durability::Volatile();
        dds::pub::qos::DataWriterQos reliable_sensor_qos;
        reliable_sensor_qos << dds::core::policy::Reliability::Reliable() << dds::core::policy::History::KeepLast(10) << dds::core::policy::Durability::TransientLocal() << dds::core::policy::Deadline(dds::core::Duration::from_millisecs(1000));
        
        std::cout<<"===[PUBLISHER] Successfully created Publisher Entity"<<std::endl;
        dds::pub::DataWriter<SensorData::RawSensorData> sensorWriterObj(publisher_entity, sensorTelemetyTopic);
        std::cout<<"===[PUBLISHER] Writer created" << std::endl;
        std::cout<<"===[PUBLISHER] STARTED"<<std::endl;

        std::thread temp_thread(temp_sensor_data, std::ref(temp_sensor_data_queue), 20.0, 100.0);
        std::thread pres_thread(press_sensor_data, std::ref(pres_sensor_data_queue), 220.0, 350.0);
        std::thread flow_thread(flow_sensor_data, std::ref(flow_sensor_data_queue), 500.0, 1000.0);
        std::thread sensor_thread(aggregrator, std::ref(temp_sensor_data_queue), std::ref(pres_sensor_data_queue), std::ref(flow_sensor_data_queue), std::ref(sensorWriterObj));

        std::this_thread::sleep_for(std::chrono::seconds(20));

        ctrl_switch_temp = true;
        ctrl_switch_pressure = true;
        ctrl_switch_flow = true;
        ctrl_switch_aggregator = true;

        std::cout<<"\n===[PUBLISHER] STOPPED"<<std::endl;
        temp_thread.join();
        pres_thread.join();
        flow_thread.join();
        sensor_thread.join();

    }catch (const dds::core::Exception& ce){
        std::cerr << "===[PUBLISHER] Exception : " << ce.what() <<std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}