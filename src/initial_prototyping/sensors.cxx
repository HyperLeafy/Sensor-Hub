#include <iostream>
#include <chrono>
#include <random>
#include <ctime>
#include <thread>
#include <atomic>
#include "/home/blank/Projects/Internship/sentienc/onboarding-task/sensor-hub-test/utilites/safe_queue.h"

// Thread control flag
std::atomic<bool> ctrl_switch_temp{false};
std::atomic<bool> ctrl_switch_pressure{false};
std::atomic<bool> ctrl_switch_flow{false};

// Sensor Data struct
struct sesnorData{
    std::string sensor_id;
    double_t value;
    int32_t timeStamp;
};


// Funciton to generate Temprature sensor data
void temp_sensor_data(safeQueue<sesnorData>& temp_sense_queue ,double_t min_temp, double_t max_temp){
    static std::random_device RD_T;
    std::uniform_real_distribution<double_t> dis_generator(min_temp, max_temp);

    // The flag controll from outside to stop the temp data flow
    while(!ctrl_switch_temp){
        sesnorData mesured_reading_temp;
        // This what generates the actuall value
        mesured_reading_temp.sensor_id = "Temp-Sensor";
        mesured_reading_temp.value = dis_generator(RD_T); 
        mesured_reading_temp.timeStamp = std::chrono::system_clock::now().time_since_epoch().count();
        // Pushing every thing into the queue
        temp_sense_queue.push_in_queue(mesured_reading_temp);
    }
}

void pressure_sensor_data(safeQueue<sesnorData>& pressure_sensor_queue, double min_press, double_t max_press){
    static std::random_device RD_P;
    std::uniform_real_distribution<double_t> dis_generator(min_press, max_press);

    while(!ctrl_switch_pressure){
        sesnorData measured_reading_press;

        measured_reading_press.sensor_id = "Pressure-Sensor";
        measured_reading_press.value = dis_generator(RD_P);
        measured_reading_press.timeStamp = std::chrono::system_clock::now().time_since_epoch().count();
        pressure_sensor_queue.push_in_queue(measured_reading_press);
    }

}

void flow_sensor_data(safeQueue<sesnorData>& flow_sensor_queue, double_t min_rate, double_t max_rate){
    static std::random_device RD_P;
    std::uniform_real_distribution<double_t> dis_generator(min_rate, max_rate);

    while(!ctrl_switch_flow){
        sesnorData measured_reading_flow;

        measured_reading_flow.sensor_id = "Pressure-Sensor";
        measured_reading_flow.value = dis_generator(RD_P);
        measured_reading_flow.timeStamp = std::chrono::system_clock::now().time_since_epoch().count();
        flow_sensor_queue.push_in_queue(measured_reading_flow);
        // TO dealy values
        std::this_thread::sleep_for(std::chrono::seconds(2)); 
        // To print out the whole queue
        flow_sensor_queue.printQueue();
    }
}

int32_t main(){
    safeQueue<sesnorData> sensor_queue_temp;
    safeQueue<sesnorData> sensor_queue_pressure;
    safeQueue<sesnorData> sensor_queue_flow;


    std::thread temp_thread(temp_sensor_data, std::ref(sensor_queue_temp), 1000.00, 2000.00);

    std::thread pressure_thread(pressure_sensor_data, std::ref(sensor_queue_pressure), 200.00, 500.00);

    std::thread flow_thread(flow_sensor_data, std::ref(sensor_queue_flow), 10.00, 20.00);

    if(temp_thread.joinable()){
        temp_thread.join();
    }  
    // std::cout << "Collected " << sensor_queue.size() << " readings\n";
    return 0;
}