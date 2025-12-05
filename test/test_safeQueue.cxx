#include <gtest/gtest.h>
#include <thread>
#include "../utilites/safe_queue.h"

struct TestMsg {
    std::string sensor_id;
    double value;
    uint64_t timestamp;
    uint32_t sequence_num;
};


TEST(ChannelQueue, ProducerConsumer) {
    safeQueue<TestMsg> q;

    // Producer thread
    std::thread producer([&q] {
        for (int i = 0; i < 10; ++i) {
            TestMsg msg;
            msg.sensor_id = "TestSensor";
            msg.value = i * 1.1;
            msg.timestamp = 1000 + i;
            msg.sequence_num = i;
            q.push_in_queue(msg);
        }
    });

    // Consumer thread
    std::thread consumer([&q] {
        TestMsg msg;
        int count = 0;
        while (count < 10) {
            if (q.try_pop(msg)) {
                EXPECT_EQ(msg.sensor_id, "TestSensor");
                EXPECT_EQ(msg.sequence_num, count);
                count++;
            }
        }
    });

    producer.join();
    consumer.join();
}
