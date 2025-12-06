#include <iostream>
#include <gtest/gtest.h>
#include "sensor.pb.h"

TEST(Serialization, RoundTrip){
    sensor_proto::proto_serial_data a;
    a.set_sensor_id("Temp");
    a.set_value(42.5);
    a.set_timestamp(123456);
    a.set_sequence_num(7);

    std::string buf;
    ASSERT_TRUE(a.SerializeToString(&buf));

    sensor_proto::proto_serial_data b;
    ASSERT_TRUE(b.ParseFromString(buf));

    EXPECT_EQ(b.sensor_id(), "Temp");
    EXPECT_DOUBLE_EQ(b.value(), 42.5);
    EXPECT_EQ(b.timestamp(), 123456);
    EXPECT_EQ(b.sequence_num(), 7);
}