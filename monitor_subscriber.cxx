#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <fstream>
#include <numeric>
#include <iomanip>
#include <map>
#include <vector>
#include "utilites/safe_queue.h"
#include "dds/dds.hpp"
#include "build/message_schema.hpp"
#include "Serializer/sensor.pb.h"
#include "build/Sensor_wrapper.hpp"

using namespace org::eclipse::cyclonedds;

std::atomic<bool> ctrl_switch{false};
std::mutex log_mutex;

struct RECIVED_DATA : sensorData::msg{ 
    uint64_t revive_time; 
};

void log_message(const RECIVED_DATA& data){
    std::ofstream logFile("Subscriber-Log.csv", std::ios::app);
    std::lock_guard<std::mutex> lock(log_mutex);    
    logFile << data.sensor_id() << " "
            << data.value() << " "
            << data.timeStamp() << " "
            << data.revive_time << " "
            << data.sequence_num() << "\n";        
}

int64_t latency(const RECIVED_DATA& data){
    return static_cast<int64_t>(data.revive_time) - static_cast<int64_t>(data.timeStamp());
}

void clearScreen() {
    std::cout << "\033[2J\033[1;1H"; // ANSI escape codes
}

void printDashboard(
    const std::map<std::string, int32_t>& total_received,
    const std::map<std::string, int32_t>& total_expected,
    const std::map<std::string, int32_t>& gaps_detected,
    const std::map<std::string, std::vector<int64_t>>& latency_sensor,
    const std::map<std::string, double>& latest_value,
    const std::map<std::string, uint64_t>& latest_seq,
    const std::map<std::string, int64_t>& latest_lat
) {
    clearScreen();
    
    std::cout << "\n======================== TELEMETRY MONITOR DASHBOARD ========================\n\n";
    std::cout << std::left 
              << std::setw(15) << "Sensor"
              << std::setw(12) << "Value"
              << std::setw(8) << "Seq"
              << std::setw(12) << "Lat(ms)"
              << std::setw(12) << "Avg Lat"
              << std::setw(12) << "Loss %"
              << std::setw(15) << "Recv/Exp" << "\n";
    std::cout << std::string(90, '-') << "\n";
    
    for (const auto& [sensor, recv] : total_received) {
        double avg_lat = 0.0;
        if (latency_sensor.count(sensor) && !latency_sensor.at(sensor).empty()) {
            const auto& lats = latency_sensor.at(sensor);
            avg_lat = std::accumulate(lats.begin(), lats.end(), 0.0) / lats.size();
        }
        
        int32_t expected = total_expected.count(sensor) ? total_expected.at(sensor) : 0;
        int32_t gaps = gaps_detected.count(sensor) ? gaps_detected.at(sensor) : 0;
        double loss_rate = (expected > 0) ? (gaps * 100.0) / expected : 0.0;
        
        double value = latest_value.count(sensor) ? latest_value.at(sensor) : 0.0;
        uint64_t seq = latest_seq.count(sensor) ? latest_seq.at(sensor) : 0;
        int64_t lat = latest_lat.count(sensor) ? latest_lat.at(sensor) : 0;
        
        std::cout << std::left 
                  << std::setw(15) << sensor
                  << std::setw(12) << std::fixed << std::setprecision(2) << value
                  << std::setw(8) << seq
                  << std::setw(12) << lat
                  << std::setw(12) << std::fixed << std::setprecision(2) << avg_lat
                  << std::setw(12) << std::fixed << std::setprecision(2) << loss_rate
                  << recv << "/" << expected << "\n";
    }
    
    // Overall stats
    int32_t total_gaps = 0, total_recv = 0, total_exp = 0;
    for (const auto& [id, _] : total_received) {
        total_gaps += gaps_detected.count(id) ? gaps_detected.at(id) : 0;
        total_recv += total_received.at(id);
        total_exp += total_expected.count(id) ? total_expected.at(id) : 0;
    }
    double overall_loss = (total_exp > 0) ? (total_gaps * 100.0) / total_exp : 0.0;
    
    std::cout << "\n" << std::string(90, '=') << "\n";
    std::cout << "OVERALL: Received: " << total_recv 
              << " | Expected: " << total_exp
              << " | Lost: " << total_gaps
              << " | Loss Rate: " << std::fixed << std::setprecision(2) << overall_loss << "%\n";
    std::cout << std::string(90, '=') << "\n";
}


sensorData::msg on_data_recived(const SensorData::RawSensorData& raw_data_message){
    sensor_proto::proto_serial_data proto_msg;
    sensorData::msg temporary_data;
    std::string buffer(raw_data_message.data().begin(), raw_data_message.data().end());
    if(!proto_msg.ParseFromString(buffer)){
        std::cerr << " Failed to Deserialze the buffer \n";
        return temporary_data;
    }
    temporary_data.sensor_id(proto_msg.sensor_id());
    temporary_data.sequence_num(proto_msg.sequence_num());
    temporary_data.value(proto_msg.value());
    temporary_data.timeStamp(proto_msg.timestamp());

    // returnig final sensorData::msg 
    return temporary_data;
}   

int32_t main(){
    std::map<std::string, int32_t> seq_map; 
    std::map<std::string, int32_t> total_received_sensor;
    std::map<std::string, int32_t> total_expected_sensor;
    std::map<std::string, int32_t> gaps_detected;
    std::map<std::string, std::vector<int64_t>> latency_sensor;
    std::map<std::string, double> latest_value;
    std::map<std::string, uint64_t> latest_seq;
    std::map<std::string, int64_t> latest_lat;

    try{
        dds::domain::DomainParticipant participant(domain::default_id());
        dds::topic::Topic<SensorData::RawSensorData> sensorTopic(participant, "SENSOR-TELEMETRY");
        dds::sub::Subscriber subscriber(participant);
        
        dds::sub::DataReader<SensorData::RawSensorData> sensorReader(subscriber, sensorTopic);

        int msg_count = 0;
        while(!ctrl_switch){
            auto temporary_sensor_data = sensorReader.take();

            for(auto& it: temporary_sensor_data){
                if(!it.info().valid()) continue;

                uint64_t rec_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch()
                                    ).count();

                RECIVED_DATA data;
                // Converting raw into mangable data 
                static_cast<sensorData::msg&>(data) = on_data_recived(it.data());
                // adding recived time stamp
                data.revive_time = rec_time;
                // final data 
                log_message(data);

                std::string sensor_id = data.sensor_id();
                uint64_t current_seq = data.sequence_num();
                int64_t lat = latency(data);
                
                latency_sensor[sensor_id].push_back(lat);
                total_received_sensor[sensor_id]++;
                latest_value[sensor_id] = data.value();
                latest_seq[sensor_id] = current_seq;
                latest_lat[sensor_id] = lat;

                // Gap detection
                if(seq_map.count(sensor_id)){
                    int32_t expected = seq_map[sensor_id] + 1;
                    if(current_seq != expected){
                        int32_t gap_size = current_seq - expected;
                        gaps_detected[sensor_id] += gap_size;
                    }
                    total_expected_sensor[sensor_id] += current_seq - seq_map[sensor_id];
                } else {
                    total_expected_sensor[sensor_id] = 1;
                }
                seq_map[sensor_id] = current_seq;

                msg_count++;
                
                // Refresh dashboard every 10 messages
                if (msg_count % 10 == 0) {
                    printDashboard(total_received_sensor, total_expected_sensor, gaps_detected, latency_sensor, latest_value, latest_seq, latest_lat);
                }
            }
        }

    }catch(const dds::core::Exception& e){
        std::cerr << "DDS Error: " << e.what() << std::endl;
        return 1;
    }
}