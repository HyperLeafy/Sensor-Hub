#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <atomic>

#include "dds/dds.hpp"
#include "build/message_schema.hpp"

using namespace org::eclipse::cyclonedds;

// -------- Control flags for shutdown ----------
std::atomic<bool> ctrl_switch_temp{false};
std::atomic<bool> ctrl_switch_pressure{false};
std::atomic<bool> ctrl_switch_flow{false};


// ------------ Sensor thread functions ------------

void temp_sensor_data(dds::pub::DataWriter<sensorData::msg>& writer,
                      double min_temp, double max_temp)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min_temp, max_temp);

    while (!ctrl_switch_temp)
    {
        sensorData::msg msg;
        msg.sensor_ID("Temp-Sensor");
        msg.value(dist(gen));
        msg.timeStamp(std::chrono::system_clock::now().time_since_epoch().count());

        writer.write(msg);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


void pressure_sensor_data(dds::pub::DataWriter<sensorData::msg>& writer,
                          double min_press, double max_press)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min_press, max_press);

    while (!ctrl_switch_pressure)
    {
        sensorData::msg msg;
        msg.sensor_ID("Pressure-Sensor");
        msg.value(dist(gen));
        msg.timeStamp(std::chrono::system_clock::now().time_since_epoch().count());

        writer.write(msg);

        std::this_thread::sleep_for(std::chrono::milliseconds(700));
    }
}


void flow_sensor_data(dds::pub::DataWriter<sensorData::msg>& writer,
                      double min_rate, double max_rate)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(min_rate, max_rate);

    while (!ctrl_switch_flow)
    {
        sensorData::msg msg;
        msg.sensor_ID("Flow-Sensor");
        msg.value(dist(gen));
        msg.timeStamp(std::chrono::system_clock::now().time_since_epoch().count());

        writer.write(msg);

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}


// -------------------- main --------------------

int32_t main()
{
    try
    {
        // 1) Create participant
        dds::domain::DomainParticipant participant(domain::default_id());

        // 2) Create topics (one per sensor)
        dds::topic::Topic<sensorData::msg> tempTopic(participant,     "TempTopic");
        dds::topic::Topic<sensorData::msg> pressureTopic(participant, "PressureTopic");
        dds::topic::Topic<sensorData::msg> flowTopic(participant,     "FlowTopic");

        // 3) Create publisher
        dds::pub::Publisher publisher(participant);

        // 4) Create writers
        dds::pub::DataWriter<sensorData::msg> tempWriter(publisher, tempTopic);
        dds::pub::DataWriter<sensorData::msg> pressureWriter(publisher, pressureTopic);
        dds::pub::DataWriter<sensorData::msg> flowWriter(publisher, flowTopic);

        std::cout << "=== Sensor Publisher started ===" << std::endl;

        // 5) Start sensor threads
        std::thread temp_thread(
            temp_sensor_data,
            std::ref(tempWriter),
            1000.0, 2000.0
        );

        std::thread pressure_thread(
            pressure_sensor_data,
            std::ref(pressureWriter),
            200.0, 500.0
        );

        std::thread flow_thread(
            flow_sensor_data,
            std::ref(flowWriter),
            10.0, 20.0
        );

        // Let the sensors run (or replace with proper shutdown logic)
        std::this_thread::sleep_for(std::chrono::seconds(30));

        // Stop all threads
        ctrl_switch_temp = true;
        ctrl_switch_pressure = true;
        ctrl_switch_flow = true;

        temp_thread.join();
        pressure_thread.join();
        flow_thread.join();

        std::cout << "=== Sensor Publisher stopped ===" << std::endl;
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "DDS Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
