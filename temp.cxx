#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <fstream>
#include <numeric>
#include "utilites/safe_queue.h"
#include "dds/dds.hpp"
#include "build/message_schema.hpp"

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



//-------------------------------------------
// DASHBOARD
//-------------------------------------------

void clearScreen() {
    std::cout << "\033[2J\033[1;1H"; // ANSI escape codes
}

void dashboard(
    const std::map<std::string, int32_t>& total_received,
    const std::map<std::string, int32_t>& total_expected,
    const std::map<std::string, int32_t>& gaps_detected,
    const std::map<std::string, std::vector<int64_t>>& latency_per_sensor,
    const std::map<std::string, double>& latest_value,
    const std::map<std::string, uint64_t>& latest_seq,
    const std::map<std::string, int64_t>& latest_lat
){
    clearScreen();

    std::cout << "\n==========================TELEMETRY SUBSCRIBER DASHBOARD========================\n\n";
    std::cout << std::left << std::setw(15) << "Sensor" << std::setw(15) << "value" << std::setw(15) << "Latency(ms)" << std::setw(15) << "Losss %" << std::setw(15) << "Recv/Exp" << "\n";
    std::cout << std::string(90, '- -') << "\n";

    for(const auto& [sensor, recv]: total_received){
        
    }
}




int32_t main(){
    std::map<std::string, int32_t> seq_map; 
    std::map<std::string, int32_t> total_received_sensor;
    std::map<std::string, int32_t> total_expected_sensor;
    std::map<std::string, int32_t> gaps_detected;
    std::map<std::string, std::vector<int64_t>> latency_sensor;

    try{
        dds::domain::DomainParticipant participant(domain::default_id());
        dds::topic::Topic<sensorData::msg> sensorTopic(participant, "SENSOR-TELEMETRY");
        dds::sub::Subscriber subscriber(participant);
        std::cout << "===[SUBSCRIBER] Successfully created a Subscriber Entity" << std::endl;

        dds::sub::DataReader<sensorData::msg> sensorReader(subscriber, sensorTopic);

        while(!ctrl_switch){
            auto temporary_sensor_data = sensorReader.take();

            for(auto& it: temporary_sensor_data){
                if(!it.info().valid()) continue;

                uint64_t rec_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch()
                                    ).count();

                RECIVED_DATA data;
                static_cast<sensorData::msg&>(data) = it.data();
                data.revive_time = rec_time;

                log_message(data);

                std::string sensor_id = data.sensor_id();
                uint64_t current_seq = data.sequence_num();
                int64_t lat = latency(data);
                latency_sensor[sensor_id].push_back(lat);
                total_received_sensor[sensor_id]++;

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

                // Compute overall metrics
                int32_t total_gaps = 0, total_received_all = 0, total_expected_all = 0;
                for(const auto& [id, _]: seq_map){
                    total_gaps += gaps_detected[id];
                    total_received_all += total_received_sensor[id];
                    total_expected_all += total_expected_sensor[id];
                }
                double overall_loss_rate = (total_expected_all > 0) ? (total_gaps * 100.0) / total_expected_all : 0.0;

                // Compute average latency per sensor
                const auto& lats = latency_sensor[sensor_id];
                double avg_latency = !lats.empty() ? 
                    std::accumulate(lats.begin(), lats.end(), 0.0) / lats.size() : 0.0;

                // Terminal output
                std::cout << sensor_id << " " << data.value() 
                          << " TS:" << data.timeStamp() 
                          << " RT:" << data.revive_time
                          << " SEQ:" << data.sequence_num()
                        //   << " AVG_LAT:" << avg_latency << "ms\n";
                        << "LATENCY:" << lat << "ms\n";

                std::cout << "OVERALL LOSS: " << overall_loss_rate 
                          << "% | TOTAL RECEIVED: " << total_received_all
                          << " | TOTAL EXPECTED: " << total_expected_all << "\n\n";
            }
        }

    }catch(const dds::core::Exception& e){
        std::cerr << "DDS Error: " << e.what() << std::endl;
        return 1;
    }
}




