// test_end_to_end.cxx
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <cmath>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "utilites/safe_queue.h"
#include "../build/Serializer/sensor.pb.h"

// ------------------------
// Minimal sensor message
// ------------------------
struct TestMsg {
    std::string sensor_id;
    double value;
    uint64_t timestamp;
    uint32_t sequence_num;
};

// ------------------------
// Aggregator logic (DDS-free)
// ------------------------
void aggregator(safeQueue<TestMsg>& temp,
                safeQueue<TestMsg>& pressure,
                safeQueue<TestMsg>& flow,
                std::vector<std::string>& serialized_outputs,
                std::atomic<bool>& stop_flag) {

    const int64_t TOLERANCE_MS = 1000;
    sensor_proto::proto_serial_data proto_msg;

    std::vector<TestMsg> temporary_container;
    TestMsg data;

    while(!stop_flag) {
        if(temp.try_pop(data)) temporary_container.push_back(data);
        if(pressure.try_pop(data)) temporary_container.push_back(data);
        if(flow.try_pop(data)) temporary_container.push_back(data);

        while(!temporary_container.empty()) {
            uint64_t ref_ts = temporary_container.front().timestamp;
            std::vector<TestMsg> to_publish;

            for(auto it = temporary_container.begin(); it != temporary_container.end(); ) {
                if(std::abs(int64_t(it->timestamp - ref_ts)) <= TOLERANCE_MS) {
                    to_publish.push_back(*it);
                    it = temporary_container.erase(it);
                } else {
                    ++it;
                }
            }

            for(auto& msg : to_publish) {
                proto_msg.set_sensor_id(msg.sensor_id);
                proto_msg.set_value(msg.value);
                proto_msg.set_timestamp(msg.timestamp);
                proto_msg.set_sequence_num(msg.sequence_num);

                std::string buffer;
                proto_msg.SerializeToString(&buffer);
                serialized_outputs.push_back(buffer);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

// ------------------------
// Test
// ------------------------
TEST(E2E, PipelineSerializationTest) {
    safeQueue<TestMsg> temp_queue, pres_queue, flow_queue;
    std::vector<std::string> serialized_outputs;
    std::atomic<bool> stop_flag{false};

    // Producer threads
    auto producer = [](safeQueue<TestMsg>& q, const std::string& id){
        for(int i=0;i<5;i++){
            TestMsg msg{id, double(i*10), uint64_t(1000+i*100), uint32_t(i)};
            q.push_in_queue(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    };

    std::thread temp_th(producer, std::ref(temp_queue), "Temp");
    std::thread pres_th(producer, std::ref(pres_queue), "Pressure");
    std::thread flow_th(producer, std::ref(flow_queue), "Flow");

    // Aggregator thread
    std::thread agg_th(aggregator, std::ref(temp_queue), std::ref(pres_queue), std::ref(flow_queue),
                       std::ref(serialized_outputs), std::ref(stop_flag));

    temp_th.join();
    pres_th.join();
    flow_th.join();

    // Give aggregator a moment to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop_flag = true;
    agg_th.join();

    // ------------------------
    // Verify all messages serialized
    // ------------------------
    ASSERT_EQ(serialized_outputs.size(), 15); // 5 messages from each queue

    for(const auto& buf : serialized_outputs) {
        sensor_proto::proto_serial_data msg;
        ASSERT_TRUE(msg.ParseFromString(buf));
        EXPECT_FALSE(msg.sensor_id().empty());
        EXPECT_GE(msg.value(), 0.0);
    }
}
